#ifndef Lcd133x64H
#define Lcd133x64H

#include <stdint.h>
#include "font.h"

#define bool uint8_t

//----------------------------- Константы: -----------------------------------

#define NOM_CONTR 120 //номинальный контраст LCD
#define BOLD 0x10    //смещение на жирные цифры
#define NORM 0x30    //смещение на нормальные цифры

#define OFFSET_X 0 //сдвиг по X, всего 21 позиция, x < 128 (ном. 3)
#define OFFSET_Y 0 //сдвиг по Y, всего 6 строк (ном. 3)

#define MFONT_X 5   //ширина стандартного шрифта
#define MFONT_Y 7   //высота стандартного шрифта

#define LFONT_X 4   //ширина мелкого шрифта
#define LFONT_Y 6   //высота мелкого шрифта

#define HFONT_X 9  //ширина крупного шрифта
#define HFONT_Y 13 //высота крупного шрифта

//Разрешение дисплея:

#define RES_X 128
#define RES_Y 64

//Количество строк и знакомест:

#define MAX_LIN 8
#define MAX_POS 21

//Максимальные значения адресов:

#define MAX_X 128 //для совместимости с TIC151
#define MAX_Y (RES_Y / 8)

//-------------------------- Прототипы функций: ------------------------------

void LCD_Init(void);                  //инициализация LCD
void LCD_free(void);
void LCD_Send(uint8_t data, uint8_t address0);
void LCD_Send_Data(uint8_t data);
void LCD_Send_Command(uint8_t cmd);

void LCD_Upd_rect(int x0, int y0, int x1, int y1);
void LCD_Update(void);                //обновление RAM LCD

void LCD_SetXY(int x, int y);       //установка координат
void LCD_GetXY(int* x, int* y);

void LCD_mem_clear(void);
void LCD_Clear(void);                 //очистка дисплея
void LCD_scroll(int n);

void LCD_PutChar(char ch);            //вывод символа
void LCD_PutString(char *s);  //вывод строки

void LCD_txt(char c_h, int em);

void set_font(font_t *f);
font_t* get_font(void);

void set_color(uint8_t c);
void pix(int x, int y);
void line(int x0, int y0, int x1, int y1);
void rect(int x0, int y0, int x1, int y1);
void circle(int X1, int Y1, int R);
void image(int k);
//
void LCD_marker(int en);
void test_xy(void);
void g_marker(int x, int y);
//----------------------------------------------------------------------------
void lcd_set_sat(uint8_t n, uint8_t v);
void gr_txt(char *str, int x, int y, int dx);

#endif
