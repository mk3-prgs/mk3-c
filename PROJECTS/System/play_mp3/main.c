#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "diskio.h"
#include <string.h>
#include "xprintf.h"
//
//#include "mtc16201/pwm_out.h"
//
extern uint32_t rtc_io;
//--------------------------------------------------------------
extern SemaphoreHandle_t x_io;
void shell(void* arg);
//--------------------------------------------------------------
void play(void* arg);
void test_dac(void*);

void prvSetupHardware( void );

uint32_t get_fattime(void)
{
	return	0;
}

void x_out_done(void)
{

}

void hard_fault_ini(void);

char cur_path[32]={ 0 };

int sys_main(void)
{
	prvSetupHardware();

	//hard_fault_ini();

	x_io = xSemaphoreCreateMutex(); //xSemaphoreCreateBinary();
	xSemaphoreGive(x_io);
    //
	xTaskCreate(play,   "play",    configMINIMAL_STACK_SIZE*8, NULL, 2, (xTaskHandle*)NULL);
    //
	vTaskStartScheduler();
    //
	for(;;){}
}

/*-----------------------------------------------------------*/
volatile unsigned long ulHighFrequencyTimerTicks;

volatile uint32_t tm_spi0;
volatile uint32_t tm_rtc;
volatile uint32_t t_ms;

void Tick_1ms(void)
{
    if(t_ms)    t_ms--;
    if(tm_rtc)  tm_rtc--;
    if(tm_spi0) tm_spi0--;
    //if(tm_spi2) tm_spi2--;
    //
    //if(t_10ms) t_10ms--;
    //else {
    //    t_10ms = 10;
    //    b_10ms=1;
    //    }
    //if(t_10ms == 5) b_SysTick=1;
    disk_timerproc();
    ulHighFrequencyTimerTicks++;
}

//
void vApplicationTickHook(void)
{
    Tick_1ms();
}

//
void vApplicationStackOverflowHook(TaskHandle_t task, char* s)
{
    xprintf("task: 0x%08x %s\n", task, s);
}

//
unsigned getRunTimeCounterValue(void)
{
    return ulHighFrequencyTimerTicks;
}

//
void configureTimerForRunTimeStats(void)
{
    ulHighFrequencyTimerTicks = 0UL;
}
