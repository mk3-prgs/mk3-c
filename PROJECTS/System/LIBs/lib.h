#ifndef _LIB_DEFINED
#define _LIB_DEFINED

#include <stdint.h>

char bcdd(char d);

char ddcb(char d);

int ogran(int d,int min,int max);
int c_ogran(int d,int min,int max);

void usleep(uint32_t us);

void msleep(int ms);

/*
unsigned char ogranc(unsigned char d,unsigned char min,unsigned char max)
{
	if(d<=min) d=min;
	else if(d>=max) d=max;
	return(d);
}
*/
#endif

























