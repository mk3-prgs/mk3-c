#include <FreeRTOS.h>
#include "task.h"

#include "xprintf.h"
#include "font.h"

extern font_t font_L;
extern font_t font_M;
extern font_t font_H;
//
#include "mt12864b.h"

void gsat(char *s)
{
char *pt=&s[4];
long t;

uint8_t n=0;
uint8_t v=0;
//
    if(xatoi(&pt, &t)) {
        n=t;
        if(xatoi(&pt, &t)) {
            v = t;
            }
        }
    n &= 0x3f;
    v &= 0x07;

    LCD_Send_Command(0x20+v);
    LCD_Send_Command(0x81);
    LCD_Send_Command(n);
    //
    xprintf("n=0x%02x v=0x%02x\n", n, 0x20+v);
}

extern int T_x;
extern int T_y;

void glcd_init(char *s)
{
    LCD_Init();
    LCD_Clear();
    set_color(1);
    /*
    set_font(&font_M);
    g_marker(1,1);
    LCD_PutString("Привет Мир!\n");
    */
}

void gecho(char *s)
{
char c;
    for(;;) {
        c = xfunc_in();
		if(c == 0) {
            vTaskDelay(10);
            continue;
            }
        else if(c == 0x03) break;
        LCD_PutChar(c);
        }
}

void gclr(char *s)
{
    LCD_Clear();
}

void gfont(char *s)
{
int l=1;
long d;
font_t *pf=&font_M;
char *pt=&s[5];

    if(xatoi(&pt, &d)) {
        l=d;
        }
    //
    if(l == 0) pf = &font_L;
    else if(l == 1) pf = &font_M;
    else if(l == 2) pf = &font_H;
    set_font(pf);
}

void glcd(char *s)
{
char *pt=&s[4];

uint8_t c = *pt++;
    if(c == ' ') c=*pt;
    else return;

    //rect(0,0,127,63);
/*
    circle(63, 31, 30);
    line(0, 0, 127, 63);
    line(6,1,6,4);
    line(1,6,4,6);
    rect(0,0,127,63);
    LCD_Upd_rect(0, 0, 127, 63);
    LCD_Update();
*/

    //LCD_Clear();

    //xprintf("%02x ", c);
    //if(c == '\r') LCD_PutChar('\n');
    //LCD_PutChar(c);

    if(c == 'E') {
        LCD_Update();
        }
    /*
    else if(c == '+') {
        n++;
        set_sat(n,v);
        }
    else if(c == '-') {
        n--;
        set_sat(n,v);
        }
    else if(c == '1') {
        v++;
        set_sat(n,v);
        }
    else if(c == '2') {
        v--;
        set_sat(n,v);
        }
    else if(c == 'p') {
        LCD_Send_Command(0x82);
        }
    else if(c == 'n') {
        LCD_Send_Command(0x83);
        }
    else if(c == 'N') {
        LCD_Send_Command(0xA6);
        }
    else if(c == 'R') {
        LCD_Send_Command(0xA7);
        }
    */
    else if(c == 'U') {
        LCD_Clear();

        set_color(1);
        //line(0, 0, 128, 64);
        line(1, 63, 127, 1);
        line(-64, -64, 64, 64);
        rect(5,5,120,60);

        LCD_Update();
        }
    else if(c == 'I') {
        //LCD_Init();
        //set_color(0);
        LCD_Clear();
        set_color(1);
        image(1);
        LCD_Update();
        }
    else {
        if(c == '\r') LCD_PutChar('\n');
        LCD_PutChar(c);
        }
}

void img(char *s)
{
char *pt=&s[3];
long t;
uint16_t d=0;
//
    if(xatoi(&pt, &t)) {
        d=t;
        }
    //
    LCD_Init();
    set_color(0);
    LCD_Clear();
    set_color(1);
    image(d);
    LCD_Update();
}
