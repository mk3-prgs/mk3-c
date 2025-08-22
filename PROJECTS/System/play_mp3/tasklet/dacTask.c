/*************************************************************************************************
 * @file		dacTask.c
 *
 * @brief		Драйвер ЦАП
 *
 * @version		v1.2
 * @date		11.09.2013
 * @author		Mike Smith
 *
 * @note
 * 		Задача DAC для синхронизации с декодером использует mailbox и флаг:
 * 1. dacCmdMailbox - через него принимает команды управления режимом работы и указатель
 *    на буфер для воспроизведения;
 * 2. dacDataNeedFlag - флаг-признак необходимости подготовки новых сэмплов.
 *
 * @note
 * 		Модуль не имеет собственного буфера воспроизведения, указатель на внешний буфер с данными
 * передаётся командой DAC_DATA через ящик dacCmdMailbox.
 *
 *  	Формат одного сэмпла в буфере:
 *  [0] - младший байт слова для левого канала;
 *  [1] - старший байт слова для левого канала;
 *  [2] - младший байт слова для правого канала;
 *  [3] - старший байт слова для правого канала;
 *  ...
 *
 ************************************************************************************************/


//*-----------------------------------------------------------------------------------------------
//*			Подключаемые внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"
#include "dacTask.h"

//*-----------------------------------------------------------------------------------------------
//*			Параметры
//*-----------------------------------------------------------------------------------------------
#define DAC_TASK_STK_SIZE	48


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
static uint32_t * currentSampleBufferPtr;		///< указатель на начало следующего буфера
static uint32_t currentSampleBufferSize;		///< количество сэмплов в следующем буфере
static uint32_t currentSrate = 0;				///< текущий битрейт
//static OS_STK dac_TaskStk[DAC_TASK_STK_SIZE];	///< стек задачи
static dac_state_t dac_State;					///< переменная состояния модуля ЦАП

int     dac_DataReqFlag;	///< этот флаг устанавливается при необходимости декодирования новой порции данных
QueueHandle_t dac_quiue;		///< сюда поступают команды управления ЦАП


//*-----------------------------------------------------------------------------------------------
//*			Параметры и настройки периферии
//*-----------------------------------------------------------------------------------------------
#define DMA_TIMER				TIMER3
#define DMA_TIMER_IRQn			TIMER3_IRQn
//#define DMA_TIMER_IRQ_vector	TIMER3_IRQHandler
#define DMA_RCC_APB1Periph		RCU_TIMER3
#define DMA_TIMER_DBG_STOP		DBGMCU_CR_DBG_TIM4_STOP

#define AUDIO_DMA_ch			DMA1_Channel7
#define AUDIO_DMA_TC_Flag		DMA1_FLAG_TC7
#define AUDIO_DMA_IRQn			DMA1_Channel7_IRQn

#define DAC_Trigger_TRGO		DAC_Trigger_T4_TRGO
#define DAC_DMA_IRQHandler		DMA1_Channel7_IRQHandler


//*-----------------------------------------------------------------------------------------------
//*			Прототипы локальных функций
//*-----------------------------------------------------------------------------------------------
static void DAC_Task(void* pdata);
static int32_t DAC_SetSrate(uint32_t srate);


//*-----------------------------------------------------------------------------------------------
//*			Звуковые сэмплы для отладки
//*-----------------------------------------------------------------------------------------------
const uint32_t silence_data[] =
{
	0x80008000,	0x80008000,
	0x80008000,	0x80008000,
	0x80008000,	0x80008000,
	0x80008000,	0x80008000
};


//*-----------------------------------------------------------------------------------------------
/**			Инициализация задачи-ЦАП
 *
 * @param keyEventMailbox_ptr																	*/
