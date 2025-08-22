#ifndef _MTC16201_DEFINED
#define _MTC16201_DEFINED
//
#include <stdint.h>
#include "lib.h"

void lcd_init(void);

void mtc16201_Init(void);
void marker(char pos);

int putc_lcd(char c);

void lcd_print_s(char *s);

void Tim3_Init(void);

void set_sat(uint16_t d);
void set_led(uint16_t d);

#endif

