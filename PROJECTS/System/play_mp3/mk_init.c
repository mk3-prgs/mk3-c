#include <gd32f10x.h>
//#include "bitbanding.h"
#include <memory.h>

void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if(SysTick_Config(SystemCoreClock / 1000U)) {
        /* capture error */
        while (1) {}
    }
    /* configure the systick handler priority */
    //NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

//#define ALARM BIT_BAND_SRAM(GPIOA+12,GPIO_PIN_8)
extern uint32_t _evector;

void prvSetupHardware( void )
{
    /*
    uint32_t* src=(uint32_t*)0x08000000;
    uint32_t* dst=(uint32_t*)0x20000000;
    int len=0x200;
    int i;
    memset(dst, 0, len);
    for(i=0; i<(0x130/4); i++) *dst++ = *src++;
    *dst = 0xaaaa5555;
    */
    SystemInit();
    //
    ///nvic_vector_table_set(NVIC_VECTTAB_RAM, 0x00);
    //
    systick_config();
    //
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_AF);
    //
    gpio_deinit(GPIOA);
    gpio_deinit(GPIOB);
    gpio_deinit(GPIOC);
    gpio_deinit(GPIOD);
    gpio_afio_deinit();
    //
    /* Disable the Serial Wire Jtag Debug Port SWJ-DP */
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP, ENABLE);
    //
    ///gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); // ALARM< Power
    //ALARM=1;
    //
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    nvic_irq_enable(RTC_IRQn, 7, 0);
    //
    //nvic_irq_enable(TIMER3_IRQn, 15, 0);
    //nvic_irq_enable(TIMER2_IRQn, 14, 0);
}

