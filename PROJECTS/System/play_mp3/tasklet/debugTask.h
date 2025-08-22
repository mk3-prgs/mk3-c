/*************************************************************************************************
 * @file		debugTask.h
 *
 * @brief		Заголовок модуля отладки
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef DEBUG_H_
#define DEBUG_H_

//*-----------------------------------------------------------------------------------------------
//*			Типы
//*-----------------------------------------------------------------------------------------------

/// структура с отлалочными флагами
typedef struct
{
	uint32_t timer_ms;
	_Bool showDecoderInfo;
	_Bool showFrameDecodeTime;
	_Bool showPlayerInfo;
} debug_mode_t;


/// структура для сбора статистики работы
typedef struct
{
	// описание декодера
	uint32_t fileNumber;		// номер текущего файла
	char fileName[13];			// имя текущего файла
	fileTypes_t fileType;		// тип текущего файла

	// загрузка CPU
	uint32_t minCPUidle_mks;	// минимальная загрузка CPU за 1 фрейм
	uint32_t maxCPUidle_mks;	// максимальная загрузка CPU за 1 фрейм
	uint32_t averageCPUidle_mks;// накопитель времени для расчёта средней загрузки CPU за 1 трек

	// ошибки
	uint32_t dacBuffLoss;		// счётчик количества потерь синхронизации ЦАП и декодера (данные не готовы)
} decodeStatistic_t;


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
extern debug_mode_t debug_mode;
extern decodeStatistic_t decodeStatistic;


//*-----------------------------------------------------------------------------------------------
//*			Функции модуля
//*-----------------------------------------------------------------------------------------------

// функция-маркер начала измеряемого временного интервала
static inline void StartTimeMeasurement()
{
	TIM_Cmd(TIM5, DISABLE);
	debug_mode.timer_ms = 0;
	TIM5->CNT = 0;
	TIM_Cmd(TIM5, ENABLE);
}

// функция-маркер окончания измеряемого временного интервала
// возвращает время между вызовами start_timer() и stop_timer() в мкс
static inline uint32_t GetCurrentTime()
{
	__disable_irq();
	uint32_t temp = debug_mode.timer_ms*1000 + TIM5->CNT;
	__enable_irq();
	return temp;
}

// останов таймера и возврат времени с момента вызова StartTimeMeasurement()
static inline uint32_t StopTimeMeasurement()
{
	TIM_Cmd(TIM5, DISABLE);
	return debug_mode.timer_ms*1000 + TIM5->CNT;
}

// инициализация отладочного модуля
void DebugModuleInit(void);

#endif /* DEBUG_H_ */
