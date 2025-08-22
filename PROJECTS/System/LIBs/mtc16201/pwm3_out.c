#include "pwm3_out.h"

void set_sol(uint16_t d)
{
    //if(d> 8800) d=8800;
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_2, d);
}

void set_p_sol(uint16_t d)
{
    TIMER_CAR(TIMER3) = (uint16_t)d;
}

void PWM3_Init(void)
{
    //gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);

    rcu_periph_clock_enable(RCU_TIMER3);

    timer_deinit(TIMER3);

    timer_parameter_struct tpar;

    tpar.prescaler=(27-1);
    tpar.period=5000;
    tpar.clockdivision=TIMER_CKDIV_DIV1;
    tpar.alignedmode       = TIMER_COUNTER_EDGE;
    tpar.counterdirection=TIMER_COUNTER_UP;
    tpar.repetitioncounter = 0;
    timer_init(TIMER3, &tpar);

    timer_oc_parameter_struct t_oc;
    timer_channel_output_struct_para_init(&t_oc);
    //
    t_oc.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    t_oc.outputstate  = TIMER_CCX_ENABLE;
    t_oc.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    t_oc.outputnstate = TIMER_CCXN_ENABLE;
    t_oc.ocidlestate  = TIMER_OC_IDLE_STATE_HIGH;
    t_oc.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;

    timer_channel_output_config(TIMER3, TIMER_CH_2, &t_oc);

    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_2, 1);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_2, TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);

    //timer_primary_output_config(TIMER2, ENABLE);

    //TIM_ARRPreloadConfig(TIM3, ENABLE);
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER3);
    /* auto-reload preload enable */
    timer_channel_output_state_config(TIMER3, TIMER_CH_2, ENABLE);
    //
    timer_interrupt_enable(TIMER3, TIMER_INT_UP);
    //
    timer_enable(TIMER3);
    //
    NVIC_SetPriority(TIMER3_IRQn , 7);
    NVIC_EnableIRQ(TIMER3_IRQn);
    //
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,  GPIO_PIN_8);
    //
}

//#define TEST1  GET_BIT_BB(GPIOB + 12, 11)
//#define TEST0  GET_BIT_BB(GPIOB + 12, 10)

//extern volatile int a_en;

void TIMER3_IRQHandler(void)
{
    if(timer_interrupt_flag_get(TIMER3, TIMER_INT_UP) == SET) {
        timer_interrupt_flag_clear(TIMER3, TIMER_INT_UP);

        //a_en=1;
        //adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);

        //TEST1 = !TEST1;
        //TEST0 = 1;
        }
}
//
/*
void pwm3_test(void* arg)
{
TickType_t l_t;
const TickType_t freq = 100;
int pwm=0;
int key=1;
    //
    PWM3_Init();
    set_sol(9999);
    //
    l_t = xTaskGetTickCount();
    vTaskDelayUntil(&l_t, freq);

    for(;;) {
        vTaskDelayUntil(&l_t, freq);
        //
        set_sol(pwm);
        //
        if(key==0) {
            if(pwm<9700) pwm+=200;
            else { key=1; }
            }
        else {
            if(pwm>300) pwm-=200;
            else { key=0; }
            }
        }
}
*/
