#include <FreeRTOS.h>
#include "task.h"

#include <gd32f10x.h>
#include <stdint.h>
#include <string.h>

#include "mt12864b.h"

#include "font.h"
#include "ak.h"
//#include "2.h"
//#include "3.h"
//#include "4.h"
//#include "5.h"

#include "spi_df.h"

static font_t* pf;
static int sP_x;
static int sP_y;

static int P_x;             //текущая X-координата
static int P_y;             //текущая Y-координата
static uint8_t dbf[MAX_X * MAX_Y+128];
//static uint8_t* dbf=NULL;
static uint8_t LCD_inited=0;
//
int abs(int x)
{
    if(x<0) x = -x;
    return(x);
}

//-------------------------- Инициализация LCD: ------------------------------
/*
void _delay_us(uint32_t us)
{
volatile uint32_t d = 4*us;
    for(; d>0; d--) {
       __asm__("NOP");
       __asm__("NOP");
       __asm__("NOP");
       __asm__("NOP");
       __asm__("NOP");
       __asm__("NOP");
       }
}
*/
void LCD_Init_Hardware()
{
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, P_LCD_CS);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, P_LCD_A0);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, P_LCD_RST);
    LCD_CS=1;
    LCD_A0=1;
    LCD_RST=0;
    //
    ///spi_df_Init();
    //
    //if(dbf == NULL) dbf = (uint8_t*)pvPortMalloc(MAX_X * MAX_Y+128);
}

void LCD_free(void)
{
    //if(dbf != NULL) vPortFree(dbf);
}

void LCD_Send(uint8_t data, uint8_t address0)
{

    if(!address0) {
        LCD_A0=0;
        }
    else {
        LCD_A0=1;
        }
    spi0_QU(1, &data, 1);
}

void LCD_Send_Data(uint8_t data)
{
    LCD_Send(data, 1);
}

void LCD_Send_Command(uint8_t cmd)
{
    LCD_Send(cmd, 0);
}

void LCD_Init(void)
{
//size_t count;
    LCD_Init_Hardware();
    //
    LCD_RST=0;
    vTaskDelay(100);
    LCD_RST=1;
    //
    LCD_Send_Command(0xE2);
    vTaskDelay(1);
    LCD_Send_Command(0x2F); // Select the power circuit operation mode

    LCD_Send_Command(0x27); // V0 Voltage Regulator Internal Resistor ratio Set

    LCD_Send_Command(0x89); // Select the step-up of the internal voltage converter
    LCD_Send_Command(0x00); // DC[1:0]

    LCD_Send_Command(0x81); // Vo
    LCD_Send_Command(0x2b); // Electronic Control Value

//    LCD_Send_Command(0xd3);
//    LCD_Send_Command(8);

    LCD_Send_Command(0x40); // Display Start Address COM0 0x40 ... 0x7f
    LCD_Send_Command(0xA6); // Normal/Reverse Display (0xA6/0xA7)
    LCD_Send_Command(0xC8); // Common Output Mode Select (0xC0... 0xCF)
    LCD_Send_Command(0xA0); // Set the display data RAM address SEG output correspondence (0xA0/0xA1)
    LCD_Send_Command(0xAF); // Включение дисплея (0xAE - off)
    //
    LCD_inited=1;
}

//------------------------- Обновление RAM LCD: ------------------------------
static int min_x;
static int min_y;
static int max_x;
static int max_y;

void LCD_Upd_rect(int x0, int y0, int x1, int y1)
{
    min_x = 0; //x0;
    max_x = 127; //x1;
    min_y = 0; //y0/8;
    max_y = 7; //y1/8;
}

void LCD_Update(void)
{
int k;
uint8_t h,l;
uint8_t *pd=NULL;
uint8_t s_bf[128];
    //
    h = 0; //0x0f & (min_x>>4);
    l = 0; //0x0f & (min_x);
    for(k=0; k<8; k++) {
        LCD_Send_Command(0xB0 + (7-k));
        LCD_Send_Command(0x10 + h);
        LCD_Send_Command(0x00 + l);
        //
        LCD_A0=1;
        //_delay_us(10);
        //SPI_Send(dbf + (k*128), 128);
        pd = dbf + (k*128);
        memcpy(s_bf, pd, 128);
        spi0_QU(1, s_bf, 128);
        //_delay_us(100);
        }
}

void LCD_mem_clear(void)
{
    memset(dbf, 0x00, MAX_X * MAX_Y);
}

void LCD_Clear(void)
{
    LCD_mem_clear();
    LCD_Update();
}

void LCD_scroll(int n)
{
int x, y;
uint16_t a,b;
    for(y = (MAX_Y-1); y > 0; y--) {
        for(x = 0; x < MAX_X; x++) {

            a = dbf[128*(y-1)+x];
            b = dbf[128*(y)  +x];

            a = (b | (a<<8))>>n;
            dbf[128*(y)  +x] = a;
            }
        }
    y = 0;
    for(x = 0; x < MAX_X; x++) {
        dbf[128*y+x] =dbf[128*y+x]>>n;
        }
    LCD_Update();
}

