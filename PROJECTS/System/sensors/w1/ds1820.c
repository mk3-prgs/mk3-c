#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"

#include <gd32f10x.h>
#include "core_cmFunc.h"
#include "xprintf.h"
#include "ds1820.h"
#include "find_ds.h"
#include "bit_field.h"

static QueueHandle_t ow_queue=0;

void CalcCRC(unsigned char *CRCVal,unsigned char value)
{
unsigned char odd,bcnt;
	for(bcnt=0;bcnt<8;bcnt++) {
		odd=(value ^ *CRCVal)&0x01;
		*CRCVal >>= 1;
		value >>= 1;
		if(odd) *CRCVal ^= 0x8C;
		}
}

unsigned char crc(unsigned char len, unsigned char *d)
{
unsigned char i;
unsigned char CRCVal;
	CRCVal=0;
	for(i=0;i<(len-1);i++ ) CalcCRC(&CRCVal,d[i] );
	if(*(d+i) != CRCVal) return(0);
	else return(1);
}

int rTemp(void)
{
long t;
int d;
uint8_t buf[9];
unsigned char i;
    //
	d=-5005;
	//
	if(ow_rst()) return(d);
	//
	ow_wr(0xcc);
	//for(i=0;i<8;i++) wr(eeprom_read_byte(&bf[i]));
	ow_wr(0xbe);
	for(i=0; i<9; i++) {
        buf[i]=ow_rd();
        }
	if(crc(9, buf)) {
		d = (((unsigned int)buf[1])<<8)+((unsigned int)buf[0]); // ...в 0.0625-х долях градуса
		t = (((long)d) * 625) + 50;
		d=t/100;
		}
	return(d);
}

void sTemp(void)
{
	if(ow_rst()) return;
	ow_wr(0xcc);
	ow_wr(0x44);
}

unsigned char rRom(unsigned char *bf)
{
unsigned char b=0;
int i;
	if(ow_rst()) return(0);
    //
	ow_wr(0x33);
	for(i=0; i<9; i++) bf[i] = ow_rd();
    //
	if(crc(9, bf)) {
		b=1;
		}
	return(b);
}

///////////////////////////////////////////////////////////////////////////////
//
//
#define WL1 GET_BIT_BB(GPIOB+12,11)
#define IWL1 GET_BIT_BB(GPIOB+8,11)

//#define TEST GET_BIT_BB(GPIOB+12,10)

void ow_init(void)
{
static uint8_t s_init=0;
    if(s_init) return;
    //
    ow_queue = xQueueCreate(1, sizeof(uint8_t));
    //gpio_pin_remap_config(GPIO_TIMER1_PARTIAL_REMAP, ENABLE);
    gpio_init(GPIOB, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_11);
    WL1 = 1;
    //gpio_init(GPIOB, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
    //TEST = 1;

    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1);
    timer_parameter_struct tpar;
    tpar.prescaler=108;
    tpar.period=1000;
    tpar.clockdivision=TIMER_CKDIV_DIV1;
    tpar.alignedmode      = TIMER_COUNTER_EDGE;
    tpar.counterdirection = TIMER_COUNTER_UP;
    tpar.repetitioncounter = 0;
    timer_init(TIMER1, &tpar);
//
    timer_auto_reload_shadow_enable(TIMER1);
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
    //
    nvic_irq_enable(TIMER1_IRQn, 9, 0);
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);
    //
    //timer_enable(TIMER1);
    //
}

static int32_t  ow_op=0;
static uint8_t  ow_bit=0;
static uint8_t  ow_rst_bit=0;

#define US1 1
static uint32_t ow_time[]=
{
    1,  // 0
    5*US1, // 1
    7*US1, // 2
    17*US1, // 3
    60*US1, // 4
    1*US1, // 5
};

static uint32_t ow_state=0;

void TIMER1_IRQHandler(void)
{
static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        //
    if(timer_interrupt_flag_get(TIMER1, TIMER_INT_UP) == SET) {
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
        //TEST=0;
        timer_autoreload_value_config(TIMER1, ow_time[ow_state+1]);
        //
        if(ow_state==0) {
            WL1=1;
            ow_state=1;
            }
        else if(ow_state==1) {
            WL1=0;
            ow_state=2;
            }
        else if(ow_state==2) {
            if(ow_op != 0) WL1=1; // Чтение бита и запись единицы
            ow_state=3;
            //TEST=0;
            }
        else if(ow_state==3) {
            ow_bit=IWL1;
            ow_rst_bit = ow_bit;
            ow_state=4;
            //TEST=1;
            }
        else if(ow_state==4) {
            WL1=1;
            ow_state=5;
            }
        else if(ow_state==5) {
            WL1=1;
            timer_disable(TIMER1);
            ow_state=0;
            if(ow_queue != 0) xQueueSendFromISR(ow_queue, &ow_bit, &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
                }
            }
        }
}

void  ow_wbit(uint8_t c)
{
uint8_t s;
    ow_op=0x01 & c;
	timer_counter_value_config(TIMER1, 0);
	timer_autoreload_value_config(TIMER1, ow_time[0]);
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
    timer_enable(TIMER1);
	if(ow_queue != 0) xQueueReceive(ow_queue, &s, 10);
}

void  ow_wr(uint8_t c)
{
int i;
uint8_t s;
	for(i=0; i<8; i++) {
		ow_op=0x01 & c;
		c >>= 1;
		timer_counter_value_config(TIMER1, 0);
        timer_autoreload_value_config(TIMER1, ow_time[0]);
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
        timer_enable(TIMER1);
		if(ow_queue != 0) xQueueReceive(ow_queue, &s, 10);
		}
}

uint8_t ow_rbit(void)
{
uint8_t b;
	ow_op=-1;
	timer_counter_value_config(TIMER1, 0);
	timer_autoreload_value_config(TIMER1, ow_time[0]);
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
    timer_enable(TIMER1);
	if(ow_queue != 0) xQueueReceive(ow_queue, &b, 10);
    //
	return(b);
}

uint8_t ow_rd(void)
{
uint8_t i;
uint8_t d;
uint8_t b;
	d=0;
	for(i=0;i<8;i++){
		d /= 2;
		ow_op=-1;
		timer_counter_value_config(TIMER1, 0);
        timer_autoreload_value_config(TIMER1, ow_time[0]);
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
        timer_enable(TIMER1);
		if(ow_queue != 0) xQueueReceive(ow_queue, &b, 10);
		if(b) d |= 0x80;
		else d &= ~0x80;
		}
	return(d);
}

uint8_t ow_rst(void)
{
uint8_t s;
    ow_time[1]=480;
    ow_time[2]=90;
    ow_time[4]=1000;
    ow_op=-1;
    timer_counter_value_config(TIMER1, 0);
    timer_autoreload_value_config(TIMER1, 1);
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
    timer_enable(TIMER1);
    if(ow_queue != 0) xQueueReceive(ow_queue, &s, 10);
    ow_time[1]=5;
    ow_time[2]=7;
    ow_time[4]=60;
    //
	return(ow_rst_bit);
}

extern unsigned char ROM_NO[8];

void wl(char *s)
{
uint8_t bf[9];
int i;
//
    ow_init();
    //
    rRom(bf);
    for(i=0; i<9; i++) xprintf("%02x ", bf[i]);
    xprintf("\n");
    //
    OWTargetSetup(0x28);
    int res = OWFirst();
    xprintf("res=%d\n", res);
    res = OWNext();
    xprintf("res=%d\n", res);
    //
    for(i=0; i<8; i++) xprintf("%02x ", ROM_NO[i]);
    xprintf("\n");
}
