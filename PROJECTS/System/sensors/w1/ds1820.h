#ifndef __MY_DA18B20_H_
#define __MY_DA18B20_H_
#include "bit_field.h"

//void     wlan_init(uint16_t n);
//uint16_t rp_pp(uint16_t n);
//void     wbit(uint8_t b, uint16_t n);
//uint8_t  rbit(uint16_t n);
//uint8_t  rd(uint16_t n);
//void     wr(uint8_t c, uint16_t n);

void ow_init(void);
uint8_t ow_rst(void);
void ow_wr(uint8_t d);
uint8_t ow_rd(void);
void ow_wbit(uint8_t b);
uint8_t ow_rbit(void);

void CalcCRC(unsigned char *CRCVal,unsigned char value);
unsigned char crc(unsigned char len,unsigned char *d);
int rTemp(void);
void sTemp(void);
unsigned char rRom(unsigned char *bf);

void ow_temp(char *s);

#endif