//------------------ Загрузка экранной памяти из массива: --------------------

/// x = 0 ... 127
/// y = 0 ... 63

void image(int k)
{
int x, y;
char *pt=(char*)ak;
/*
    if(k==1) pt=(char*)header_data_1;
    else if(k==2) pt=(char*)header_data_2;
    else if(k==3) pt=(char*)header_data_3;
    else if(k==4) pt=(char*)header_data_4;
    else if(k==5) pt=(char*)header_data_5;
*/
    for(y = RES_Y-1; y >= 0; y--) {
        for(x = 0; x < RES_X; x++) {
            if(*pt) set_color(0);
            else set_color(1);
            pix(x, y);
            pt++;
            }
        }
    set_color(1);
  //
  //LCD_Upd_rect(0, 0, RES_X, RES_Y);
  //LCD_Update();
}

//------------------------- Установка координат: -----------------------------

void LCD_SetXY(int x, int y)
{
    P_x = x;
    P_y = y;
}

//-------------------------- Чтение координаты x: ----------------------------

void LCD_GetXY(int* x, int* y)
{
    *x = P_x;
    *y = P_y;
}

//-------------------------- Чтение координаты y: ----------------------------

char LCD_GetY(void)
{
  return(P_y);
}

static uint8_t color;

void set_color(uint8_t c)
{
    color = c;
}

/// x = 0 ... 127
/// y = 0 ... 63

void pix(int x, int y)
{
uint8_t c;
uint8_t m;
int a;
    //
    if((x<0)||(y<0)||(x>=RES_X)||(y>=RES_Y)) return;
    //
    a = (y / 8) * RES_X + x;
    c = dbf[a];
    m = 0x80 >> (y%8);
    //
    if(color) c |= m;
    else c &= ~m;
    //
    dbf[a] = c;
}

void line(int x0, int y0, int x1, int y1)
{
    int  i;
    int  x, y;
    //
    int  dx = y1 - y0;
    int  dy = x1 - x0;
    int  mx   = (abs(dx) > abs(dy)) ? dx : dy;
    int  tang = (abs(dx) > abs(dy)) ? ((dy << 10) / dx) : ((dx << 10) / dy);

    (mx > 0) ? mx++ : mx--; // рисовать конец линии

    for(i = 0; i != mx; (mx > 0) ? i++ : i--) {
        x = (abs(dx) > abs(dy)) ? (x0 + ((i * tang) >> 10)) : (x0 + i);
        y = (abs(dx) > abs(dy)) ? (y0 + i)                  : (y0 + ((i * tang) >> 10));
        pix(x, y);
        }
}

void rect(int x0, int y0, int x1, int y1)
{
    line(x0, y0, x1, y0); // >
    line(x0, y1, x1, y1); // >
    line(x0, y0, x0, y1);
    line(x1, y0, x1, y1);
}

// R - радиус, X1, Y1 - координаты центра
void circle(int X1, int Y1, int R)
{
int x = 0;
int y = R;
int delta = 1 - 2 * R;
int error = 0;
    while(y >= x) {
       pix(X1 + x, Y1 + y);
       pix(X1 + x, Y1 - y);
       pix(X1 - x, Y1 + y);
       pix(X1 - x, Y1 - y);
       pix(X1 + y, Y1 + x);
       pix(X1 + y, Y1 - x);
       pix(X1 - y, Y1 + x);
       pix(X1 - y, Y1 - x);
       error = 2 * (delta + y) - 1;
       if((delta < 0) && (error <= 0)) {
           delta += (2 * (++x) + 1);
           continue;
           }
       if((delta > 0) && (error > 0)) {
           delta -= (2 * (--y) + 1);
           continue;
           }
       delta += (2 * ((++x) - (--y)));
       }
}


//////////////////////////////////////////////////////////////////////////////
///////////////////// Работа с текстом ///////////////////////////////////////

void set_font(font_t *p)
{
    pf = p;
    //
    pf->gl=pf->gl;
    pf->w=pf->w;
    pf->h=pf->h;
}

font_t* get_font(void)
{
    return pf;
}

//-------- Вывод символа по текущим координатам (в экранную память): ---------

extern int u_d0;
extern int u_d1;
char conv(char c);

void gr_txt(char *str, int x, int y, int dx)
{
char c;
int sx = P_x;
int sy = P_y;
//
    P_x = x;
    P_y = y;
    for(;;) {
        c = *str++;
        if(c=='\0') break;
        LCD_txt(c, 1);
        P_x += dx;
        }
    P_x = sx;
    P_y = sy;
}

