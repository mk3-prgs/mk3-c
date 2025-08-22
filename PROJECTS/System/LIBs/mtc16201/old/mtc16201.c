#include <stm32f10x.h>
#include <stm32f10x_conf.h>

#include "mtc16201.h"
#include "bit_field.h"
#include "lib.h"
//

///////////////////////////////////////////////////////////////
//uint8_t utf_d0[];
//uint8_t utf_d1[];

uint8_t utf_d0[] = {
0x81, 0xa2, // Ё
0x90, 0x41, // А
0x91, 0xa0, // Б
0x92, 0x42, // В
0x93, 0xa1, // Г
0x94, 0xe0, // Д
0x95, 0x45, // Е
0x96, 0xa3, // Ж
0x97, 0xa4, // З
0x98, 0xa5, // И
0x99, 0xa6, // Й
0x9a, 0x4b, // К
0x9b, 0xa7, // Л
0x9c, 0x4d, // М
0x9d, 0x48, // Н
0x9e, 0x4f, // О
0x9f, 0xa8, // П
0xa0, 0x50, // Р
0xa1, 0x43, // С
0xa2, 0x54, // Т
0xa3, 0xa9, // У
0xa4, 0xaa, // Ф
0xa5, 0x58, // Х
0xa6, 0xe1, // Ц
0xa7, 0xab, // Ч
0xa8, 0xac, // Ш
0xa9, 0xe2, // Щ
0xaa, 0xad, // Ъ
0xab, 0xae, // Ы
0xac, 0xc4, // Ь
0xad, 0xaf, // Э
0xae, 0xb0, // Ю
0xaf, 0xb1, // Я
0xb0, 0x61, // а
0xb1, 0xb2, // б
0xb2, 0xb3, // в
0xb3, 0xb4, // г
0xb4, 0xe3, // д
0xb5, 0x65, // е
0xb6, 0xb6, // ж
0xb7, 0xb7, // з
0xb8, 0xb8, // и
0xb9, 0xb9, // й
0xba, 0xba, // к
0xbb, 0xbb, // л
0xbc, 0xbc, // м
0xbd, 0xbd, // н
0xbe, 0x6f, // о
0xbf, 0xbe, // п
};

uint8_t utf_d1[] = {
0x91, 0xb5, // ё
0x80, 0x70, // р
0x81, 0x63, // с
0x82, 0xbf, // т
0x83, 0x79, // у
0x84, 0xe4, // ф
0x85, 0x78, // х
0x86, 0xe5, // ц
0x87, 0xc0, // ч
0x88, 0xc1, // ш
0x89, 0xe6, // щ
0x8a, 0xc2, // ъ
0x8b, 0xc3, // ы
0x8c, 0xc4, // ь
0x8d, 0xc5, // э
0x8e, 0xc6, // ю
0x8f, 0xc7, // я
};

static int u_d0=0;
static int u_d1=0;

static char conv(char c)
{
char ret=c;
	//
	if(u_d0) {
        if((c >= 0x90)&&(c <= 0xbf)) {
            ret = utf_d0[1 + (2 * (c-0x90+1))]; // А ... п
            }
        else if(c==0x81) ret = utf_d0[1]; // Ё
        }
    else if(u_d1) {
        if((c >= 0x80)&&(c <= 0x8f)) {
            ret = utf_d1[1+ (2 * (c-0x80+1))]; // А ... п
            }
        else if(c==0x91) ret = utf_d1[1]; // Ё
        }
	else ret = c;
	return(ret);
}

//
static uint16_t led = 9000;
static uint16_t sat = 2000;

void set_sat(uint16_t d)
{
    sat = d;
    TIM_SetCompare4(TIM3, sat);
}

void set_led(uint16_t d)
{
    led = d;
    TIM_SetCompare3(TIM3, led);
}

extern volatile uint32_t t_ms;
//
///////////////////////////////////////////////////////////////////
//#define RS  GET_BIT(GPIOA_BASE + 12).Bit13
#define RS  GET_BIT_BB(GPIOA_BASE + 12, 13)

//#define RW  GET_BIT(GPIOA_BASE + 12).Bit14
#define RW  GET_BIT_BB(GPIOA_BASE + 12, 14)

