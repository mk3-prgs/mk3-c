#include "mk2-c.h"
#include "dfdata.h"
#include "freq_meter.h"
//
#include "xprintf.h"
#include "lib.h"
//
volatile uint32_t dt_cap;
static volatile uint32_t s_cap;
static volatile uint32_t old_cap, new_cap;

static volatile uint32_t fm_ovf;
static volatile uint32_t ovf;

uint32_t r_Nob(void)
{
uint64_t n;
    //
    if((dt_cap > 0)&&(dNob>0)) {
        n = ((SystemCoreClock / 72) * 60) / dt_cap;
        n /= dNob;
        }
    else n=0;
    //
    n = ogran(n,0,999);
    //
    return(n);
    //return(dt_cap);
}

void fm_init(void)
{
    gpio_init(FREQ_P, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, FREQ_I);
    //
    rcu_periph_clock_enable(RCU_TIMER3);
    timer_deinit(TIMER3);
//
    timer_parameter_struct tpar;
    timer_struct_para_init(&tpar);
    //
    tpar.prescaler=288-1;
    tpar.period=0xffff;
    tpar.clockdivision = TIMER_CKDIV_DIV1;
    tpar.alignedmode   = TIMER_COUNTER_EDGE;
    tpar.counterdirection=TIMER_COUNTER_UP;
    tpar.repetitioncounter = 0;
    //
    timer_init(TIMER3, &tpar);
    //
    timer_ic_parameter_struct ip;
    timer_channel_input_struct_para_init(&ip);
    ip.icfilter=0;
    ip.icpolarity=TIMER_IC_POLARITY_RISING;
    ip.icprescaler=TIMER_IC_PSC_DIV1;
    ip.icselection=TIMER_IC_SELECTION_DIRECTTI;
    timer_input_capture_config(TIMER3, TIMER_CH_3, &ip);
    //
    dt_cap = old_cap = new_cap = 0;
    fm_ovf = 0;
    //
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER3);
    //
    // Разрешаем прерывания
    timer_interrupt_flag_clear(TIMER3, TIMER_INT_UP | TIMER_INT_CH3);
    timer_interrupt_enable(TIMER3, TIMER_INT_UP | TIMER_INT_CH3);
    //
    timer_enable(TIMER3);
    //
    ///xprintf("tm3 enabled!\r\n");
    //
    NVIC_SetPriority(TIMER3_IRQn , 7);
    NVIC_EnableIRQ(TIMER3_IRQn);
}

void TIMER3_IRQHandler(void)
{
    if(timer_interrupt_flag_get(TIMER3, TIMER_INT_UP) == SET) {
        timer_interrupt_flag_clear(TIMER3, TIMER_INT_UP);
        fm_ovf += 0x10000;
        ovf++;
        //
        if(ovf>4) {
            fm_ovf = dt_cap = old_cap = new_cap = 0;
            ovf=5;
            }
        }
    //
    if(timer_interrupt_flag_get(TIMER3, TIMER_INT_CH3) == SET) {
        timer_interrupt_flag_clear(TIMER3, TIMER_INT_CH3);
        //
        old_cap = new_cap;
        new_cap = fm_ovf + timer_channel_capture_value_register_read(TIMER3, TIMER_CH_3); ///TIM_GetCapture4(TIM4);
        //
        ovf=0;
        if(new_cap >= old_cap) {
            s_cap += (new_cap - old_cap);
            s_cap -= dt_cap;
            dt_cap = s_cap / 4;
            }
        }
}

