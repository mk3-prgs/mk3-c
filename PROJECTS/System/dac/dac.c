#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdint.h>
#include <string.h>
#include "xprintf.h"
#include "ff.h"

#include "mtc16201/pwm3_out.h"
/*
#include "font.h"
extern font_t font_L;
extern font_t font_M;
extern font_t font_H;
#include "mt12864b.h"
*/
#include "bit_field.h"
#define TEST0     GET_BIT_BB(GPIOA+12,0)
#define TEST1     GET_BIT_BB(GPIOA+12,1)

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
        if((i%16)==0) xprintf("\n%08x: ", addr); addr+=16;
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

void main(int argv, char* argc[])
{
TickType_t l_t;
const TickType_t freq = 250;
int i;
uint32_t val = 70;
char cmd[64];
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

//const uint8_t convertarr[CONVERT_NUM] = {0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF, 0xCC, 0x99, 0x66, 0x33};
const uint8_t sin_arr[CONVERT_NUM] = {
127, 133, 139, 145, 151, 157, 163, 169,
175, 181, 186, 192, 197, 202, 207, 212,
216, 221, 225, 229, 232, 235, 239, 241,
244, 246, 248, 250, 251, 252, 253, 253,
254, 253, 253, 252, 251, 250, 248, 246,
244, 241, 239, 235, 232, 229, 225, 221,
216, 212, 207, 202, 197, 192, 186, 181,
175, 169, 163, 157, 151, 145, 139, 133,
127, 120, 114, 108, 102,  96,  90,  84,
 78,  72,  67,  61,  56,  51,  46,  41,
 37,  32,  28,  24,  21,  18,  14,  12,
  9,   7,   5,   3,   2,   1,   0,   0,
  0,   0,   0,   1,   2,   3,   5,   7,
  9,  12,  14,  18,  21,  24,  28,  32,
 37,  41,  46,  51,  56,  61,  67,  72,
 78,  84,  90,  96, 102, 108, 114, 120
};

const uint8_t cos_arr[CONVERT_NUM] = {
254, 253, 253, 252, 251, 250, 248, 246,
244, 241, 239, 235, 232, 229, 225, 221,
216, 212, 207, 202, 197, 192, 186, 181,
175, 169, 163, 157, 151, 145, 139, 133,
126, 120, 114, 108, 102,  96,  90,  84,
 78,  72,  67,  61,  56,  51,  46,  41,
 37,  32,  28,  24,  21,  18,  14,  12,
  9,   7,   5,   3,   2,   1,   0,   0,
  0,   0,   0,   1,   2,   3,   5,   7,
  9,  12,  14,  18,  21,  24,  28,  32,
 37,  41,  46,  51,  56,  61,  67,  72,
 78,  84,  90,  96, 102, 108, 114, 120,
126, 133, 139, 145, 151, 157, 163, 169,
175, 181, 186, 192, 197, 202, 207, 212,
216, 221, 225, 229, 232, 235, 239, 241,
244, 246, 248, 250, 251, 252, 253, 253
};

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
    dma_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_struct.memory_addr  = (uint32_t)sin_arr;
    dma_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_struct.number       = CONVERT_NUM;
    dma_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(DMA1, DMA_CH2, &dma_struct);

    dma_circulation_enable(DMA1, DMA_CH2);
    dma_channel_enable(DMA1, DMA_CH2);
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
    dma_channel_enable(DMA1, DMA_CH3);
    //
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

