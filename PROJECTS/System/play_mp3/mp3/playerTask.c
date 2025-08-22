//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"
#include "dacTask.h"
#include "playerTask.h"
#include "mp3Task.h"
#include "tasksPrior.h"
#include "diskio.h"
//*-----------------------------------------------------------------------------------------------
//*			Локальные прототипы
//*-----------------------------------------------------------------------------------------------
static void playerCtrlTask(void* pdata);
static FIL * start(FILINFO * fileinfo);
static void stop(FIL * file);
static FILINFO * next(void);
static void pause(void);
static void cont(void);
static void stopError(void);


//*-----------------------------------------------------------------------------------------------
//*			Константы
//*-----------------------------------------------------------------------------------------------
// размер стека
#define PLAYER_TASK_STACK_SIZE	1024


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
static void (*decoderDisposePtr)(void) = NULL;		/// указатель функции деинициализации декодера

// переменные используются для передачи Message
static cmd_t 		player_cmd;
static uint8_t 		dac_cmd;

//static OS_EventID 	cmdEventMailbox;				///< управление проигрывателем
QueueHandle_t cmdEvent=0;
extern QueueHandle_t dac_Cmd;		    ///< сюда поступают команды управления ЦАП

// файловые структуры
FATFS	FATFS_Obj;
DIR		dir;
char	dirPath[128];			// текущий директорий
char	currentFileName[13];	// имя текущего файла

player_state_t playerState = NO_INIT;		// переменная состояния модуля
fileTypes_t	currentFiletype = UNKNOWN;		// тип текущего декодера


//*-----------------------------------------------------------------------------------------------
/**			Инициализация главного модуля проигрывателя
 *
 * @param keyEventMailbox - возвращает указатель на используемый почтовый ящик для управления	*/
//*-----------------------------------------------------------------------------------------------
QueueHandle_t* playerCtrlInit(void)
{
	//cmdEventMailbox = CoCreateMbox(EVENT_SORT_TYPE_FIFO);
	cmdEvent = xQueueCreate(10, sizeof(cmd_t));
    xprintf("xQueue: %x\n", cmdEvent);

    xTaskCreate(playerCtrlTask, "playCtrl", configMINIMAL_STACK_SIZE*4, NULL, PLAYER_TASK_PRIOR, (xTaskHandle*)NULL);
	return(&cmdEvent);
}

#define TEST5     GET_BIT_BB(GPIOC+12,5)

//*-----------------------------------------------------------------------------------------------
/**			Задача-проигрыватель
 *
 *	@param pdata - указатель на доп. параметры (не используется)									*/
