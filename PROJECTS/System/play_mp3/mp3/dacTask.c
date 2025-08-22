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
#include "tasksPrior.h"

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
///static OS_STK dac_TaskStk[DAC_TASK_STK_SIZE];	///< стек задачи
static dac_state_t dac_State;					///< переменная состояния модуля ЦАП

uint32_t  dac_DataReqFlag;		///< этот флаг устанавливается при необходимости декодирования новой порции данных
QueueHandle_t dac_Cmd;		///< сюда поступают команды управления ЦАП

//*-----------------------------------------------------------------------------------------------
//*			Параметры и настройки периферии
//*-----------------------------------------------------------------------------------------------
/*
#define DMA_TIMER				TIMER3
#define DMA_TIMER_IRQn			TIMER3_IRQn
#define DMA_TIMER_IRQ_vector	TIMER3_IRQHandler
#define DMA_RCC_APB1Periph		RCU_TIMER3
//#define DMA_TIMER_DBG_STOP		DBGMCU_CR_DBG_TIM4_STOP

#define AUDIO_DMA_ch			DMA1_Channel7
#define AUDIO_DMA_TC_Flag		DMA1_FLAG_TC7
#define AUDIO_DMA_IRQn			DMA1_Channel7_IRQn

#define DAC_Trigger_TRGO		DAC_Trigger_T4_TRGO
#define DAC_DMA_IRQHandler		DMA0_Channel6_IRQHandler
*/
#define DAC0_ADDR (DAC+0x0CU)
//*-----------------------------------------------------------------------------------------------
//*			Прототипы локальных функций
//*-----------------------------------------------------------------------------------------------
void DAC_Task(void* pdata);
static int32_t DAC_SetSrate(uint32_t srate);


//*-----------------------------------------------------------------------------------------------
//*			Звуковые сэмплы для отладки
//*-----------------------------------------------------------------------------------------------
const uint32_t silence_data[] =
{
	0x80000070,
	0x80100160,
	0x80200250,
	0x80300340,
	0x80400430,
	0x80500520,
	0x80600610,
	0x80700700
};

extern QueueHandle_t mp3_Cmd;

//*-----------------------------------------------------------------------------------------------
/**			Инициализация задачи-ЦАП
 *
 * @param keyEventMailbox_ptr																	*/
