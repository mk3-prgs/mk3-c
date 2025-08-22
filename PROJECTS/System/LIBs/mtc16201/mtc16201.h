#ifndef _MTC16201_DEFINED
#define _MTC16201_DEFINED
//
#include "lib.h"

void lcd_init(void);

void set_DB(uint8_t d);
void mtc16201_Init(void);
void marker(char pos);
int lcd_putchar(int c);
void lcd_print_s(char *s);
void lcd_hex(uint8_t d);
//
int putc_lcd(int c);
//
int d_sat(int d);
int d_led(int d);
void set_sat(uint16_t d);
void set_led(uint16_t d);

#endif