//*-----------------------------------------------------------------------------------------------
void playerCtrlTask(void* pdata)
{
    pdata=pdata;
static FIL * file = NULL;
static FILINFO * fileInfo = NULL;
static cmd_t message;
    //
	// смонтировать диск
	disk_initialize(0);
    FRESULT result = f_mount(&FATFS_Obj, "0:", 0);
	if(result != FR_OK) {
		xprintf("Ошибка монтирования диска %d\n", result);
		stopError();
        }
	else {
		xprintf("Монтирование диска: успешно\n");
        }

	// открыть директорий 'music'
	char * dirName = "/\0";
	result = f_opendir(&dir, dirName);
	if(result != FR_OK) {
		xprintf("Невозможно открыть директорию; ошибка %d\n",(unsigned int)result);
		stopError();
        }
	else {
		xprintf("Директория %s открыта успешно\n", dirName);
        }
	memcpy(dirPath, dirName, strlen(dirName));

	playerState = STOPPED;

	// TODO здесь можно считать и отсортировать список файлов

	// запустить
	player_cmd = CMD_START;

	///CoPostMail(cmdEventMailbox, &player_cmd);

    cmd_t* msg = &message;
    int ret=0;

	for(;;) {
		// работаем по событиям, поступающим на ящик cmdEventMailbox
		/// StatusType result;
		/// cmd_t * msg = CoPendMail(cmdEventMailbox, 20, &result);
		/// if(result != E_OK) continue;
        /* xQueueReceive( xQueue, &( pxRxedMessage ), ( TickType_t ) 10 ) */
        TEST3 = !TEST3;
        if(cmdEvent != 0) {
            ret = xQueueReceive(cmdEvent, msg, 10);
            if(ret != pdPASS) { continue; }
            }
        else {
            vTaskDelay(50);
            continue;
            }
        //
        xprintf("playerState=%d msg=%d [0x%08x]\n", playerState, *msg, msg);
        //
		// автомат состояний
		switch(playerState) {
		// состояние "проигрывание"
		case PLAYING:
			// перехват сообщений о нажатии клавиш
			if(*msg == CMD_PAUSE) {
				xprintf("пауза\r\n");
				pause();
				playerState = PAUSED;
                }
			else if(*msg == CMD_NEXT) {
				xprintf("следующий\r\n");
				stop(file);
				fileInfo = next();
				start(fileInfo);
                }
			else if((*msg == DECODE_ERROR) || (*msg == SONG_COMPLETE)) {
				stop(file);
				fileInfo = next();
				start(fileInfo);
                }
			break;

		// состояние "остановлено"
		case STOPPED:
			if(*msg == CMD_START) {
				xprintf("старт\r\n");
				fileInfo = next();
				start(fileInfo);
				playerState = PLAYING;
                }
			else if(*msg == CMD_NEXT) {
				xprintf("следующий\r\n");
				fileInfo = next();
				start(fileInfo);
				playerState = PLAYING;
                }
			break;

		// состояние "пауза"
		case PAUSED:
			if(*msg == CMD_START) {
				xprintf("продолжить\r\n");
				cont();
                }
			else if(*msg == CMD_NEXT) {
				xprintf("следующий\r\n");
				stop(file);
				fileInfo = next();
				file = start(fileInfo);
                }
			break;
		default:
			playerState = STOPPED;
			stop(file);
			break;
		}
		//
	}
}


/*================================================================================================
//		далее служебные процедуры
================================================================================================*/

//*-----------------------------------------------------------------------------------------------
/**			аварийная остановка																	*/
//*-----------------------------------------------------------------------------------------------
static void stopError(void)
{
	while(1) {
		TEST1=1;
		vTaskDelay(100);
		TEST1=0;;
		vTaskDelay(100);
        }
}


//*-----------------------------------------------------------------------------------------------
/**			Сделать паузу																		*/
//*-----------------------------------------------------------------------------------------------
static void pause(void)
{
	playerState = PAUSED;
	dac_cmd = DAC_PAUSE;

	///CoPostMail(dac_CmdMailbox, &dac_cmd);
}


//*-----------------------------------------------------------------------------------------------
/**			Продолжить																			*/
//*-----------------------------------------------------------------------------------------------
static void cont(void)
{
	playerState = PLAYING;
	dac_cmd = DAC_CONTINUE;
	///CoPostMail(dac_CmdMailbox, &dac_cmd);
}


//*-----------------------------------------------------------------------------------------------
/**			Остановить вопроизведение
 *
 * @param file - указатель на дескриптор файла (используется для его закрытия)					*/
//*-----------------------------------------------------------------------------------------------
static void stop(FIL * file)
{
	// остановить проигрыватель
	playerState = STOPPED;

	// остановить ЦАП
	///static uint8_t dac_cmd = DAC_STOP;
	///CoPostMail(dac_CmdMailbox, &dac_cmd);

	// закрыть файл, если открыт
	if(file != NULL) {
		xprintf("Закрытие файла...");
		FRESULT result = f_close(file);
		if(result != FR_OK) {
			xprintf("Ошибка %d закрытия файла %s\r\n", result, currentFileName);
			stopError();
            }
		else {
			xprintf("Файл %s закрыт успешно\r\n", currentFileName);
            }
        }
}


//*-----------------------------------------------------------------------------------------------
/**			Перейти к воспроизведению следующего файла
 *
 * @return - указатель на структуру с информацией о файле типа FILINFO							*/
