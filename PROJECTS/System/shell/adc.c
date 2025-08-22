#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdint.h>
#include <string.h>
#include "xprintf.h"
#include "ff.h"

#include <gd32f10x.h>
//#include "mt12864b.h"

#ifdef OLD_BOARD
#define NUMB_KAN 2
#else
#define NUMB_KAN 4
#endif

#include "bit_field.h"
//#define TEST0  GET_BIT_BB(GPIOB+12,10)
//#define TEST1  GET_BIT_BB(GPIOB+12,11)
//#include "ssd1306.h"


void adc_dma_config(void);
void ADC_init(void);

void adc_config(void);

uint16_t adc_value[512];

void adc_config(void)
{
    /* enable ADC0 clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* enable DMA0 clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);

    /* reset ADC */
    adc_deinit(ADC0);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC scan function enable */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* ADC temperature and Vrefint enable */
    adc_tempsensor_vrefint_enable();

    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 4);

    adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_0, ADC_SAMPLETIME_28POINT5);
    adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_1, ADC_SAMPLETIME_28POINT5);
    /* ADC temperature sensor channel config */
    adc_inserted_channel_config(ADC0, 2, ADC_CHANNEL_16, ADC_SAMPLETIME_28POINT5);
    /* ADC internal reference voltage channel config */
    adc_inserted_channel_config(ADC0, 3, ADC_CHANNEL_17, ADC_SAMPLETIME_28POINT5);

    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_2_EXTTRIG_INSERTED_NONE);

    adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);

    /* enable ADC interface */
    adc_enable(ADC0);
    vTaskDelay(2);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
    //
    adc_tempsensor_vrefint_enable();
}

void adc_pwr(char* s)
{
//char *p = &s[8];
    //if(*p == '0') POWER=0;
    //else POWER=1;
}

void adc_task(void* dummy);

