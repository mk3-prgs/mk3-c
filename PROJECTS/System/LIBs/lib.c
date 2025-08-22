#include "xprintf.h"
#include "lib.h"

char bcdd(char d)
{
char c;
	c=d/10;
	c*=16;
	c+=d%10;
	return(c);
}

char ddcb(char d)
{
char c;
	c=d/16;
	c*=10;
	c+=d%16;
	return(c);
}


int ogran(int d,int min,int max)
{
	if(d<min) d=min;
	else if(d>max) d=max;
	return(d);
}

int c_ogran(int d,int min,int max)
{
	if(d<min) d=max;
	else if(d>max) d=min;
	return(d);
}


void usleep(uint32_t us)
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

extern volatile uint32_t t_ms;

void msleep(int ms)
{
    t_ms = ms;
    while(t_ms) {}
}

void print_fl(float f)
{
union {
    uint32_t d;
    float f;
} u;

char c='\0';
    //
    u.f = f;
    //
    int s = 0x01 & (u.d >> 31);
    //
    if(s) c='-';
    //
    u.d &= ~0x80000000;
    //
    int a = (int)(u.f*1000.0);
    int b = a%1000;
    a /= 1000;
    if(c) xputc(c);
    xprintf("%d.%03d", a, b);
    //
}
