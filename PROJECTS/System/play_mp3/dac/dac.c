#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdint.h>
#include <string.h>
#include "xprintf.h"
#include "usart.h"
#include "ff.h"

#include "mtc16201/pwm3_out.h"

#include "bit_field.h"
#define TEST7     GET_BIT_BB(GPIOC+12,7)
#define TEST6     GET_BIT_BB(GPIOC+12,6)
#define TEST5     GET_BIT_BB(GPIOC+12,5)
#define TEST4     GET_BIT_BB(GPIOC+12,4)

#define CONVERT_NUM          (128)
#define DAC0_R8DH_ADDRESS    (0x40007410)

//#define DAC1_R12DH              REG32(DAC + 0x14U)   /*!< DAC1 12-bit right-aligned data holding register */
//#define DAC1_L12DH              REG32(DAC + 0x18U)   /*!< DAC1 12-bit left-aligned data holding register */
//#define DAC1_R8DH               REG32(DAC + 0x1CU)   /*!< DAC1 8-bit right-aligned data holding register */

#define DAC1_DATA    (0x40007414U)

void _delay_us(uint32_t us);
void adc_dma_config(void);
void ADC_init(void);
void dac_config(void);
void dac_dma_config(void);
void timer5_config(void);
void timer6_config(void);

void hex_dump(uint32_t addr, uint16_t* bf, int len)
{
int i;
    xprintf("\n");
    for(i=0; i<len; i++) {
        if((i%16)==0) { xprintf("\n%08x: ", addr); addr+=16; }
        xprintf("%04x ", bf[i]);
        }
    xprintf("\n");
}

void format_cmd(char *cmd, int numb)
{
int i;
    for(i=0; i<numb; i++) {
        if((*cmd=='\r')||(*cmd=='\n')||(*cmd=='\0')) { *cmd='\0'; break; }
        cmd++;
        }
}

#define LIN_LEN 1024
uint16_t lin_arr[LIN_LEN];

void test_dac(void*)
{
TickType_t l_t;
const TickType_t freq = 250;
int i;
uint32_t val = 70;
char cmd[64];
    /// Консоль
	usart_init();
	xdev_out(x_putc);
	xdev_in(x_getc);
	//
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	//
    xprintf("dac_dma_config()\n");
    dac_dma_config();
    xprintf("dac_config()\n");
    dac_config();
    xprintf("timer5_config()\n");
    timer5_config();
    //timer6_config();
    l_t = xTaskGetTickCount();
    vTaskDelayUntil(&l_t, freq);

    vTaskPrioritySet(NULL, 5);

    for(i=0; i<LIN_LEN; i++) {
        lin_arr[i] = ((0x1000 / LIN_LEN)*i)<<4;
        }
    hex_dump(0, lin_arr, LIN_LEN);

    for(;;) {
        ///vTaskDelayUntil(&l_t, freq);
        //
        //dac_data_set(DAC0, DAC_ALIGN_12B_R, adc0);
        //dac_software_trigger_enable(DAC0);
        //
        //dac_data_set(DAC1, DAC_ALIGN_12B_R, adc1);
        //dac_software_trigger_enable(DAC1);
        //
        xprintf("\r\n# ");
        memset(cmd, 0, 64);
        xgets(cmd, 63);
        //
        //TEST4 = !TEST4;
        //
        xprintf("\n");
        format_cmd(cmd, 64);
        ///xprintf("cmd: [%s]\n", cmd);
        //
        if(!strncmp(cmd, "init", 4)) {
            xprintf("\n");
            }
        else if(!strncmp(cmd, "val", 3)) {
            long l;
            char *pt=&cmd[4];
            if(xatoi(&pt, &l)) {
                val=l;
                }
            val &= 0xffff;
            xprintf("val=%u\n", val);
            timer_autoreload_value_config(TIMER5, val);
            timer_autoreload_value_config(TIMER6, val);
            xprintf("\n");
            }
        else if(!strcmp(cmd, "quit")) {
            break;
            }
        }
}