void adc_init(char *s)
{
    //
    //gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); // ALARM< Power
    //POWER=0;
    //
    //adc_config();
    adc_dma_config();
    ADC_init();
    /*
    uint32_t reg = GPIO_CTL0(GPIOA);
    reg &= ~0xff; // pin 0, 1
    GPIO_CTL0(GPIOA) = reg;
    */
    xprintf("ADC0 inited!\n");
    //
    ///xTaskCreate( adc_task, "adc",  configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}

int Is=0;
extern int CO2;

void adc_task(void* dummy)
{
TickType_t l_time;
TickType_t freq=250;
int i;
int d;
int s_I=0;
int n;
char str[32];
float x;
//
    l_time = xTaskGetTickCount();
    for(;;) {
        vTaskDelayUntil(&l_time, freq);
        ///LCD_mem_clear();
        /*
        g_marker(0,2);
        LCD_PutChar('U');
        g_marker(0,6);
        LCD_PutChar('I');
        //
        line(8, 0, 8, 30);
        line(8, 32, 8, 62);
        line(0, 0, 127, 0);
        line(0, 32, 127, 32);
        */
        //image(0);

        s_I=0;
        n=0;
        for(i=0; i<323; i++) {
            if((i%2) == 0) { // 3337 - 592 = 2745 <=> 12.0V
                d = adc_value[i]-592;
                if(d<0) d=0;
                x = d;
                x *= (22.0/2745);
                d = x;
                ///pix(i/4+12 , d+32); // 30
                }
            if((i%2) != 0) { // Ток
                d = (adc_value[i]-3218);
                if(d<0) d = 0;

                if(i<300) {
                    s_I += d;
                    n++;
                    }
                x = d;
                x *= (20.0/272.0);
                d = x;
                ///pix(i/4+12, d);
                }
            Is = s_I/n;
            }
        ///g_marker(16,6);
        x = Is;
        x *= (100.0/272.0); //3490 - 3218 = 270
        Is=x;
        xprintf(str, "%d.%02dA\n", Is/100, Is%100);
        ///LCD_PutString(str);
        //
        ///g_marker(16,1);
        ///xsprintf(str, "%d", CO2);
        ///LCD_PutString(str);
        //
        ///LCD_txt(0, 0);
        ///LCD_Update();

        }
}

void adc(char *s)
{
//int i;
//int k;
//long t;
//int n=1;
//char* pt=&s[3];
//int kl=0;
//int d=0;
    //
    //if(xatoi(&pt, &t)) {
    //        n = t;
    //        }
    ///for(k=0; k<n; k++) {
        /*
        d=0; kl=0;
        for(i=0; i<512; i++) {
            if((i%10)==0) xprintf("\n%03d: ", i);
            if((i%2)==0) {
                d += adc_value[i];
                kl++;
                xprintf("%4d ", adc_value[i]);
                }
            }
        xprintf("\n");
        xprintf("Ns=%d\n", d/kl);
        */
        /*
        kl = !kl;
        for(i=0; i<50; i++) {
            if(kl) d+=100;
            else   d-=100;
            if(d>5000) d=5000;
            else if(d<0) d=0;
            ///set_sol(d);
            vTaskDelay(100);
            }
        */
        ///}
    xprintf("%04x %04x %04x %04x\n",
            adc_value[0],
            adc_value[1],
            adc_value[2],
            adc_value[3]
            );
    int a,b;
    a=adc_value[2];
    b=adc_value[3];
    xprintf("t=%4d v=%4d\n", a, b);
    float temp = (1.42 - a*3.3/4096) * 1000 / 4.3 + 25;
    float vref = (b * 3.3 / 4096);
    int t=10*temp;
    int v=100*vref;
    xprintf("%d.%dC %d.%02dV\n", t/10, t%10, v/100, v%100);

    adc_value[0]=0;
    adc_value[1]=0;
    adc_value[2]=0;
    adc_value[3]=0;
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

void adc_w(char *s)
{
int i;
long t;
int n=1;
char* pt=&s[4];
    //
    if(xatoi(&pt, &t)) {
            n = t;
            }
    //
    xprintf("ADC0:\n");
    for(i=0; i<n; i++) {
        /*
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
        vTaskDelay(1000);
        //
        int a,b;
        a=ADC_IDATA0(ADC0);
        b=ADC_IDATA1(ADC0);
        xprintf("t=%4d v=%4d\n", a, b);
        float temp = (1.42 - a*3.3/4096) * 1000 / 4.3 + 25;
        float vref = (b * 3.3 / 4096);
        int t=10*temp;
        int v=100*vref;
        xprintf("%d.%dC %d.%02dV\n", t/10, t%10, v/100, v%100);
        */
        //xprintf("%4d %4d %4d %4d\n", adc_value[0], adc_value[1], adc_value[2], adc_value[3]);
        //
        int a;
        int b;
        a=adc_value[0];
        b=adc_value[1];
        //b=adc_value[3];
        float v0 = (a-500) * 0.00332623;
        float v1 = (b-73)  * 0.000802825;
        //float vref = (b * 3.3 / 4096);
        int U0 = 100.0 * v0;
        int U1 = 100.0 * v1;
        xprintf("U0=%d.%02dV U1=%d.%02dV\n", U0/100, U0%100, U1/100, U1%100);

        ///char str[64];
        ///xsprintf(str, "\nU0=%d.%02dV U1=%d.%02dV\r", U0/100, U0%100, U1/100, U1%100);
        ///LCD_PutString(str);

        adc_value[0]=0;
        adc_value[1]=0;
        adc_value[2]=0;
        adc_value[3]=0;
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
        //TEST1 = 1;
        //
        vTaskDelay(500);
        }
}
void adc_cal(char *s)
{
    int32_t a[4];
    /* reset the selected ADC1 calibration registers */
    ADC_CTL1(ADC0) |= (uint32_t) ADC_CTL1_RSTCLB;
    /* check the RSTCLB bit state */
    while(RESET != (ADC_CTL1(ADC0) & ADC_CTL1_RSTCLB)) {}
    vTaskDelay(1);
    //
    adc_value[0]=0;
    adc_value[1]=0;
    adc_value[2]=0;
    adc_value[3]=0;
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    vTaskDelay(100);
    xprintf("%4d %4d %4d %4d\n", adc_value[0], adc_value[1], adc_value[2], adc_value[3]);
    a[0]=adc_value[0];
    a[1]=adc_value[1];
    a[2]=adc_value[2];
    a[3]=adc_value[3];
    /* enable ADC calibration process */
    ADC_CTL1(ADC0) |= ADC_CTL1_CLB;
    /* check the CLB bit state */
    while(RESET != (ADC_CTL1(ADC0) & ADC_CTL1_CLB)) {}
    vTaskDelay(1);
    //
    adc_value[0]=0;
    adc_value[1]=0;
    adc_value[2]=0;
    adc_value[3]=0;
    //adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    vTaskDelay(100);
    xprintf("%4d %4d %4d %4d\n", adc_value[0], adc_value[1], adc_value[2], adc_value[3]);
    //
    a[0] -= adc_value[0];
    a[1] -= adc_value[1];
    a[2] -= adc_value[2];
    a[3] -= adc_value[3];
    xprintf("%4d %4d %4d %4d\n", a[0], a[1], a[2], a[3]);
}

void ADC_init(void)
{
#ifdef OLD_BOARD
    //gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); // ALARM< Power
    //POWER=0;
#endif // OLD_BOARD
    //
    /* enable ADC0 clock */
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_ADC1);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV16);
    //
#ifdef OLD_BOARD
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
#else
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
#endif
    //
    /* reset ADC */
    adc_deinit(ADC0);
    adc_deinit(ADC1);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);

    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, NUMB_KAN);

    adc_tempsensor_vrefint_enable();

    /* ADC regular channel config */