//*-----------------------------------------------------------------------------------------------
void DAC_TaskInit(void)
{
	xprintf("Иициализация модуля ЦАП...");

	// флаги и майлбоксы
	dac_DataReqFlag = CoCreateFlag(0, 0);
	///dac_CmdMailbox = CoCreateMbox(EVENT_SORT_TYPE_FIFO);
	dac_quiue = xQueueCreate(4, sizeof(uint8_t));

	// локальные переменные
	dac_State = DAC_STOPPED;
    currentSampleBufferSize = sizeof(silence_data)/4;
    currentSampleBufferPtr = (uint32_t *)silence_data;

    // конфигурирование таймера частоты дискретизации
	// в дальнейшем частота переполнения таймера будет подстроена в соответствии с требуемым битрейтом
	///RCC_APB1PeriphClockCmd(DMA_RCC_APB1Periph, ENABLE);
	rcu_periph_clock_enable(RCU_TIMER3);

    ///TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    ///TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    ///TIM_TimeBaseStructure.TIM_Prescaler = 0x0000;
    ///TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    ///TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    ///TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
    // в качестве альтернативы можно воспользоваться функцией инициализации по умолчанию:
    //TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    ///TIM_TimeBaseInit(DMA_TIMER, &TIM_TimeBaseStructure);
    //
    timer_parameter_struct tpar;
    tpar.prescaler=0;
    tpar.period=0xFFFF;
    tpar.clockdivision    = TIMER_CKDIV_DIV1;
    tpar.alignedmode      = TIMER_COUNTER_EDGE;
    tpar.counterdirection = TIMER_COUNTER_UP;
    tpar.repetitioncounter = 0;
    timer_init(TIMER3, &tpar);

    // сформировать TRGO по переполнению таймера
    // TRGO будет использоваться для запуска преобразования ЦАП
    ///TIM_SelectOutputTrigger(DMA_TIMER, TIM_TRGOSource_Update);

    // формировать запрос DMA по переполнению таймера
    ///TIM_DMACmd(DMA_TIMER, TIM_DMA_Update, ENABLE);

    // остановить таймер, если ядро процессора остановлено
    ///DBGMCU->CR |= DMA_TIMER_DBG_STOP;
/*
	// инициализация канала DMA
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(AUDIO_DMA_ch);
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & DAC->DHR12LD;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&silence_data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = sizeof(silence_data)/4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	// DMA_Mode_Circular
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(AUDIO_DMA_ch, &DMA_InitStructure);

	// сбросить флаг прерывания
	DMA_ClearFlag(AUDIO_DMA_TC_Flag);
	// разрешить прерывание по окончании передачи буфера
	DMA_ITConfig(AUDIO_DMA_ch, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(AUDIO_DMA_IRQn);

	// инициализация ЦАП
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	// настроить пины ЦАП как аналоговые входы (рекомендация STM)
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 + GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	DAC_InitTypeDef DAC_InitStructure;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = 0;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);

	// запустить таймер частоты дискретизации
    DAC_SetSrate(48000);

	// инициализация задачи
    OS_TID result = CoCreateTask(DAC_Task, (void *)0, DAC_TASK_PRIOR, &dac_TaskStk[DAC_TASK_STK_SIZE - 1], DAC_TASK_STK_SIZE);

    if (result == E_CREATE_FAIL)
    	printf("ошибка\r\n");
    else
    	printf("выполнено\r\n");
*/
}


//*-----------------------------------------------------------------------------------------------
/**			обработчик прерывания по окончании передачи буфера DMA								*/
//*-----------------------------------------------------------------------------------------------
void DAC_DMA_IRQHandler(void)
{
	DAC_DMA_TEST_HIGH;
	DMA1->IFCR = AUDIO_DMA_TC_Flag;	// сбросить флаг прерывания
	///CoEnterISR(); 					// вход в прерывание

	AUDIO_DMA_ch->CCR &= (uint16_t)(~DMA_CCR1_EN);	// отключить DMA

	// проверить состояние флага
	StatusType result = CoAcceptSingleFlag(dac_DataReqFlag);
	// если флаг сброшен - есть новые данные
	if(result == E_FLAG_NOT_READY) {
		AUDIO_DMA_ch->CNDTR = currentSampleBufferSize;			// перегрузить счётчик
		AUDIO_DMA_ch->CMAR = (uint32_t)currentSampleBufferPtr;	// и указатель
		AUDIO_DMA_ch->CCR |= DMA_CCR1_EN;						// разрешить dma

		///isr_SetFlag(dac_DataReqFlag);	// установить флаг запроса данных
		LED_OFF;						// погасить светодиод ошибки
        }
	// если новых данных ещё нет - повторить передачу текущего буфера
	else {
		// если при отсутствии новых сэмплов нужно проиграть тишину -
		// инициализировать указатель массивом silence_data:
	    // currentSampleBufferSize = sizeof(silence_data)/4;
	    // currentSampleBufferPtr = (uint32_t *)silence_data;

		// иначе будет повторно воспроизведен прерыдущий буфер
		AUDIO_DMA_ch->CNDTR = currentSampleBufferSize;
		AUDIO_DMA_ch->CMAR = (uint32_t)currentSampleBufferPtr;
		AUDIO_DMA_ch->CCR |= DMA_CCR1_EN;

		LED_ON;		// включить светодиод индикации ошибки

		decodeStatistic.dacBuffLoss++;
        }
	///CoExitISR();	// выход из прерывания
	DAC_DMA_TEST_LOW;
}


