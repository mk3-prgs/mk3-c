//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "includes.h"
#include "mp3dec.h"


//*-----------------------------------------------------------------------------------------------
//*			Константы
//*-----------------------------------------------------------------------------------------------
#define MP3_TASK_STK_SIZE	256			// размер стека задачи
#define MP3_FILEBUFF_SIZE	4096		// размер файлового буфера
#define DAC_BUFFER_SIZE		(1152*2)	// размер выходного буфера (16-битных слов) для одного фрейма *2 канала
#define MP3_HALT_ID			4

#define MP3_TASK_PRIOR 15

#define MONO_SUPPORT

//*-----------------------------------------------------------------------------------------------
//*			Описание типов
//*-----------------------------------------------------------------------------------------------
/// результат чтения потока MP3 из файла для функции
typedef enum
{
	READ_OK 		=  0,
	READ_ERROR		= -1,
	READ_END_DATA	= -2
} read_result_t;


/// структура переменной состояния
// внимание! если данная структура не будет выровнена по адресу кратному 4 байт,
// в декодере mp3 возникнет исключение UNALIGNED
typedef struct
{
	// переменные для задачи
	///OS_STK task_stk[MP3_TASK_STK_SIZE];	// стек задачи
	xTaskHandle decoderTaskID;			// ID задачи

	// файловые переменные
	FIL *mp3_file;						// указатель на играемый файл
	uint32_t fileAddr;					// адрес первого непрочитанного из файла байта

	// переменные для декодера
	HMP3Decoder hMP3Decoder;			// указатель на ОЗУ декодера
	MP3FrameInfo mp3FrameInfo;			// параметры фрейма

	// буферы
	uint8_t inputBuf[MP3_FILEBUFF_SIZE];	// буфер для входного потока
	int16_t outBuff[2][DAC_BUFFER_SIZE];	// буфер выходного потока
	uint32_t outBuffPtr;					// указатель на свободный буфер

	uint8_t * inputDataPtr;		// указатель начала данных в файловом буфере
	int bytes_left;				// указатель количества данных в файловом буфере
	uint32_t samprate;
	uint32_t frameCNT;			// счётчик декодированных фреймов

	// локальные переменные для управления ЦАП и задачей проигрывателя
	dac_srate_t			dac_srate;
	dac_data_portion_t 	dac_data_portion;
	dac_cmd_t 			dac_cmd;
	cmd_t	 			player_cmd;

	xTaskHandle*		dac_DataReqFlag;
	int     			dac_CmdMailbox;
	xTaskHandle*		player_CmdMailBox;
} mp3DecoderState_t;


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
static mp3DecoderState_t * mp3DecoderState;	// переменная состояния, для которой выделяется место в куче
static _Bool allocated = false;				// флаг состояния памяти декодера mp3

//*-----------------------------------------------------------------------------------------------
//*			Прототипы
//*-----------------------------------------------------------------------------------------------
static void MP3Task(void* pdata);

static uint32_t m_time=0;
void StartTimeMeasurement(void)
{
    m_time=0;
}

uint32_t StopTimeMeasurement(void)
{
    return(m_time);
}

//*-----------------------------------------------------------------------------------------------
/**			Останов программы и индикация ошибки												*/
//*-----------------------------------------------------------------------------------------------
static void StopError(void)
{
	vTaskDelay(500);
	DBG_HALT(MP3_HALT_ID);
	while(1)
	{
		LED_ON;
		vTaskDelay(100);
		LED_OFF;
		vTaskDelay(100);
	}
}

//*-----------------------------------------------------------------------------------------------
/**			функция деинициализации декодера
 *
 * @note освобождает память в куче, занятую декодером											*/
//*-----------------------------------------------------------------------------------------------
static void MP3_Deinit(void)
{
	if (!allocated)
		return;

	///OS_TID tempTaskID = mp3DecoderState-> decoderTaskID;
	MP3FreeDecoder(mp3DecoderState->hMP3Decoder);
	free(mp3DecoderState);
	allocated = false;
	///CoDelTask(tempTaskID);
}

//*-----------------------------------------------------------------------------------------------
/**			Инициализация задачи-декодера MP3
 *
 * @param mp3_file  - указатель на файловую структуру (открытый файл для воспроизведения)
 * @param dac_CmdMailbox - майлбокс для управления ЦАП
 * @param playCmdMailBox - майлбокс для управления главным проигрываетелем
 *
 * @return				 - указатель на функцию деинициализации декодера						*/
