/*************************************************************************************************
 * @file		debugTask.h
 *
 * @brief		��������� ������ �������
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef DEBUG_H_
#define DEBUG_H_

//*-----------------------------------------------------------------------------------------------
//*			����
//*-----------------------------------------------------------------------------------------------

/// ��������� � ����������� �������
typedef struct
{
	uint32_t timer_ms;
	_Bool showDecoderInfo;
	_Bool showFrameDecodeTime;
	_Bool showPlayerInfo;
} debug_mode_t;


/// ��������� ��� ����� ���������� ������
typedef struct
{
	// �������� ��������
	uint32_t fileNumber;		// ����� �������� �����
	char fileName[13];			// ��� �������� �����
	fileTypes_t fileType;		// ��� �������� �����

	// �������� CPU
	uint32_t minCPUidle_mks;	// ����������� �������� CPU �� 1 �����
	uint32_t maxCPUidle_mks;	// ������������ �������� CPU �� 1 �����
	uint32_t averageCPUidle_mks;// ���������� ������� ��� ������� ������� �������� CPU �� 1 ����

	// ������
	uint32_t dacBuffLoss;		// ������� ���������� ������ ������������� ��� � �������� (������ �� ������)
} decodeStatistic_t;


//*-----------------------------------------------------------------------------------------------
//*			����������
//*-----------------------------------------------------------------------------------------------
extern debug_mode_t debug_mode;
extern decodeStatistic_t decodeStatistic;


//*-----------------------------------------------------------------------------------------------
//*			������� ������
//*-----------------------------------------------------------------------------------------------

// �������-������ ������ ����������� ���������� ���������
static inline void StartTimeMeasurement()
{
	TIM_Cmd(TIM5, DISABLE);
	debug_mode.timer_ms = 0;
	TIM5->CNT = 0;
	TIM_Cmd(TIM5, ENABLE);
}

// �������-������ ��������� ����������� ���������� ���������
// ���������� ����� ����� �������� start_timer() � stop_timer() � ���
static inline uint32_t GetCurrentTime()
{
	__disable_irq();
	uint32_t temp = debug_mode.timer_ms*1000 + TIM5->CNT;
	__enable_irq();
	return temp;
}

// ������� ������� � ������� ������� � ������� ������ StartTimeMeasurement()
static inline uint32_t StopTimeMeasurement()
{
	TIM_Cmd(TIM5, DISABLE);
	return debug_mode.timer_ms*1000 + TIM5->CNT;
}

// ������������� ����������� ������
void DebugModuleInit(void);

#endif /* DEBUG_H_ */