/*
void dac_config(void)
{
    rcu_periph_clock_enable(RCU_DAC);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    //
    dac_deinit();
    // configure the DAC0
    dac_trigger_source_config(DAC0, DAC_TRIGGER_SOFTWARE);
    dac_trigger_enable(DAC0);
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_disable(DAC0);

    // enable DAC0 for DAC0
    dac_enable(DAC0);
    //
    // configure the DAC1
    dac_trigger_source_config(DAC1, DAC_TRIGGER_SOFTWARE);
    dac_trigger_enable(DAC1);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_disable(DAC1);

    // enable DAC0 for DAC1
    dac_enable(DAC1);
}

*/
void dac_config(void)
{
    rcu_periph_clock_enable(RCU_DAC);
    //
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    //
    dac_deinit();
    //
    //===========================================================
    /* configure the DAC0 */
    dac_trigger_source_config(DAC0, DAC_TRIGGER_T5_TRGO);
    dac_trigger_enable(DAC0);
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);

    /* enable DAC0 and DMA for DAC0 */
    dac_enable(DAC0);
    dac_dma_enable(DAC0);
    //
    //===========================================================
    //
    // configure the DAC1
    dac_trigger_source_config(DAC1, DAC_TRIGGER_T5_TRGO);
    dac_trigger_enable(DAC1);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC1);
    //
    //dac_wave_bit_width_config(DAC1, DAC_WAVE_BIT_WIDTH_12);
    //
    // enable DAC1 and DMA for DAC1
    dac_enable(DAC1);
    dac_dma_enable(DAC1);
    //
}

void dac_dma_config(void)
{
    rcu_periph_clock_enable(RCU_DMA1);
    //
    dma_parameter_struct dma_struct;
    /* clear all the interrupt flags */
    dma_flag_clear(DMA1, DMA_CH2, DMA_INTF_GIF);
    dma_flag_clear(DMA1, DMA_CH2, DMA_INTF_FTFIF);
    dma_flag_clear(DMA1, DMA_CH2, DMA_INTF_HTFIF);
    dma_flag_clear(DMA1, DMA_CH2, DMA_INTF_ERRIF);

    /* configure the DMA1 channel 2 */
    dma_struct.periph_addr  = DAC0_R8DH_ADDRESS;
    dma_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_struct.memory_addr  = (uint32_t)lin_arr;
    dma_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_struct.number       = LIN_LEN;
    dma_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(DMA1, DMA_CH2, &dma_struct);

    dma_circulation_enable(DMA1, DMA_CH2);
    //====================================================
    //
    // clear all the interrupt flags
    dma_flag_clear(DMA1, DMA_CH3, DMA_INTF_GIF);
    dma_flag_clear(DMA1, DMA_CH3, DMA_INTF_FTFIF);
    dma_flag_clear(DMA1, DMA_CH3, DMA_INTF_HTFIF);
    dma_flag_clear(DMA1, DMA_CH3, DMA_INTF_ERRIF);

    // configure the DMA1 channel 2
    dma_struct.periph_addr  = DAC1_DATA;
    dma_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_struct.memory_addr  = (uint32_t)lin_arr;
    dma_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_struct.number       = LIN_LEN;
    dma_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(DMA1, DMA_CH3, &dma_struct);

    dma_circulation_enable(DMA1, DMA_CH3);
    //====================================================
    nvic_irq_enable(DMA1_Channel2_IRQn, 10, 0);
    dma_interrupt_enable(DMA1, DMA_CH2, DMA_INT_FTF | DMA_INT_HTF | DMA_INT_ERR);
    //
    dma_channel_enable(DMA1, DMA_CH2);
    dma_channel_enable(DMA1, DMA_CH3);
}

void timer5_config(void)
{
    rcu_periph_clock_enable(RCU_TIMER5);
    /* configure the TIMER5 */
    timer_prescaler_config(TIMER5, 0x01, TIMER_PSC_RELOAD_UPDATE);
    timer_autoreload_value_config(TIMER5, 70);
    timer_master_output_trigger_source_select(TIMER5, TIMER_TRI_OUT_SRC_UPDATE);
    //
    timer_enable(TIMER5);
}

void timer6_config(void)
{
    rcu_periph_clock_enable(RCU_TIMER6);
    // configure the TIMER6
    timer_prescaler_config(TIMER6, 0x01, TIMER_PSC_RELOAD_UPDATE);
    timer_autoreload_value_config(TIMER6, 0x70);
    timer_master_output_trigger_source_select(TIMER6, TIMER_TRI_OUT_SRC_UPDATE);
    //
    timer_enable(TIMER6);
}

// #define DMA_FLAG_ADD(flag, shift)           ((flag) << ((shift) * 4U))
// DMA_FLAG_G
// DMA_INT_FTF | DMA_INT_HTF | DMA_INT_ERR
void DMA1_Channel2_IRQHandler(void)
{
    uint32_t flag = DMA_INTF(DMA1) >> ((DMA_CH2) * 4U); // & DMA_FLAG_ADD(flag, channelx);
    DMA_INTC(DMA1) = (DMA_INT_FTF | DMA_INT_HTF | DMA_INT_ERR) << ((DMA_CH2) * 4U);
    if(flag & DMA_INT_FTF) {
        TEST7 = !TEST7;
        }
    if(flag & DMA_INT_HTF) {
        TEST6 = !TEST6;
        }
    if(flag & DMA_INT_ERR) {
        TEST5 = !TEST5;
        }
    //

}
