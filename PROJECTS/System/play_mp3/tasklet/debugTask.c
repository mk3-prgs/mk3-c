/*************************************************************************************************
 * @file		debugTask.c
 *
 * @brief		Модуль отладки
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
debug_mode_t debug_mode;			// отладочные флаги
decodeStatistic_t decodeStatistic;	// отладочная статистика


//*-----------------------------------------------------------------------------------------------
//*			Служебные функции для измерения времени
//*-----------------------------------------------------------------------------------------------
// обработчик прерывания счётного таймера
void TIM5_IRQHandler(void)
{
	TIM5->SR &= ~TIM_SR_UIF;
	debug_mode.timer_ms++;
}

// остальные функции инлайные


//*-----------------------------------------------------------------------------------------------
/**			Инициализация отладчика																*/
//*-----------------------------------------------------------------------------------------------
void DebugModuleInit(void)
{
	debug_mode.showDecoderInfo = true;
	debug_mode.showFrameDecodeTime = false;
	debug_mode.showPlayerInfo = true;

	// настройка таймера для отсчёта времени
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	// запросить текущую тактовую частоту
	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);

	// Time base configuration
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_Prescaler = RCC_ClocksStatus.SYSCLK_Frequency/1000000-1;	// 1 МГц тактирование
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM5, ENABLE);

	// событие при переполнении
	TIM_ITConfig(TIM5, TIM_DIER_UIE, ENABLE);
	// прерывание
	NVIC_SetPriority(TIM5_IRQn, 10);
	NVIC_EnableIRQ(TIM5_IRQn);
}