//*-----------------------------------------------------------------------------------------------
void DAC_TaskInit(void)
{
	xprintf("Иициализация модуля ЦАП...\n");

	// флаги и майлбоксы
	dac_DataReqFlag = 0;
	///dac_CmdMailbox = CoCreateMbox(EVENT_SORT_TYPE_FIFO);
	dac_Cmd = xQueueCreate(10, sizeof(dac_data_portion_t));
    xprintf("DAC xQueue: %x\n", dac_Cmd);
    //
	// локальные переменные
	dac_State = DAC_STOPPED;
    currentSampleBufferSize = sizeof(silence_data)/4;
    currentSampleBufferPtr = (uint32_t *)silence_data;

    // конфигурирование таймера частоты дискретизации
	// в дальнейшем частота переполнения таймера будет подстроена в соответствии с требуемым битрейтом
	/*
	RCC_APB1PeriphClockCmd(DMA_RCC_APB1Periph, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0x0000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
    // в качестве альтернативы можно воспользоваться функцией инициализации по умолчанию:
    //TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseInit(DMA_TIMER, &TIM_TimeBaseStructure);
    */
    rcu_periph_clock_enable(RCU_TIMER3);
    timer_deinit(TIMER3);
    /*
    timer_parameter_struct tpar;
    //
    tpar.prescaler=1;
    tpar.period=0xffff;
    tpar.clockdivision=TIMER_CKDIV_DIV1;
    //tpar.alignedmode      = TIMER_COUNTER_EDGE;
    tpar.counterdirection = TIMER_COUNTER_UP;
    tpar.repetitioncounter = 0x0000;
    //
    timer_init(TIMER3, &tpar);
    */
    timer_prescaler_config(TIMER3, 0x00, TIMER_PSC_RELOAD_UPDATE);
    timer_autoreload_value_config(TIMER3, 200);
    // сформировать TRGO по переполнению таймера
    // TRGO будет использоваться для запуска преобразования ЦАП
    ///TIM_SelectOutputTrigger(DMA_TIMER, TIM_TRGOSource_Update);
    timer_master_output_trigger_source_select(TIMER3, TIMER_TRI_OUT_SRC_UPDATE);
    // формировать запрос DMA по переполнению таймера
    ///TIM_DMACmd(DMA_TIMER, TIM_DMA_Update, ENABLE);
    //timer_channel_dma_request_source_select(TIMER3, TIMER_DMAREQUEST_UPDATEEVENT);
    //
    timer_dma_enable(TIMER3, TIMER_DMA_UPD);

    nvic_irq_enable(TIMER3_IRQn, 8, 0);
    timer_interrupt_enable(TIMER3, TIMER_INT_UP);
    timer_enable(TIMER3);
    /// остановить таймер, если ядро процессора остановлено
    /// DBGMCU->CR |= DMA_TIMER_DBG_STOP;

	// инициализация канала DMA
	///RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	rcu_periph_clock_enable(RCU_DMA0);
    /*
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
    */
    dma_parameter_struct dma_struct;
    dma_struct.periph_addr  = (uint32_t)&xDAC->DHR12LD;
    dma_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_struct.memory_addr  = (uint32_t)&silence_data;
    dma_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;
    dma_struct.number       = sizeof(silence_data)/4;
    dma_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(DMA0, DMA_CH6, &dma_struct);

    dma_circulation_disable(DMA0, DMA_CH6);

	// сбросить флаг прерывания
	///DMA_ClearFlag(AUDIO_DMA_TC_Flag);
	dma_flag_clear(DMA0, DMA_CH6, DMA_INTF_GIF);
    dma_flag_clear(DMA0, DMA_CH6, DMA_INTF_FTFIF);
    dma_flag_clear(DMA0, DMA_CH6, DMA_INTF_HTFIF);
    dma_flag_clear(DMA0, DMA_CH6, DMA_INTF_ERRIF);
	// разрешить прерывание по окончании передачи буфера
	///DMA_ITConfig(AUDIO_DMA_ch, DMA_IT_TC, ENABLE);
	dma_interrupt_enable(DMA0, DMA_CH6, DMA_INT_FTF);
	///NVIC_EnableIRQ(AUDIO_DMA_IRQn);
    nvic_irq_enable(DMA0_Channel6_IRQn, 10, 0);
    //
	// инициализация ЦАП
	///RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	///RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	rcu_periph_clock_enable(RCU_DAC);
    // настроить пины ЦАП как аналоговые входы (рекомендация STM)
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    //
    dac_deinit();
    /*
	DAC_InitTypeDef DAC_InitStructure;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = 0;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    //
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	//
	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);
	*/
    dac_trigger_source_config(DAC0, DAC_TRIGGER_T3_TRGO);
    dac_trigger_enable(DAC0);
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);
    dac_dma_enable(DAC0);
    //
    dac_trigger_source_config(DAC1, DAC_TRIGGER_T3_TRGO);
    dac_trigger_enable(DAC1);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC1);
    dac_dma_enable(DAC1);
    //
    dac_concurrent_enable();
    //
    dac_enable(DAC0);
    dac_enable(DAC1);
	// запустить таймер частоты дискретизации
    DAC_SetSrate(48000);

	// инициализация задачи
	/*
    OS_TID result = CoCreateTask(DAC_Task, (void *)0, DAC_TASK_PRIOR, &dac_TaskStk[DAC_TASK_STK_SIZE - 1], DAC_TASK_STK_SIZE);

    if (result == E_CREATE_FAIL)
    	printf("ошибка\r\n");
    else
    	printf("выполнено\r\n");
    */
    xTaskCreate(DAC_Task, "DAC_Task", configMINIMAL_STACK_SIZE, NULL, DAC_TASK_PRIOR, (xTaskHandle*)NULL);
}

