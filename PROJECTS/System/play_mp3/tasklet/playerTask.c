/*************************************************************************************************
 * @file		playerTask.c
 *
 * @brief		Модуль главного проигрывателя
 *
 * @version		v1.0
 * @date		10.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"


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
#define PLAYER_TASK_STACK_SIZE	200


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
//static OS_STK pltask_stk[PLAYER_TASK_STACK_SIZE];	///< стек задачи
static void (*decoderDisposePtr)(void) = NULL;		/// указатель функции деинициализации декодера

// переменные используются для передачи Message
static cmd_t 		player_cmd;
static uint8_t 		dac_cmd;
static OS_EventID 	cmdEventMailbox;				///< управление проигрывателем

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
OS_EventID playerCtrlInit(void)
{
	decodeStatistic.fileNumber = 0;

	cmdEventMailbox = CoCreateMbox(EVENT_SORT_TYPE_FIFO);

	// инициализация задачи
	CoCreateTask(playerCtrlTask,
				(void *)0,
				PLAYER_TASK_PRIOR,
				&pltask_stk[PLAYER_TASK_STACK_SIZE-1],
				PLAYER_TASK_STACK_SIZE);

	return cmdEventMailbox;
}


//*-----------------------------------------------------------------------------------------------
/**			Задача-проигрыватель
 *
 *	@param pdata - указатель на доп. параметры (не используется)									*/