//#define EN  GET_BIT(GPIOA_BASE + 12).Bit15
#define EN  GET_BIT_BB(GPIOA_BASE + 12, 15)
//
static void Set_DB_Out(void)
{
    GPIO_InitTypeDef gpio;
    //---------------------------------------------------
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_10MHz;
    //---------------------------------------------------
    gpio.GPIO_Pin = GPIO_Pin_6 |
                    GPIO_Pin_7 |
                    GPIO_Pin_8 |
                    GPIO_Pin_9 |
                    GPIO_Pin_10 |
                    GPIO_Pin_11 |
                    GPIO_Pin_12 |
                    GPIO_Pin_13;
    GPIO_Init(GPIOC , &gpio);
    //-----------------------------------------------------
}
//
static void Set_DB_Inp(void)
{
    GPIO_InitTypeDef gpio;
    //---------------------------------------------------
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    //---------------------------------------------------
    gpio.GPIO_Pin = GPIO_Pin_6 |
                    GPIO_Pin_7 |
                    GPIO_Pin_8 |
                    GPIO_Pin_9 |
                    GPIO_Pin_10 |
                    GPIO_Pin_11 |
                    GPIO_Pin_12 |
                    GPIO_Pin_13;
    GPIO_Init(GPIOC , &gpio);
    //-----------------------------------------------------
}
//
void mtc16201_Init(void)
{
    GPIO_InitTypeDef gpio;
    //---------------------------------------------------
    //  EN, RS, RW
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_10MHz;
    gpio.GPIO_Pin = GPIO_Pin_13 |
                    GPIO_Pin_14 |
                    GPIO_Pin_15;
    GPIO_Init(GPIOA , &gpio);
    //---------------------------------------------------
    // led, sat (Tim3)
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB , &gpio);
    //----------------------------------------------------
    Set_DB_Out();
    //-----------------------------------------------------
    //
    EN=0;
    RS=0;
    RW=0;
}

//
static void set_DB(uint8_t d)
{
    uint32_t temp = GPIOC->ODR;
    temp &= ~((uint32_t)0x3fc0);
    temp |= (((uint32_t)d) << 6);
    GPIOC->ODR = temp;
}
//
static void pcwr(unsigned char r, unsigned char c)
{
	if(r) RS=1;
	else RS=0;
	//
	RW=0;
	//
	set_DB(c);
	//
	usleep(5);
	EN=1;
	usleep(5);
	EN=0;
	//
}

//
static unsigned char pcrd(void)
{
unsigned char c = 0;

	//if(r) RS=1;
	//else RS=0;
	//
	//Set_DB_Inp();
    //RW=1;
	//usleep(5);
    //
    usleep(2);
	EN=1;
	usleep(5);
	//if(GPIOC->IDR & GPIO_Pin_13) c = 1;
	//else c = 0;
	c = GPIOC->IDR >> 6;
	EN=0;
	usleep(2);
	//
    //RW=0;
    //Set_DB_Out();
    //usleep(1);
	return(c);
}
//

static int isr(void)
{
    //
uint32_t i;
    //
    i = 0;
    //
    RS=0;
    Set_DB_Inp();
    //
    RW=1;
    //
    while(0x80 & pcrd()) {
        //
        if(i>1000) {break;}
        //
        i++;
        }
    //
    RW=0;
    Set_DB_Out();
    //
    return(i);
    //
    //usleep(500);
    //return(1);
}

int putc_lcd(char c)
{
    //return 0;
    //
	isr();
    //
	if((u_d0)||(u_d1)) {
	    pcwr(1, conv(c));
	    u_d0=0;
	    u_d1=0;
        }
	else {
        if(c=='\r') {
            pcwr(0,0x80);
            //mtc_error=isr();
            }
        else if(c=='\n') {
            pcwr(0,0xc0);
            //mtc_error=isr();
            }
        else if(c=='\t') {
            pcwr(0,0x01);
            //mtc_error=isr();
            }
        else {
            if(c==0xd0)      {u_d0=1; u_d1=0;}
            else if(c==0xd1) {u_d0=0; u_d1=1;}
            else {
                pcwr(1,c);
                //mtc_error=isr();
                }
            }
        }
	return(c);
}

