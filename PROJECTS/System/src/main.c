#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "diskio.h"
#include <string.h>
//
//#include "mtc16201/pwm_out.h"
#include "main.h"
#include "xprintf.h"
#include "usart.h"
#include "diskio.h"
//
FATFS FatFs[2];
static int sys_mounted=0;

void sys_mount(void)
{
    if(sys_mounted==0) {
        disk_initialize(0);
        f_mount(&FatFs[0], "0:", 0);
        //
#ifndef OLD_BOARD
        disk_initialize(1);
        f_mount(&FatFs[1], "1:", 0);
#endif // OLD_BOARD
        //
        sys_mounted=1;
        }
}
//
extern SemaphoreHandle_t x_io;
//
static int con_inited=0;
void init_console(void)
{
    if(con_inited==0) {
        x_io = xSemaphoreCreateMutex(); //xSemaphoreCreateBinary();
        xSemaphoreGive(x_io);
        //
        ///       Консоль
        usart_init(115200U);
        xdev_out(x_putc);
        xdev_in(x_getc);
        con_inited=1;
        xprintf("\n\n");
        }
}

//--------------------------------------------------------------
void io_task(void* arg);
void shell(void* arg);
void file_io(void* arg);
//void press(void* arg);
//void sht(void *arg);
//--------------------------------------------------------------

void prvSetupHardware( void );

void net_pool(void *arg);

void ili_init(char *s);
void max6675(void* p);

void start_tasks(void* arg)
{
    // wait RTC registers synchronized flag set
    rtc_register_sync_wait();
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    //
    //uart2_Init();
    ///ili_init("ili_init");
    //
    ///xTaskCreate( io_task, "io_task",  configMINIMAL_STACK_SIZE*4, NULL, 7, ( xTaskHandle * ) NULL);
    xTaskCreate( shell,   "shell",    configMINIMAL_STACK_SIZE*6, NULL, 2, ( xTaskHandle * ) NULL);
	xTaskCreate( file_io, "file_io",  configMINIMAL_STACK_SIZE*4, NULL, 6, ( xTaskHandle * ) NULL);
	///xTaskCreate( max6675, "max6675",  configMINIMAL_STACK_SIZE*4, NULL, 3, ( xTaskHandle * ) NULL);
	//
    //xTaskCreate( sht, "sht",      configMINIMAL_STACK_SIZE*2, NULL, 8, ( xTaskHandle * ) NULL);
    //
    xTaskHandle th=NULL;
    uint32_t *ps= (void*)0x08040000;
    void (*p)(void* arg);
    p = (void*)*(ps);
    if((uint32_t)p != 0xffffffff) {
        //xTaskCreate( p, "startup",  configMINIMAL_STACK_SIZE*2, NULL, 4, &th);
        }
    //
    vTaskDelete(NULL);
}

int sys_main(void)
{
	prvSetupHardware();

//void *p = malloc(256);
/*
setvbuf(stdin, NULL, _IONBF, 0);
setvbuf(stdout, NULL, _IONBF, 0);
setvbuf(stderr, NULL, _IONBF, 0);
//
#define	_IOFBF	0		// setvbuf should set fully buffered
#define	_IOLBF	1		// setvbuf should set line buffered
#define	_IONBF	2		// setvbuf should set unbuffered

SET(LD_FLAGS
        -Wl,-u,vfprintf;
        -Wl,-u,_printf_float;
        -Wl,-u,_scanf_float;
        "другие_флаги")
*/
    //
    xTaskCreate( start_tasks, "",  configMINIMAL_STACK_SIZE*2, NULL, 7, ( xTaskHandle * ) NULL);
    ///
    //gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    //gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    ///
	vTaskStartScheduler();
    //
	for(;;){}
}

/*-----------------------------------------------------------*/
volatile unsigned long ulHighFrequencyTimerTicks;

volatile uint32_t tm_spi0;
volatile uint32_t tm_spi_io;
volatile uint32_t tm_rtc;
volatile uint32_t t_ms;

//#define TEST  GET_BIT_BB(GPIOB+12,11)

void Tick_1ms(void)
{
    if(t_ms)    t_ms--;
    if(tm_rtc)  tm_rtc--;
    if(tm_spi0) tm_spi0--;
    if(tm_spi_io) tm_spi_io--;
    disk_timerproc();
    ulHighFrequencyTimerTicks++;
    //
    //TEST = !TEST;
}

//
void vApplicationTickHook(void)
{
    Tick_1ms();
}

//
void vApplicationStackOverflowHook( TaskHandle_t task, char* s)
{
    (void) task;
    (void) s;
    taskDISABLE_INTERRUPTS();
    for (;;);
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