//*-----------------------------------------------------------------------------------------------
static void playerCtrlTask(void* pdata)
{
    pdata=pdata;
    //
	static FIL * file = NULL;
	static FILINFO * fileInfo = NULL;

	// смонтировать диск
	///FRESULT result = f_mount(0, &FATFS_Obj);
	disk_initialize(0);
    FRESULT result = f_mount(&FATFS_Obj, "0:", 0);
	if (result != FR_OK)
	{
		printf("Ошибка монтирования диска %d\r\n", result);
		stopError();
	}
	if (debug_mode.showPlayerInfo)
	{
		printf("Монтирование диска: успешно\r\n");
	}

	// открыть директорий 'music'
	char * dirName = "/\0";
	result = f_opendir(&dir, dirName);
	if (result != FR_OK)
	{
		printf("Невозможно открыть директорию; ошибка %d\r\n",(unsigned int)result);
		stopError();
	}
	if (debug_mode.showPlayerInfo)
	{
		printf("Директория %s открыта успешно\r\n", dirName);
	}
	memcpy(dirPath, dirName, strlen(dirName));

	playerState = STOPPED;

	// TODO здесь можно считать и отсортировать список файлов

	// запустить
	player_cmd = CMD_START;
	CoPostMail(cmdEventMailbox, &player_cmd);

	for(;;)
	{
		// работаем по событиям, поступающим на ящик cmdEventMailbox
		StatusType result;
		cmd_t * msg = CoPendMail(cmdEventMailbox, 20, &result);
		if (result != E_OK)
			continue;

		// автомат состояний
		switch (playerState)
		{
		// состояние "проигрывание"
		case PLAYING:
			// перехват сообщений о нажатии клавиш
			if (*msg == CMD_PAUSE)
			{
				if (debug_mode.showPlayerInfo)
				{
					printf("пауза\r\n");
				}
				pause();
			}
			else if (*msg == CMD_NEXT)
			{
				if (debug_mode.showPlayerInfo)
				{
					printf("следующий\r\n");
				}
				stop(file);
				fileInfo = next();
				start(fileInfo);
			}
			else if	((*msg == DECODE_ERROR) ||
					 (*msg == SONG_COMPLETE))
			{
				stop(file);
				fileInfo = next();
				start(fileInfo);
			}
			else {}

			break;

		// состояние "остановлено"
		case STOPPED:
			if (*msg == CMD_START)
			{
				if (debug_mode.showPlayerInfo)
				{
					printf("старт\r\n");
				}
				fileInfo = next();
				start(fileInfo);
			}
			else if  (*msg == CMD_NEXT)
			{
				if (debug_mode.showPlayerInfo)
				{
					printf("следующий\r\n");
				}
				fileInfo = next();
				start(fileInfo);
			}
			break;

		// состояние "пауза"
		case PAUSED:
			if (*msg == CMD_START)
			{
				if (debug_mode.showPlayerInfo)
				{
					printf("продолжить\r\n");
				}
				cont();
			}
			else if (*msg == CMD_NEXT)
			{
				if (debug_mode.showPlayerInfo)
				{
					printf("следующий\r\n");
				}
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
	CoTickDelay(500);
	DBG_HALT(PLAYER_HALT_ID);
	while(1)
	{
		LED_ON;
		CoTickDelay(100);
		LED_OFF;
		CoTickDelay(100);
	}
}


//*-----------------------------------------------------------------------------------------------
/**			Сделать паузу																		*/
//*-----------------------------------------------------------------------------------------------
static void pause(void)
{
	playerState = PAUSED;
	dac_cmd = DAC_PAUSE;
	CoPostMail(dac_CmdMailbox, &dac_cmd);
}


//*-----------------------------------------------------------------------------------------------
/**			Продолжить																			*/
//*-----------------------------------------------------------------------------------------------
static void cont(void)
{
	playerState = PLAYING;
	dac_cmd = DAC_CONTINUE;
	CoPostMail(dac_CmdMailbox, &dac_cmd);
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
	static uint8_t dac_cmd = DAC_STOP;
	CoPostMail(dac_CmdMailbox, &dac_cmd);

	// закрыть файл, если открыт
	if (file != NULL)
	{
		printf("Закрытие файла...");
		FRESULT result = f_close(file);
		if (result != FR_OK)
		{
			printf("Ошибка %d закрытия файла %s\r\n", result, currentFileName);
			stopError();
		}

		if (debug_mode.showPlayerInfo)
		{
			printf("Файл %s закрыт успешно\r\n", currentFileName);
		}
	}

	// напечатать статистику
	//print_EndStatistic();
}


//*-----------------------------------------------------------------------------------------------
/**			Перейти к воспроизведению следующего файла
 *
 * @return - указатель на структуру с информацией о файле типа FILINFO							*/
//*-----------------------------------------------------------------------------------------------
static FILINFO * next(void)
{
	static FILINFO fileInfo;

	decodeStatistic.fileNumber++;

	for(;;)
	{
		FRESULT result = f_readdir(&dir, &fileInfo);
		if (result != FR_OK)
		{
			printf("Ошибка следующего файла; ошибка %d",(unsigned int)result);
			stopError();
		}

		// если первый байт имени 0 - файлы закончились
		if (fileInfo.fname[0] == 0)
		{
			player_cmd = CMD_STOP;
			CoPostMail(cmdEventMailbox, &player_cmd);
			printf("Файлы закончились.\r\n");
			return NULL;
		}

		// отсекаем файлы нулевой длины - это обычно директории
		if (fileInfo.fsize == 0)
		{
		}
		else
			break;
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
	if (fileinfo == NULL)
	{
		playerState = STOPPED;
		return NULL;
	}

	static FIL file;

	// прочитать информацию о файле
	// memset(&songinfo, 0, sizeof(SONGINFO));
	// файл открывается, читается заголовок и закрывается
	// read_song_info_for_song((SONGFILE *)fileinfo->fname, &songinfo);

	// подготовить полный путь
	char * fullFileName = CoKmalloc(256);
	if (fullFileName == NULL)
	{
		printf("Ошибка выделения памяти");
		printf("Error in %s line %u\r\n", (uint8_t *)__FILE__, (unsigned int)__LINE__);
		stopError();
	}
	uint32_t len = strlen(dirPath);
	memcpy(fullFileName, dirPath, len);
	fullFileName[len] = '/';
	memcpy(&fullFileName[len + 1], fileinfo->fname, 13);

	// открыть файл
	FRESULT result = f_open(&file, fullFileName, FA_OPEN_EXISTING | FA_READ);
	CoKfree(fullFileName);
	if (result != FR_OK)
	{
		printf("Ошибка [%d] открытия файла %s\r\n", (unsigned int)result, fileinfo->fname);

		player_cmd = CMD_STOP;
		CoPostMail(cmdEventMailbox, &player_cmd);
		return &file;
	}
	else
	{
		printf("----------------------------------------\r\n");
		printf("Файл %s открыт (%d байт)\r\n", fileinfo->fname, (unsigned int)fileinfo->fsize);
	}
	// запомнить имя файла
	memcpy(currentFileName, fileinfo->fname, 13);
	memcpy(decodeStatistic.fileName, fileinfo->fname, 13);

	// проверить тип файла
	fileTypes_t fileType = get_filetype(fileinfo->fname);

	// если не совпадает с текущим, значит, нужно запустить новый проигрыватель
	// если другой проигрыватель был инициализирован, деинициализировать его
	if (currentFiletype != fileType)
	{
		if (decoderDisposePtr != NULL)
		{
			decoderDisposePtr();
			decoderDisposePtr = NULL;
		}
		playerState = STOPPED;
	}

	// инициализировать статистику
	decodeStatistic.dacBuffLoss = 0;
	// statisticInit(fileinfo);

	// инициализировать нужный декодер
	switch (fileType)
	{
	case MP3:
	{
		//print_StartStatistic();
		static decoder_param_t decoder_param;
		decoder_param.file = &file;
		decoder_param.dacCmdMailbox = dac_CmdMailbox;
		decoder_param.dacDataReqFlag = dac_DataReqFlag;
		decoder_param.playCmdMailBox = cmdEventMailbox;
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
		CoPostMail(cmdEventMailbox, &player_cmd);
		break;
	}

	decodeStatistic.fileType = currentFiletype;

	if (currentFiletype != UNKNOWN)
	{
		// запустить ЦАП
		dac_cmd = DAC_START;
		CoPostMail(dac_CmdMailbox, &dac_cmd);
	}

	playerState = PLAYING;
	return &file;
}