#ifdef OLD_BOARD
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_12, ADC_SAMPLETIME_13POINT5 );
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_13, ADC_SAMPLETIME_13POINT5 );
#else
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0,  ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_1,  ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_16, ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_17, ADC_SAMPLETIME_239POINT5);
#endif

    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

    /* ADC discontinuous mode */
    adc_discontinuous_mode_config(ADC0, ADC_REGULAR_CHANNEL, NUMB_KAN);

    /* clear the ADC flag */
    adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);
    adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOIC);
    /* enable ADC interrupt */
    adc_interrupt_enable(ADC0, ADC_INT_EOC);
    NVIC_EnableIRQ(ADC0_1_IRQn);
    //nvic_irq_enable(ADC0_1_IRQn, 1, 1);

    /* enable ADC interface */
    adc_enable(ADC0);
    vTaskDelay(1);

    adc_tempsensor_vrefint_enable();

    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);

    /* ADC DMA function enable */
    adc_dma_mode_enable(ADC0);
    //
    //adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

/* ADC_DMA_channel configuration */
dma_parameter_struct dma_data_parameter;

void adc_dma_config(void)
{
    /* enable DMA0 clock */
    rcu_periph_clock_enable(RCU_DMA0);

    /* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter;
    dma_struct_para_init(&dma_data_parameter);
    /* ADC DMA_channel configuration */
    dma_deinit(DMA0, DMA_CH0);

    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)(adc_value);
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = NUMB_KAN; //256*NUMB_KAN;
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_data_parameter);

    dma_circulation_enable(DMA0, DMA_CH0);
    //
    NVIC_EnableIRQ(DMA0_Channel0_IRQn);
    //dma_interrupt_enable(DMA0, DMA0_Channel0_IRQn, DMA_INT_FTF | DMA_INT_HTF);
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH0);
    //
}

volatile int a_en=0;

void DMA0_Channel0_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FLAG_FTF)){
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_G);
        a_en=0;
        }
     //TEST0 = 0;
}

void ADC0_1_IRQHandler(void)
{
    /* clear the ADC flag */
    adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);
    adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOIC);
    //
    if(a_en) adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

void f_adc(char *s)
{
uint32_t sampl = ADC_SAMPLETIME_13POINT5;
long t;
int n=3;
char* pt=&s[5];
    //
    if(xatoi(&pt, &t)) {
            n = t;
            }
    //
    if(n==1) sampl = ADC_SAMPLETIME_1POINT5;
    else if(n==2) sampl = ADC_SAMPLETIME_7POINT5;
    else if(n==3) sampl = ADC_SAMPLETIME_13POINT5;
    else if(n==4) sampl = ADC_SAMPLETIME_28POINT5;
    else if(n==5) sampl = ADC_SAMPLETIME_41POINT5;
    else if(n==6) sampl = ADC_SAMPLETIME_55POINT5;
    else if(n==7) sampl = ADC_SAMPLETIME_71POINT5;
    else if(n==8) sampl = ADC_SAMPLETIME_239POINT5;

    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0,  sampl);
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_1,  sampl);
}