//*-----------------------------------------------------------------------------------------------
void * mp3_player_init(decoder_param_t * decoder_params)
{
	// смотрим, задача не инициализирована?
	if (allocated)
	{
		// обновить счётчики/указатели
		mp3DecoderState->mp3_file = decoder_params->file;
		mp3DecoderState->inputDataPtr = NULL;
		mp3DecoderState->bytes_left = 0;
		mp3DecoderState->frameCNT = 0;
		mp3DecoderState->outBuffPtr = 0;
		mp3DecoderState->samprate = 0;
		mp3DecoderState->fileAddr = 0;
	}
	else // полная начальная инициализация
	{
		//assert_param(CoGetCurTaskID() >= MP3_TASK_PRIOR);

		// выделить память для задачи
		mp3DecoderState = malloc(sizeof(mp3DecoderState_t));
		if (mp3DecoderState == NULL)
		{
			xprintf("Ошибка выделения памяти для mp3-задачи\r\n");
			StopError();
		}

		// выделить память для декодера
		mp3DecoderState->hMP3Decoder = MP3InitDecoder();
		if (mp3DecoderState->hMP3Decoder == NULL)
		{
			xprintf("Ошибка выделения памяти для mp3-декодера\r\n");
			StopError();
		}

		allocated = true;

		// обновить счётчики/указатели
		mp3DecoderState->mp3_file = decoder_params->file;
		mp3DecoderState->dac_CmdMailbox = decoder_params->dacCmdMailbox;
		mp3DecoderState->dac_DataReqFlag = decoder_params->dacDataReqFlag;
		mp3DecoderState->player_CmdMailBox = decoder_params->playCmdMailBox;

		mp3DecoderState->inputDataPtr = NULL;
		mp3DecoderState->bytes_left = 0;
		mp3DecoderState->frameCNT = 0;
		mp3DecoderState->outBuffPtr = 0;
		mp3DecoderState->samprate = 0;
		mp3DecoderState->fileAddr = 0;

		// флаг ожидания готовности данных с карты памяти
		//mp3DecoderState->mp3_fileReadWaitFlag = CoCreateFlag(Co_TRUE, 0);

		// инициализация задачи
		/*
		mp3DecoderState->decoderTaskID = CoCreateTask(MP3Task,
													  (void *)0,
													  MP3_TASK_PRIOR,
													  &mp3DecoderState->task_stk[MP3_TASK_STK_SIZE-1],
													  MP3_TASK_STK_SIZE);
		*/
		xTaskCreate( MP3Task,   "MP3Task",    MP3_TASK_STK_SIZE, NULL, MP3_TASK_PRIOR, (xTaskHandle*)&mp3DecoderState->decoderTaskID);
		//if (debug_mode.showDecoderInfo)
		//{
			xprintf("Декодер MP3 инициализирован\r\n");
			xprintf("ID задачи декодера MP3: %u\r\n", (unsigned int)mp3DecoderState->decoderTaskID);
		//}
	}

	f_lseek(mp3DecoderState->mp3_file, mp3DecoderState->fileAddr);

	// TODO здесь можно прочитать и отобразить тег MP3-файла

	// запустить ЦАП
	mp3DecoderState->dac_cmd = DAC_START;

	///CoPostMail(mp3DecoderState->dac_CmdMailbox, &mp3DecoderState->dac_cmd);

	return &MP3_Deinit;
}


//*-----------------------------------------------------------------------------------------------
/**			Вспомогательная функция чтения потока MP3 из файла
 *
 * @param mp3DecoderState - указатель на переменную состояния проигрывателя
 * @return - результат выполнения команды														*/
//*-----------------------------------------------------------------------------------------------
static FRESULT ReadMP3buff(mp3DecoderState_t * mp3DecoderState)
{
	DATA_READ_MARKER_HIGH;

	// если буфер полный, ничего не делать
	if (mp3DecoderState->bytes_left >= MP3_FILEBUFF_SIZE)
		return READ_OK;

	// если в файловом буфере остались данные, переместить их в начало буфера
	if (mp3DecoderState->bytes_left > 0)
	{
		memcpy(mp3DecoderState->inputBuf, mp3DecoderState->inputDataPtr, mp3DecoderState->bytes_left);
	}
	assert_param(mp3DecoderState->bytes_left >= 0 && mp3DecoderState->bytes_left <= MP3_FILEBUFF_SIZE);

	// прочитать очередную порцию данных
	uint32_t bytes_to_read = MP3_FILEBUFF_SIZE - mp3DecoderState->bytes_left;
	uint32_t bytes_read;
	FRESULT result = f_read(mp3DecoderState->mp3_file,
							(BYTE *)mp3DecoderState->inputBuf + mp3DecoderState->bytes_left,
							bytes_to_read,
							(UINT*)&bytes_read);
	mp3DecoderState->fileAddr += bytes_to_read;
	if (result != FR_OK)
	{
		DATA_READ_MARKER_LOW;
		return result;
	}

	mp3DecoderState->inputDataPtr = mp3DecoderState->inputBuf;
	mp3DecoderState->bytes_left += bytes_read;

	DATA_READ_MARKER_LOW;
	if (bytes_read == bytes_to_read)
		return READ_OK;
	else
		return READ_END_DATA;
}