//*-----------------------------------------------------------------------------------------------
static FILINFO * next(void)
{
	static FILINFO fileInfo;

	for(;;) {
		FRESULT result = f_readdir(&dir, &fileInfo);
		if(result != FR_OK) {
			xprintf("Ошибка следующего файла; ошибка %d",(unsigned int)result);
			stopError();
            }

		// если первый байт имени 0 - файлы закончились
		if(fileInfo.fname[0] == 0) {
			player_cmd = CMD_STOP;
			///CoPostMail(cmdEventMailbox, &player_cmd);
			xprintf("Файлы закончились.\r\n");
			return NULL;
            }

		// отсекаем файлы нулевой длины - это обычно директории
		if(fileInfo.fsize == 0) {}
		else {
			break;
            }
        }

	return &fileInfo;
}


//*-----------------------------------------------------------------------------------------------
/**			Запустить воспроизведение файла
 *
 * @param fileinfo - указатель на описание файла (нужно его имя)
 * @return - дескриптор открытого файла															*/
//*-----------------------------------------------------------------------------------------------
static FIL * start(FILINFO * fileinfo)
{
	if(fileinfo == NULL) {
		playerState = STOPPED;
		return NULL;
        }

	static FIL file;

	// прочитать информацию о файле
	// memset(&songinfo, 0, sizeof(SONGINFO));
	// файл открывается, читается заголовок и закрывается
	// read_song_info_for_song((SONGFILE *)fileinfo->fname, &songinfo);

	// подготовить полный путь
	char * fullFileName = pvPortMalloc(256);
	if(fullFileName == NULL) {
		xprintf("Ошибка выделения памяти");
		xprintf("Error in %s line %u\r\n", (uint8_t *)__FILE__, (unsigned int)__LINE__);
		stopError();
        }
	uint32_t len = strlen(dirPath);
	memcpy(fullFileName, dirPath, len);
	fullFileName[len] = '/';
	memcpy(&fullFileName[len + 1], fileinfo->fname, 13);

	// открыть файл
	FRESULT result = f_open(&file, fullFileName, FA_OPEN_EXISTING | FA_READ);
	vPortFree(fullFileName);
	if(result != FR_OK) {
		xprintf("Ошибка [%d] открытия файла %s\r\n", (unsigned int)result, fileinfo->fname);
		player_cmd = CMD_STOP;
		///CoPostMail(cmdEventMailbox, &player_cmd);
		return &file;
        }
	else {
		xprintf("----------------------------------------\r\n");
		xprintf("Файл %s открыт (%d байт)\r\n", fileinfo->fname, (unsigned int)fileinfo->fsize);
        }
	// запомнить имя файла
	memcpy(currentFileName, fileinfo->fname, 13);

	// проверить тип файла
	fileTypes_t fileType = get_filetype(fileinfo->fname);

	// если не совпадает с текущим, значит, нужно запустить новый проигрыватель
	// если другой проигрыватель был инициализирован, деинициализировать его
	if(currentFiletype != fileType) {
		if(decoderDisposePtr != NULL) {
			decoderDisposePtr();
			decoderDisposePtr = NULL;
            }
		playerState = STOPPED;
        }

    xprintf("file_type: %d name: %s\n", fileType, fileinfo->fname);
	// инициализировать нужный декодер
	switch(fileType)
	{
	case MP3:
	{
		//print_StartStatistic();
		static decoder_param_t decoder_param;
		decoder_param.file = &file;

		decoder_param.pdacDataReqFlag = &dac_DataReqFlag;
		decoder_param.dacCmdMailBox  = dac_Cmd;
		decoder_param.mp3CmdMailBox  = mp3_Cmd;
		decoder_param.playCmdMailBox = cmdEvent;

		decoderDisposePtr = mp3_player_init(&decoder_param);
		currentFiletype = MP3;
		break;
	}

	case MOD:
	case AAC:
	case MP4:
	default:
		currentFiletype = UNKNOWN;
		player_cmd = CMD_NEXT;
		///CoPostMail(cmdEventMailbox, &player_cmd);
		break;
	}

	if(currentFiletype != UNKNOWN) {
		// запустить ЦАП
		dac_cmd = DAC_START;
		///CoPostMail(dac_CmdMailbox, &dac_cmd);
        }

	playerState = PLAYING;
	return &file;
}

