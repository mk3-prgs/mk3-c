#ifndef _TIC55_DEFINED
#define _TIC55_DEFINED

#include <gd32f10x.h>
#include <stdint.h>
//
void usleep(uint32_t us);
#define _delay_us(x) usleep(1*x)

char toHex(char c);
void tic55_init(void);
void tic55_outbyte(unsigned int c);
void print_d(char n,unsigned int d);
void print_tv(int t, int v);
void print_f(int t);
#endif
