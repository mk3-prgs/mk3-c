#include "pwm_out.h"
#include "mtc16201.h"
//
static int led;
static int sat;

int d_sat(int d)
{
    sat = d/10;
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, sat);
    return(10*sat);
}

int d_led(int d)
{
    led = d/10;
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_2, led);
    return(10*led);
}


void set_sat(uint16_t d)
{
    sat = d/10;
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, sat);
}

void set_led(uint16_t d)
{
    led = d/10;
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_2, led);
}

void PWM_Init(void)
{
    //gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);

    ///gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_0);
    //gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_1);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_0 |
                                                          GPIO_PIN_1);

    rcu_periph_clock_enable(RCU_TIMER2);

    timer_deinit(TIMER2);

    timer_parameter_struct tpar;
    tpar.prescaler=2;
    tpar.period=1000;
    tpar.clockdivision=TIMER_CKDIV_DIV1;
    tpar.alignedmode       = TIMER_COUNTER_EDGE;
    tpar.counterdirection=TIMER_COUNTER_UP;
    tpar.repetitioncounter = 0;
    timer_init(TIMER2, &tpar);

    timer_oc_parameter_struct t_oc;
    timer_channel_output_struct_para_init(&t_oc);
    //
    t_oc.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    t_oc.outputstate  = TIMER_CCX_ENABLE;
    t_oc.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    t_oc.outputnstate = TIMER_CCXN_ENABLE;
    t_oc.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    t_oc.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    //timer_channel_output_config(TIMER2, TIMER_CH_1, &t_oc); // PWM light
    timer_channel_output_config(TIMER2, TIMER_CH_2, &t_oc);
    timer_channel_output_config(TIMER2, TIMER_CH_3, &t_oc);

  ///TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
  ///TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    //timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, 10);
    //timer_channel_output_mode_config(TIMER2, TIMER_CH_1, TIMER_OC_MODE_PWM1);
    //timer_channel_output_shadow_config(TIMER2, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_2, 5000);
    timer_channel_output_mode_config(TIMER2, TIMER_CH_2, TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER2, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);

    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, 9200);
    timer_channel_output_mode_config(TIMER2, TIMER_CH_3, TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER2, TIMER_CH_3, TIMER_OC_SHADOW_DISABLE);

    //timer_primary_output_config(TIMER2, ENABLE);

    //TIM_ARRPreloadConfig(TIM3, ENABLE);
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER2);
    /* auto-reload preload enable */
    //timer_channel_output_state_config(TIMER2, TIMER_CH_1, ENABLE);
    ///timer_channel_output_state_config(TIMER2, TIMER_CH_2, ENABLE);
    timer_channel_output_state_config(TIMER2, TIMER_CH_3, ENABLE);
    //
    ///timer_interrupt_enable(TIMER2, TIMER_INT_UP);
    //
    timer_enable(TIMER2);
    //
}
/*
volatile uint32_t ovf_2=0;

void TIMER2_IRQHandler(void)
{
    if(timer_interrupt_flag_get(TIMER2, TIMER_INT_UP) == SET) {
        timer_interrupt_flag_clear(TIMER2, TIMER_INT_UP);

        ovf_2++;
        }
}
*/
