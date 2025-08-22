//#include "light.h"
#include "mtc16201/pwm_out.h"
#include "mtc16201/mtc16201.h"
#include "bit_field.h"
//
///////////////////////////////////////////////////////////////
const uint8_t mtc_d0[];
const uint8_t mtc_d1[];

static int u_d0=0;
static int u_d1=0;

static char conv(char c)
{
char ret=c;
	//
	if(u_d0) {
        if((c >= 0x90)&&(c <= 0xbf)) {
            ret = mtc_d0[1 + (2 * (c-0x90+1))]; // А ... п
            }
        else if(c==0x81) ret = mtc_d0[1]; // Ё
        }
    else if(u_d1) {
        if((c >= 0x80)&&(c <= 0x8f)) {
            ret = mtc_d1[1+ (2 * (c-0x80+1))]; // А ... п
            }
        else if(c==0x91) ret = mtc_d1[1]; // Ё
        }
	else ret = c;
	return(ret);
}

#ifdef OLD_BOARD

#define RS  GET_BIT_BB(GPIOA + 12, 13)
#define RW  GET_BIT_BB(GPIOA + 12, 14)
#define EN  GET_BIT_BB(GPIOA + 12, 15)
//
#else
///////////////////////////////////////////////////////////////////
#define RS  GET_BIT_BB(GPIOA + 12, 13)
#define RW  GET_BIT_BB(GPIOA + 12, 14)
#define EN  GET_BIT_BB(GPIOC + 12, 13)
//
#endif // OLD_BOARD

void Set_DB_Inp(void)
{
#ifdef OLD_BOARD
    gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ,  GPIO_PIN_6 |
                                                        GPIO_PIN_7 |
                                                        GPIO_PIN_8 |
                                                        GPIO_PIN_9 |
                                                        GPIO_PIN_10 |
                                                        GPIO_PIN_11 |
                                                        GPIO_PIN_12 |
                                                        GPIO_PIN_13);
#else
    gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ,  GPIO_PIN_0 |
                                                        GPIO_PIN_1 |
                                                        GPIO_PIN_2 |
                                                        GPIO_PIN_3 |
                                                        GPIO_PIN_4 |
                                                        GPIO_PIN_5 |
                                                        GPIO_PIN_6 |
                                                        GPIO_PIN_7);
#endif // OLD_BOARD
}
//
void Set_DB_Out(void)
{
#ifdef OLD_BOARD
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,   GPIO_PIN_6 |
                                                            GPIO_PIN_7 |
                                                            GPIO_PIN_8 |
                                                            GPIO_PIN_9 |
                                                            GPIO_PIN_10 |
                                                            GPIO_PIN_11 |
                                                            GPIO_PIN_12 |
                                                            GPIO_PIN_13);
#else
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,   GPIO_PIN_0 |
                                                            GPIO_PIN_1 |
                                                            GPIO_PIN_2 |
                                                            GPIO_PIN_3 |
                                                            GPIO_PIN_4 |
                                                            GPIO_PIN_5 |
                                                            GPIO_PIN_6 |
                                                            GPIO_PIN_7);
#endif // OLD_BOARD
}
//
void mtc16201_Init(void)
{
#ifdef OLD_BOARD
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_13 |
                                                           GPIO_PIN_14 |
                                                           GPIO_PIN_15);
#else
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_13 |
                                                           GPIO_PIN_14);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_13);
#endif // OLD_BOARD

    Set_DB_Out();
    //-----------------------------------------------------
    //
    EN=0;
    RS=0;
    RW=0;
}

//
void set_DB(uint8_t d)
{
    uint32_t data = d;
    uint32_t tmp = gpio_output_port_get(GPIOC);
#ifdef OLD_BOARD
    tmp &= ~(0x00003fc0);
    tmp |= (data << 6);
#else
    tmp &= ~(0xff);
    tmp |= (data);
#endif // OLD_BOARD
    gpio_port_write(GPIOC, tmp);
}
//
void pcwr(unsigned char r, unsigned char c)
{
	if(r) RS=1;
	else RS=0;
	//
	RW=0;
	//
	set_DB(c);
	usleep(5);
	EN=1;
	usleep(5);
	EN=0;
	//
	//usleep(5);
}

unsigned char pcrd(void)
{
unsigned char c = 0;
	Set_DB_Inp();
    usleep(2);
	EN=1;
	usleep(5);
#ifdef OLD_BOARD
    if(GPIO_ISTAT(GPIOC) & GPIO_PIN_13) c = 1;
    else c = 0;
#else
    if(GPIO_ISTAT(GPIOC) & GPIO_PIN_7) c = 1;
    else c = 0;
#endif // OLD_BOARD
	EN=0;
	usleep(2);
	//
    Set_DB_Out();
	return(c);
}

int isr(void)
{
uint32_t i;
    //
    i = 0;
    //
    RS=0;
    Set_DB_Inp();
    RW=1;
    //
    while(pcrd()) {
        //SET_ALARM_LED;
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
}

int putc_lcd(int c)
{
    //return 0;
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

int lcd_putchar(int c)
{
    //return 0;
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

static uint8_t hex_tab[]=
{
'0','1','2','3','4','5','6','7',
'8','9','a','b','c','d','e','f'
};

void lcd_hex(uint8_t d)
{
    lcd_putchar(hex_tab[0x0f & (d>>4)]);
    lcd_putchar(hex_tab[0x0f & d]);
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
        lcd_putchar(c);
        //putchar(toHex(c/16));
        //putchar(toHex(c%16));
        }
}
//
extern uint16_t led;
extern uint16_t sat;

void lcd_init(void)
{
    //return;
    //
    set_led(5000);
    set_sat(9400); //5000 //100
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

/*
volatile uint8_t t_tim;

void TIM3_IRQHandler(void)
{
    // Если счётчик переполнился, можно смело закидывать
    //   в регистр сравнения новое значение.
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        //
        TIM_SetCompare3(TIM3, led);
        TIM_SetCompare4(TIM3, sat);
        }

}
*/

const uint8_t mtc_d0[] = {
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

const uint8_t mtc_d1[] = {
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