//*-----------------------------------------------------------------------------------------------
/**			Останов декодера
 *
 * @param player_cmd - команда, отправляемая главному проигрывателю								*/
//*-----------------------------------------------------------------------------------------------
static void stop_decode(cmd_t player_cmd)
{
	// остановить ЦАП
	mp3DecoderState->dac_cmd = DAC_STOP;
	CoPostMail(mp3DecoderState->dac_CmdMailbox, &mp3DecoderState->dac_cmd);

	// команда проигрывателю
	mp3DecoderState->player_cmd = player_cmd;
	CoPostMail(mp3DecoderState->player_CmdMailBox, &mp3DecoderState->player_cmd);
}

void put_rc(FRESULT rc)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0" "INVALID_PARAMETER\0";
	FRESULT i;

	for(i = 0; i != rc && *str; i++) {
		while (*str++) ;
        }

	if(rc) xprintf("rc=%u FR_%s\r\n", (UINT)rc, str);
}


//*-----------------------------------------------------------------------------------------------
/**			Задача декодера
 *
 * @param pdata - указатель на данные (не используется)											*/
//*-----------------------------------------------------------------------------------------------
static void MP3Task(void* pdata)
{
    pdata=pdata;
	for(;;)
	{
		// ожидать запрос данных от ЦАП
		///StatusType result = CoWaitForSingleFlag (mp3DecoderState->dac_DataReqFlag,  0);

		StartTimeMeasurement();	// начало контроля времени выполнения

		// прочитать порцию данных
		DECODE_TEST_HIGH;
		FRESULT fileReadResult = ReadMP3buff(mp3DecoderState);
		DECODE_TEST_LOW;

		if((fileReadResult != READ_OK)&&(fileReadResult != READ_END_DATA)) {
			//if(debug_mode.showDecoderInfo) {
                xprintf("Ошибка чтения файла:\r\n");
                put_rc(fileReadResult);
				xprintf("Error in %s line %u\r\n", (uint8_t *)__FILE__, (unsigned int)__LINE__);
            //    }
			StopTimeMeasurement();
			stop_decode(DECODE_ERROR);
			continue;
            }

		if(fileReadResult == READ_END_DATA) {
			//if(debug_mode.showDecoderInfo) {
				xprintf("Воспроизведение файла завершено\r\n");
            //    }
			StopTimeMeasurement();
			stop_decode(SONG_COMPLETE);
			continue;
            }

		// поиск синхрослова - начала фрейма
		int offset = MP3FindSyncWord(mp3DecoderState->inputDataPtr, mp3DecoderState->bytes_left);
		if (offset < 0)
		{
			// синхро не найдено, очистить буфер и прочитать из файла следующую порцию
			mp3DecoderState->bytes_left = 0;
			StopTimeMeasurement();
			continue;
		}

		// указатель на начало фрейма
		mp3DecoderState->inputDataPtr += offset;
		// пропустить ненужные данные в буфере (если есть)
		mp3DecoderState->bytes_left -= offset;

		// проверка валидности фрейма (тип, версия)
		int err = MP3GetNextFrameInfo(mp3DecoderState->hMP3Decoder,
									  &mp3DecoderState->mp3FrameInfo,
									  mp3DecoderState->inputDataPtr);
		if (err < 0
#ifndef MONO_SUPPORT
			|| mp3DecoderState->mp3FrameInfo.nChans != 2	// работаем пока только со стерео
#endif
			|| mp3DecoderState->mp3FrameInfo.layer != 3)
		{
			// поиск следующего фрейма
			mp3DecoderState->bytes_left -= 2;
			mp3DecoderState->inputDataPtr += 2;
			offset = MP3FindSyncWord(mp3DecoderState->inputDataPtr, mp3DecoderState->bytes_left);
			if (offset < 0)
			{
				// следующее синхро не найдено, очистить буфер
				mp3DecoderState->bytes_left = 0;
				StopTimeMeasurement();
				continue;
			}
			else
			{
				// найден следующий фрейм, повторить цикл
				mp3DecoderState->inputDataPtr += offset;
				mp3DecoderState->bytes_left -= offset;
				StopTimeMeasurement();
				continue;
			}
		}

		// выбрать свободный буфер для декодированных данных
		short * outbuf = mp3DecoderState->outBuff[mp3DecoderState->outBuffPtr];
		mp3DecoderState->outBuffPtr = mp3DecoderState->outBuffPtr ? 0: 1;

		// декодирование фрейма
		DECODE_TEST_HIGH;
		err = MP3Decode(mp3DecoderState->hMP3Decoder, &mp3DecoderState->inputDataPtr, &mp3DecoderState->bytes_left, outbuf, 0);
		DECODE_TEST_LOW;
		if (err < 0)
		{
			//if (debug_mode.showDecoderInfo)
			//{
				xprintf("Ошибка (%d) декодирования фрейма %u, адрес чтения %u\r\n",
						(signed int)err,
						(unsigned int)mp3DecoderState->frameCNT,
						(unsigned int)mp3DecoderState->fileAddr);
			//}

			StopTimeMeasurement();

			// пропустить на 1 байт, чтобы не зависнуть на ошибочном фрейме
			// (если MP3Decode не заберёт ни одного байта из буфера inputBuf)
			if (mp3DecoderState->bytes_left > 0)
			{
				mp3DecoderState->bytes_left--;
				mp3DecoderState->inputDataPtr++;
			}
			break;
		}

		// обновить информацию о декодированном фрейме
		// нужно знать, сколько сэмплов декодировано и определить текущий битрейт
		MP3GetLastFrameInfo(mp3DecoderState->hMP3Decoder, &mp3DecoderState->mp3FrameInfo);

		// обновить частоту дискретизации
		if (mp3DecoderState->samprate != (uint32_t)mp3DecoderState->mp3FrameInfo.samprate)
		{
			mp3DecoderState->dac_srate.dac_cmd = DAC_SET_SRATE;
			mp3DecoderState->dac_srate.sample_rate = mp3DecoderState->mp3FrameInfo.samprate;
			CoPostMail(mp3DecoderState->dac_CmdMailbox, &mp3DecoderState->dac_srate);
			mp3DecoderState->samprate = mp3DecoderState->mp3FrameInfo.samprate;
		}

		DECODE_TEST_HIGH;

		// преобразовать данные в формат, понятный ЦАПу
#ifdef MONO_SUPPORT
		if (mp3DecoderState->mp3FrameInfo.nChans == 2)
		{
			// для стереомузыки только сместить шкалу
			for (uint32_t i = 0; i < (uint32_t)mp3DecoderState->mp3FrameInfo.outputSamps; i++)
			{
				outbuf[i] += 0x8000;	// или проинвертировать старший бит, что то же самое
			}
		}
		else
		{
			// для моно собрать семплы для левого и правого каналов из моно-сигнала
			// TODO переписать процедуру на асме, будет немного быстрее
			short * sptr = &outbuf[mp3DecoderState->mp3FrameInfo.outputSamps - 1];
			short * dptr = &outbuf[mp3DecoderState->mp3FrameInfo.outputSamps*2 - 1];
			for (uint32_t i = 0; i < (uint32_t)mp3DecoderState->mp3FrameInfo.outputSamps; i++)
			{
				uint16_t sample = *sptr--;
				*dptr-- = sample + 0x8000;
				*dptr-- = sample + 0x8000;
			}
		}

#else
		// TODO переписать процедуру на асме, будет немного быстрее
		for (uint32_t i = 0; i < mp3DecoderState->mp3FrameInfo.outputSamps; i++)
		{
			outbuf[i] += 0x8000;	// или проинвертировать старший бит, что то же самое
		}
#endif

		// отправить message ЦАПу
		mp3DecoderState->dac_data_portion.dac_cmd = DAC_DATA;
		mp3DecoderState->dac_data_portion.data_ptr = (uint32_t *)outbuf;
		mp3DecoderState->dac_data_portion.data_size = mp3DecoderState->mp3FrameInfo.outputSamps / mp3DecoderState->mp3FrameInfo.nChans;	// делим на 2 канала
		CoPostMail(mp3DecoderState->dac_CmdMailbox, &mp3DecoderState->dac_data_portion);
		DECODE_TEST_LOW;

		mp3DecoderState->frameCNT++;
		uint32_t time = StopTimeMeasurement();

		//if (debug_mode.showDecoderInfo && debug_mode.showFrameDecodeTime)
		//{
			xprintf("%u,%u\r\n", (unsigned int)mp3DecoderState->frameCNT, (unsigned int)time/100);
		//}
	}
}
