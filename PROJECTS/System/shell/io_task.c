#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "dfdata.h"

#define DECLARE
#include "mk2-c.h"

#include "spi_io.h"
#include "freq_meter.h"
#include "io_task.h"
#include "xprintf.h"
//
void sys_mount(void);
void init_console(void);
//
#ifdef OLD_BOARD
void init_3kn(void);
uint8_t kn_input(void);
#endif // OLD_BOARD

//static uint8_t s_3kn;
volatile uint8_t b_3kn;
QueueHandle_t kn_queue=NULL;

void kn_task(void* arg);
//
void io_task(void* arg)
{
TickType_t l_time;
TickType_t freq;
//
uint32_t tmp;
    //
    // POWER (SOL)
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    PWR_IO=0;
    // CNTRL
    gpio_init(GPIOA, GPIO_MODE_IPD, 0, GPIO_PIN_8);
    //
    spi_io_Init();
    d_out.d=0;
    spi_io_RW(0);
    //
    freq = 10;
    /*
    sec=0;
    min=0;
    regul=0;
    dt05sec=0;
    dt09sec=0;
    //
    stmin=0;
    stsec=0;
    */
    l_time = xTaskGetTickCount();
    //
    init_console();
    //sys_mount();
    //restore_data(0);
#ifdef OLD_BOARD
    init_3kn();
#endif // OLD_BOARD
    /*
    kn_queue = xQueueCreate(8, sizeof(uint8_t));
    if(kn_queue != NULL) {
        xTaskCreate( kn_task, "kn_task",  configMINIMAL_STACK_SIZE*2, NULL, 2, ( xTaskHandle * ) NULL);
        }
    */
    for(;;) {
        vTaskDelayUntil(&l_time, freq);
        //
        if(blk_fwdg) fwdgt_counter_reload();
        //
        spi_io_reInit();
        //==================================================
        LC_IO=1;
        //
        CS_IO=0;
        tmp = spi_io_RW(d_out.d);
        CS_IO=1;
        //
        LC_IO=0;
        d_inp.d = (tmp);
#ifdef OLD_BOARD
        b_3kn = kn_input();
#else
        //b_3kn = 0x07 & (~(tmp>>5));
        b_3kn = 0x07 & (~(tmp>>5));
#endif // OLD_BOARD
        /*
        int a = (FREQ != 0);
        int b = (CNTRL != 0);
        a &= 0x01;
        b &= 0x01;
        */
        /*
        if(s_3kn != b_3kn) {
            //xprintf("b_3kn=%02x\n", b_3kn);
            xQueueSend(kn_queue, (void*)&b_3kn, (TickType_t)0);
            s_3kn = b_3kn;
            }
        */
        /*
        if(FREQ) dVent = 1;
        else dVent = 0;
        */
        }
}

void alr(char *s)
{
char *pt=&s[4];
long t;
int d;
    if(xatoi(&pt, &t)) {
        d=t;
        }
    else d = 0;
    //
    Alarm = d;
}

void pwr(char *s)
{
char *pt=&s[3];
long t;
int d;
    if(xatoi(&pt, &t)) {
        d=t;
        }
    else d = 0;
    //
    if(d) d=1;
    PWR_IO = d;
}

void inp(char *s)
{
    xprintf("dd=0x%02x\n", d_inp.d);
}

void out(char *s)
{
char *pt=&s[3];
long t;
//
    if(xatoi(&pt, &t)) {
        d_out.d=t;
        }
    //
    xprintf("dd=0x%02x\n", d_out.d);
}