void LCD_txt(char c_h, int em)
{
char z;
uint32_t h;
uint32_t e_m =  0x80; //(1<<pf->h) << (h-z);
uint32_t d;

    int  a = P_y >> 3;    //адрес строки
    z= (P_y%8);           //количество сдвигов
    //
    int wf=pf->w_f;
    //
    if(wf == 3) {
        h = 24 - (pf->h+1);
        }
    else if(wf == 2) {
        h = 16 - (pf->h+1);
        if(a==7) {
            h = 8 - (pf->h+1);
            wf=1;
            }
        }
    else { // wf==1
        h = 8 - (pf->h+1);
        }
    //
    uint32_t c_m = ((1<<(pf->h+1))-1) << (h-z);
    //
    for(int i = 0; i < pf->w; i++) {
        int ix = P_x+i+pf->s_x;
        d = 0;
        if(c_h) {
            if(wf==3) {
                 d |= (uint32_t)dbf[(a+2)*128+ix];
                 d |= (uint32_t)dbf[(a+1)*128+ix] << 8;
                 d |= (uint32_t)dbf[ a   *128+ix] << 16;
                 }
            else if(wf==2) {
                d |= (uint32_t)dbf[(a+1)*128+ix];
                d |= (uint32_t)dbf[ a   *128+ix] << 8;
                }
            else { //if(wf==1) {
                d |= (uint32_t)dbf[ a   *128+ix];
                }
            //
            d &= ~c_m;
            //
            if(wf != 3) {
                d |= (pf->gl[c_h * pf->w + i] << (h-z));
                }
            else {
                int m = 2*pf->w;
                d |= ((pf->gl[c_h * m + i] << 8) | pf->gl[c_h * m + i + pf->w]) << (h-z);
                }
            //
            if(wf==3) {
                dbf[(a+2)*128+ix] = d;
                dbf[(a+1)*128+ix] = d>>8;
                dbf[ a   *128+ix] = d>>16;
                }
            else if(wf==2) {
                dbf[(a+1)*128+ix] = d;
                dbf[ a   *128+ix] = d>>8;
                }
            else { //if(wf==1) {
                dbf[ a   *128+ix] = d;
                }
            }
        else { /// Работа с маркером
            d = (uint32_t)dbf[a*128+ix];

            if(em) d |=  e_m;
            else d   &= ~e_m;

            dbf[a*128+ix] = d;
            //
            //if(e_m) xprintf("P_x=%d P_y=%d d=%08x a=%d\r\n", P_x, P_y, d, a*128+ix);
            }
        //
        //ddd[i] = a*128+ix;
        }
    //
    LCD_Update();
}

//
int T_x=0;
int T_y=0;
//
void g_marker(int x, int y)
{
    int nx = pf->n_x-1;
    int ny = pf->n_y-1;

    // clear marker
    P_x = sP_x;
    P_y = sP_y;
    LCD_txt(0, 0);

    if(x>nx) x=nx;
    else if(x<0) x=0;

    if(y>ny) {
        y=ny;
        LCD_scroll(pf->h+1);
        }
    else if(y<0) y=0;

    T_x=x;
    T_y=y;
    //
    //xprintf("P_x=%d P_y=%d T_x=%d T_y=%d\r\n", P_x, P_y, T_x, T_y);

    P_x = x * (pf->w + 1);
    P_y = ((pf->n_y-1)-y) * (pf->h + 1);

    // set marker
    LCD_txt(0, 1);
    //xprintf("P_x=%d P_y=%d T_x=%d T_y=%d\r\n", P_x, P_y, T_x, T_y);

    //LCD_Update();

    sP_x = P_x;
    sP_y = P_y;
}

//------------ Вывод null-terminated string (в экранную память): -------------

void LCD_PutString(char *s)
{
  while(*s) LCD_PutChar(*s++);
}

void LCD_PutChar(char c)
{
    if(!LCD_inited) return;
    if(c==0) return;
    //
    //
    if((u_d0)||(u_d1)) {
	    c = conv(c);
	    u_d0=0; u_d1=0;
        }
    //
    if(c == 0xd0)      {u_d0=1; u_d1=0; return;}
    else if(c == 0xd1) {u_d0=0; u_d1=1; return;}
	//
    if(c=='\r') {
        //LCD_Clear();
        T_x=0;
        }
    else if(c=='\n') {
        T_y++;
        T_x=0;
        }
    else if(c=='\t') {
        LCD_Clear();
        T_x=T_y=0;
        }
    else {
        LCD_txt(c, 0);
        T_x++;
        }
    //
    g_marker(T_x, T_y);
    //xprintf("x=%d y=%d c=%02x\r\n", T_x, T_y, c);
}

void lcd_set_sat(uint8_t n, uint8_t v)
{
    n &= 0x3f;
    v &= 0x07;

    LCD_Send_Command(0x20+v);

    LCD_Send_Command(0x81);
    LCD_Send_Command(n);
    //
    //xprintf("n=0x%02x v=0x%02x\r\n", n, 0x20+v);
}
