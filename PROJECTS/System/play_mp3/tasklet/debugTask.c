/*************************************************************************************************
 * @file		debugTask.c
 *
 * @brief		������ �������
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			������� ������
//*-----------------------------------------------------------------------------------------------
#include "includes.h"


//*-----------------------------------------------------------------------------------------------
//*			����������
//*-----------------------------------------------------------------------------------------------
debug_mode_t debug_mode;			// ���������� �����
decodeStatistic_t decodeStatistic;	// ���������� ����������


//*-----------------------------------------------------------------------------------------------
//*			��������� ������� ��� ��������� �������
//*-----------------------------------------------------------------------------------------------
// ���������� ���������� �������� �������
void TIM5_IRQHandler(void)
{
	TIM5->SR &= ~TIM_SR_UIF;
	debug_mode.timer_ms++;
}

// ��������� ������� ��������


//*-----------------------------------------------------------------------------------------------
/**			������������� ���������																*/
//*-----------------------------------------------------------------------------------------------
void DebugModuleInit(void)
{
	debug_mode.showDecoderInfo = true;
	debug_mode.showFrameDecodeTime = false;
	debug_mode.showPlayerInfo = true;

	// ��������� ������� ��� ������� �������
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	// ��������� ������� �������� �������
	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);

	// Time base configuration
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_Prescaler = RCC_ClocksStatus.SYSCLK_Frequency/1000000-1;	// 1 ��� ������������
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM5, ENABLE);

	// ������� ��� ������������
	TIM_ITConfig(TIM5, TIM_DIER_UIE, ENABLE);
	// ����������
	NVIC_SetPriority(TIM5_IRQn, 10);
	NVIC_EnableIRQ(TIM5_IRQn);
}