//*-----------------------------------------------------------------------------------------------
/**			Настроить частоту дискретизации
 *
 * @param srate - частота, Гц
 * @return - результат выполнения команды (0 - ок, -1 - ошибка)									*/
//*-----------------------------------------------------------------------------------------------
static int32_t DAC_SetSrate(uint32_t srate)
{
	if (currentSrate == srate)
		return 0;

	/*
	uint16_t arr;
	// 72 МГц
	switch(srate)
	{
	case 8000:
	case 11025:
		arr = 9000;	// 72000/8 = 9000
		break;
	case 8021:
		arr = 8976;	// 72000/8,021 = 8976,437
		break;
	case 32000:
		arr = 2250;	// 72000/32 = 2250
		break;
	case 44100:
		arr = 1633;	// 72000/44,1 = 1632,65
		break;
	case 48000:
		arr = 1500;	// 72000/48 = 1500
		break;
	case 88200:
		arr = 816;	// 72000/88,2 = 816,32
		break;
	case 96000:
		arr = 750;	// 72000/96 = 750
		break;
	case 56250:
		arr = 1280;
		break;
	default:
		return -1;
	}
	*/

	// аналитический расчёт
	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	uint32_t arr = RCC_ClocksStatus.SYSCLK_Frequency / srate;

	DMA_TIMER->ARR = (uint16_t)arr-1;

	currentSrate = srate;
	return 0;
}


//*-----------------------------------------------------------------------------------------------
/**			Главная задача ЦАП
 *
 * @param pdata не используется																	*/
//*-----------------------------------------------------------------------------------------------
static void DAC_Task(void * pdata)
{
    pdata=pdata;
	StatusType result=0;

    // главный цикл задачи
    for(;;)
    {
    	// непрерывно ждать новых писем
    	void * msg = CoPendMail (dac_CmdMailbox, 0, &result);
    	if(result != E_OK) { continue; }

		dac_cmd_t cmd = *(dac_cmd_t *)msg;
		switch(cmd)
		{
		case DAC_DATA:
			{
				__disable_irq();
				// сохранить параметры нового буфера данных
				currentSampleBufferPtr = ((dac_data_portion_t *)msg)->data_ptr;
				currentSampleBufferSize = ((dac_data_portion_t *)msg)->data_size;
				__enable_irq();

				// сбросить флаг ЦАП
				CoClearFlag(dac_DataReqFlag);
				break;
			}
		case DAC_SET_SRATE:
			{
				DAC_SetSrate(((dac_srate_t *)msg)->sample_rate);
				break;
			}
		case DAC_START:
			{
				TIM_Cmd(DMA_TIMER, DISABLE); // остановить таймер дискретизации
				AUDIO_DMA_ch->CCR &= (uint16_t)(~DMA_CCR1_EN);	// отключить DMA

				// инициализировать регистры DMA
			    currentSampleBufferSize = sizeof(silence_data)/4;
			    currentSampleBufferPtr = (uint32_t *)silence_data;
				AUDIO_DMA_ch->CNDTR = currentSampleBufferSize;
				AUDIO_DMA_ch->CMAR = (uint32_t)currentSampleBufferPtr;

				AUDIO_DMA_ch->CCR |= DMA_CCR1_EN;	// разрешить DMA
				TIM_Cmd(DMA_TIMER, ENABLE); // запустить таймер дискретизации

				isr_SetFlag(dac_DataReqFlag);
				dac_State = DAC_WORK;
				break;
			}
		case DAC_PAUSE:
			{
				TIM_Cmd(DMA_TIMER, DISABLE);
				dac_State = DAC_PAUSED;
				break;
			}
		case DAC_CONTINUE:
			{
				TIM_Cmd(DMA_TIMER, ENABLE);
				dac_State = DAC_WORK;
				break;
			}
		case DAC_STOP:
			{
				TIM_Cmd(DMA_TIMER, DISABLE);
				CoClearFlag(dac_DataReqFlag);
				dac_State = DAC_STOPPED;
				break;
			}
		}
    }
}