void marker(char pos)
{
    //return;
	if(pos>15) pos+=48;
	isr(); pcwr(0,0x80|pos);
}
//
void lcd_print_s(char *s)
{
char c;
	while((*s) != 0) {
        c = *s++;
        putc_lcd(c);
        //putchar(toHex(c/16));
        //putchar(toHex(c%16));
        }
}
//
void lcd_init(void)
{
    //return;
    //
    Tim3_Init();
    //
    set_led(9900);
    //set_sat(5000);
    set_sat(500);
    //
    mtc16201_Init();
    msleep(20);
    //
	pcwr(0,0x30);
	msleep(20);
	pcwr(0,0x30);
	msleep(20);
	pcwr(0,0x30);
	msleep(20);
//
	isr(); pcwr(0,0x38);
	isr(); pcwr(0,0x0e);
	isr(); pcwr(0,0x06);
	isr(); pcwr(0,0x03);
	isr(); pcwr(0,0x01);
//
	isr(); pcwr(0,0x40);
//0
	isr(); pcwr(1,0x08);
	isr(); pcwr(1,0x14);
	isr(); pcwr(1,0x08);
	isr(); pcwr(1,0x03);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x03);
	isr(); pcwr(1,0x00);
//1
	isr(); pcwr(1,0x08);
	isr(); pcwr(1,0x14);
	isr(); pcwr(1,0x08);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
//2 ->
	isr(); pcwr(1,0x0c);
	isr(); pcwr(1,0x0c);
	isr(); pcwr(1,0x1c);
	isr(); pcwr(1,0x1c);
	isr(); pcwr(1,0x1c);
	isr(); pcwr(1,0x0c);
	isr(); pcwr(1,0x0c);
	isr(); pcwr(1,0x00);
//3 <-
	isr(); pcwr(1,0x06);
	isr(); pcwr(1,0x06);
	isr(); pcwr(1,0x07);
	isr(); pcwr(1,0x07);
	isr(); pcwr(1,0x07);
	isr(); pcwr(1,0x06);
	isr(); pcwr(1,0x06);
	isr(); pcwr(1,0x00);
//4 ==
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x3f);
	isr(); pcwr(1,0x3f);
	isr(); pcwr(1,0x3f);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x00);
//5 к/ч
	isr(); pcwr(1,0x14);
	isr(); pcwr(1,0x18);
	isr(); pcwr(1,0x14);
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x05);
	isr(); pcwr(1,0x07);
	isr(); pcwr(1,0x01);
	isr(); pcwr(1,0x00);
//6 Га
	isr(); pcwr(1,0x00);
	isr(); pcwr(1,0x1e);
	isr(); pcwr(1,0x10);
	isr(); pcwr(1,0x12);
	isr(); pcwr(1,0x15);
	isr(); pcwr(1,0x17);
	isr(); pcwr(1,0x15);
	isr(); pcwr(1,0x00);
//7 T
	isr(); pcwr(1,0x3f);
	isr(); pcwr(1,0x3f);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x04);
	isr(); pcwr(1,0x00);

	isr(); pcwr(0,0x80);
//
}

void Tim3_Init(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  TIM_DeInit(TIM3);
  //
  TIM_TimeBaseInitTypeDef base_timer;
  TIM_TimeBaseStructInit(&base_timer);

  base_timer.TIM_Prescaler = 6; //SystemCoreClock / 100 - 1;
  base_timer.TIM_Period = 10000;
  base_timer.TIM_ClockDivision = 0;
  base_timer.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &base_timer);

  // Конфигурируем канал:
  // - начальное заполнение: 50 тиков (500 мкс)
  // - режим: edge-aligned PWM
  TIM_OCInitTypeDef timer_oc;
  TIM_OCStructInit(&timer_oc);

  timer_oc.TIM_Pulse = 1;
  timer_oc.TIM_OCMode = TIM_OCMode_PWM1;
  timer_oc.TIM_OutputState = TIM_OutputState_Enable;
  timer_oc.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC3Init(TIM3, &timer_oc);
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_OC4Init(TIM3, &timer_oc);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM3, ENABLE);
    //
    TIM_SetCompare3(TIM3, led);
    TIM_SetCompare4(TIM3, sat);
    //
    // Включаем прерывание переполнения счётчика
    //TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    // Включаем таймер
    TIM_Cmd(TIM3, ENABLE);
    // Разрешаем прерывания таймера TIM3
    //NVIC_EnableIRQ(TIM3_IRQn);
    // TIM_CtrlPWMOutputs(TIM3, ENABLE);
}

//#define tst0 GET_BIT_BB(GPIOB_BASE + 12,6)

volatile uint8_t t_tim;

void TIM3_IRQHandler(void)
{
    //tst0=1;
    // Если счётчик переполнился, можно смело закидывать
    //   в регистр сравнения новое значение.
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        //
        //TIM_SetCompare3(TIM3, led);
        //TIM_SetCompare4(TIM3, sat);
        }

    //tst0=0;
}

