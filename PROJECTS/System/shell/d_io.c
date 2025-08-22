#include "FreeRTOS.h"
#include "timers.h"

#include <string.h>
#include <stdlib.h>

#include "spi_io.h"
#include "xprintf.h"
#include "lib.h"

#include "d_io.h"

void pxCallbackFunction(TimerHandle_t xTimer);

#define DO8     GET_BIT_BB(GPIOC+12,6)
#define DO9     GET_BIT_BB(GPIOC+12,7)

void spi(char *s);
void spi_ini(char *s);

void pxCallbackFunction(TimerHandle_t xTimer);

#define DO8     GET_BIT_BB(GPIOC+12,6)
#define DO9     GET_BIT_BB(GPIOC+12,7)
#define PWR     GET_BIT_BB(GPIOB+12,8)

TimerHandle_t tmr=NULL;
int t_id;

void d_io_init(void)
{
static int is_inited=0;
    if(is_inited==0) {
        is_inited=1;
        gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
        DO8 = 0;
        DO9 = 0;
        //
        spi_io_Init();
        //
        // POWER (SOL)
        gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
        PWR=0;
        //
        // CNTPL
        gpio_init(GPIOA, GPIO_MODE_IPD, 0, GPIO_PIN_8);
        // FREQ
        gpio_init(GPIOB, GPIO_MODE_IPD, 0, GPIO_PIN_9);
        }
}

volatile uint32_t din;

void d_io(char *s)
{
char *pt=&s[5];
int n=1;
long d;
uint32_t t=10;
    //
    d_io_init();
    //
    if(xatoi(&pt, &d)) {
            n = d;
            if(xatoi(&pt, &d)) {
                t = d;
                }
            }
    if(t<1) t = 1;
    else if(t>5000) t=5000;
    //
    if(tmr == NULL) {
        tmr = xTimerCreate("0",
                    100,
                    pdTRUE, //const UBaseType_t uxAutoReload,
                    &t_id,
                    pxCallbackFunction );
        //
        xTimerStart(tmr, 10);
        }
    else {
        xTimerStop(tmr, 1000);
        xTimerChangePeriod(tmr, t, 10);
        xTimerStart(tmr, 10);
        }
    //
    for(;n>0;n--) {
        //xprintf("%d\n", n);
        DO8 = !DO8;
        DO9 = !DO8;
        //
        vTaskDelay(1000);
        }
    DO8 = 0;
    DO9 = 0;
}

void pxCallbackFunction(TimerHandle_t xTimer)
{
static uint8_t aaa;
static uint32_t s_din;
    //xprintf("timer %x\n", xTimer);
    LC_IO=0;
    usleep(5);
    LC_IO=1;
    //
    CS_IO=0;
    din = spi_io_RW(aaa);
    CS_IO=1;

    if(aaa<0xff) aaa++;
    else aaa=0;
    //
    if(s_din != din) {
        s_din=din;
        xprintf("din=%08x\n", din);
        }
}