void TIMER3_IRQHandler(void)
{
    //TEST5=1;
    timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
    //TEST5=0;
}

//*-----------------------------------------------------------------------------------------------
/**			обработчик прерывания по окончании передачи буфера DMA								*/
//*-----------------------------------------------------------------------------------------------
///void DAC_DMA_IRQHandler(void)
void DMA0_Channel6_IRQHandler(void)
{
BaseType_t xHigherPriorityTaskWoken;
	TEST5 = 1;
	//DMA1->IFCR = AUDIO_DMA_TC_Flag;	// сбросить флаг прерывания
	//CoEnterISR(); 					// вход в прерывание
    //uint32_t flag = DMA_INTF(DMA0) >> ((DMA_CH6) * 4U);
    DMA_INTC(DMA0) = (DMA_INT_FTF | DMA_INT_HTF | DMA_INT_ERR) << ((DMA_CH6) * 4U);

	///AUDIO_DMA_ch->CCR &= (uint16_t)(~DMA_CCR1_EN);	// отключить DMA
	//dma_interrupt_disable(DMA0, DMA_CH6, DMA_INT_FTF);
    dma_channel_disable(DMA0, DMA_CH6);

	// проверить состояние флага
	///StatusType result = CoAcceptSingleFlag(dac_DataReqFlag);
	uint32_t result = dac_DataReqFlag;

	// если флаг сброшен - есть новые данные
	if(result == 0) {
		///AUDIO_DMA_ch->CNDTR = currentSampleBufferSize;			// перегрузить счётчик
		dma_transfer_number_config(DMA0, DMA_CH6, currentSampleBufferSize);

		///AUDIO_DMA_ch->CMAR = (uint32_t)currentSampleBufferPtr;	// и указатель
		dma_memory_address_config(DMA0, DMA_CH6, (uint32_t)currentSampleBufferPtr);

		///AUDIO_DMA_ch->CCR |= DMA_CCR1_EN;						// разрешить dma
		dma_channel_enable(DMA0, DMA_CH6);

		///isr_SetFlag(dac_DataReqFlag);	// установить флаг запроса данных
		dac_DataReqFlag = 1;
		xQueueSendFromISR(mp3_Cmd, &dac_DataReqFlag, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken) {
            //portYIELD_FROM_ISR();
            portYIELD();
            }
        }

	// если новых данных ещё нет - повторить передачу текущего буфера
	else {
		// если при отсутствии новых сэмплов нужно проиграть тишину -
		// инициализировать указатель массивом silence_data:
	    currentSampleBufferSize = sizeof(silence_data)/4;
	    currentSampleBufferPtr = (uint32_t *)silence_data;

		// иначе будет повторно воспроизведен прерыдущий буфер
		///AUDIO_DMA_ch->CNDTR = currentSampleBufferSize;
		dma_transfer_number_config(DMA0, DMA_CH6, currentSampleBufferSize);
		///AUDIO_DMA_ch->CMAR = (uint32_t)currentSampleBufferPtr;
		dma_memory_address_config(DMA0, DMA_CH6, (uint32_t)currentSampleBufferPtr);
		///AUDIO_DMA_ch->CCR |= DMA_CCR1_EN;
		dma_channel_enable(DMA0, DMA_CH6);
        }
	///CoExitISR();	// выход из прерывания
	TEST5 = 0;
}


//*-----------------------------------------------------------------------------------------------
/**			Настроить частоту дискретизации
 *
 * @param srate - частота, Гц
 * @return - результат выполнения команды (0 - ок, -1 - ошибка)									*/
