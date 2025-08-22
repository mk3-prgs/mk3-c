#include <gd32f10x.h>
//
#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdint.h>
#include "dac.h"
#include "xprintf.h"

void usleep(uint32_t us);

static int is_inited=0;
//
void dac_init(void)
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
    dac_trigger_source_config(DAC0, DAC_TRIGGER_SOFTWARE);
    dac_trigger_disable(DAC0);
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);

    /* enable DAC0 and DMA for DAC0 */
    dac_enable(DAC0);
    //dac_dma_enable(DAC0);
    //
    //===========================================================
    //
    // configure the DAC1
    dac_trigger_source_config(DAC1, DAC_TRIGGER_SOFTWARE);
    dac_trigger_disable(DAC1);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC1);

    // enable DAC1 and DMA for DAC1
    //dac_enable(DAC1);
    //dac_dma_enable(DAC1);
    //
    dac_concurrent_enable();
    //dac_concurrent_data_set(DAC_ALIGN_12B_L, 0, 1024);
    dac_concurrent_data_set(DAC_ALIGN_12B_R, 4095, 4095);
}

void dac(char *s)
{
char *pt=&s[3];
long t;
uint16_t d=0;
//
    if(is_inited==0) dac_init();
//
    if(xatoi(&pt, &t)) {
        d=t;
        dac_concurrent_data_set(DAC_ALIGN_12B_R, 4095-d, 4095-d);
        }
    //
    xprintf("dac=%d\n", d);
}

const uint8_t sin_arr[] = {
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

void dsin(char *s)
{
char *pt=&s[4];
long t;
uint16_t d=0;
int i;
int n;
int k=1000;
float f;
float min=2000.0;
float max=2000.0;
    //
    if(is_inited==0) dac_init();
    //
    if(xatoi(&pt, &t)) {
        k=t;
        }
    //
    for(n=0; n<k; n++) {
        for(i=0;i<128;i++) {
            f=((float)sin_arr[i]) / 255.0; // 0...255
            f *= max;
            f += min;
            d = (uint16_t)f;
            dac_concurrent_data_set(DAC_ALIGN_12B_R, d, d);
            //vTaskDelay(1);
            usleep(5);
            }
        }
    dac_concurrent_data_set(DAC_ALIGN_12B_R, 4095, 4095);
}