//*-----------------------------------------------------------------------------------------------
static int32_t DAC_SetSrate(uint32_t srate)
{
	if(currentSrate == srate) return 0;

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
	uint32_t arr = SystemCoreClock / srate;

	///DMA_TIMER->ARR = (uint16_t)arr-1;
	timer_autoreload_value_config(TIMER3, arr-1);

	currentSrate = srate;
	return 0;
}


//*-----------------------------------------------------------------------------------------------
/**			Главная задача ЦАП
 *
 * @param pdata не используется																	*/
//*-----------------------------------------------------------------------------------------------
void DAC_Task(void * pdata)
{
pdata=pdata;
dac_data_portion_t message;
dac_data_portion_t* msg = &message;
int ret;
    // главный цикл задачи
    for(;;)
    {
    	// непрерывно ждать новых писем
    	///void * msg = CoPendMail (dac_CmdMailbox, 0, &result);
    	///if(result != E_OK) { continue; }
        TEST7 = !TEST7;

    	if(dac_Cmd != 0) {
            ret = xQueueReceive(dac_Cmd, msg, 100);
            if(ret != pdPASS) { continue; }
            }
        else {
            vTaskDelay(50);
            continue;
            }

		dac_cmd_t cmd = (dac_cmd_t)msg->dac_cmd;
		///xprintf("DAC_cmd = %d\n", cmd);

		switch(cmd)
		{
		case DAC_DATA:
			{
				__disable_irq();
				// сохранить параметры нового буфера данных
				currentSampleBufferPtr  = msg->data_ptr; //((dac_data_portion_t *)msg)->data_ptr;
				currentSampleBufferSize = msg->data_size; //((dac_data_portion_t *)msg)->data_size;
				// сбросить флаг ЦАП
				dac_DataReqFlag = 0;
				__enable_irq();
				break;
			}
		case DAC_SET_SRATE:
			{
				DAC_SetSrate(((dac_srate_t *)msg)->sample_rate);
				break;
			}
		case DAC_START:
			{
				///TIM_Cmd(DMA_TIMER, DISABLE); // остановить таймер дискретизации
				timer_disable(TIMER3);
				///AUDIO_DMA_ch->CCR &= (uint16_t)(~DMA_CCR1_EN);	// отключить DMA
				dma_channel_disable(DMA0, DMA_CH6);
                //
				// инициализировать регистры DMA
			    currentSampleBufferSize = sizeof(silence_data)/4;
			    currentSampleBufferPtr = (uint32_t *)silence_data;
				///AUDIO_DMA_ch->CNDTR = currentSampleBufferSize;
				dma_transfer_number_config(DMA0, DMA_CH6, currentSampleBufferSize);
				///AUDIO_DMA_ch->CMAR = (uint32_t)currentSampleBufferPtr;
                dma_memory_address_config(DMA0, DMA_CH6, (uint32_t)currentSampleBufferPtr);
				//
				///AUDIO_DMA_ch->CCR |= DMA_CCR1_EN;	// разрешить DMA
				dma_channel_enable(DMA0, DMA_CH6);
				///TIM_Cmd(DMA_TIMER, ENABLE); // запустить таймер дискретизации
                timer_enable(TIMER3);

				dac_DataReqFlag=1;
				dac_State = DAC_WORK;
				//
				TEST6 = !TEST6;
				//
				break;
			}
		case DAC_PAUSE:
			{
				///TIM_Cmd(DMA_TIMER, DISABLE);
				timer_disable(TIMER3);
				dac_State = DAC_PAUSED;
				break;
			}
		case DAC_CONTINUE:
			{
				///TIM_Cmd(DMA_TIMER, ENABLE);
				timer_enable(TIMER3);
				dac_State = DAC_WORK;
				break;
			}
		case DAC_STOP:
			{
				///TIM_Cmd(DMA_TIMER, DISABLE);
				timer_disable(TIMER3);
				//CoClearFlag(dac_DataReqFlag);
				dac_DataReqFlag=0;
				dac_State = DAC_STOPPED;
				break;
			}
		}
    }
}
