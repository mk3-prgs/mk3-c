#ifndef __BKP_DATA_H
#define __BKP_DATA_H

#include <gd32f10x.h>
#include <stdint.h>



#define e_init   BKP_DATA_41

#define ENABLE 1
#define DISABLE 0

uint16_t bkp_read_word(uint16_t adr);
uint32_t bkp_read_dword(uint16_t adr);
void bkp_write_word(uint16_t adr, uint16_t d);
void bkp_write_dword(uint16_t adr, uint32_t d);
//void bkp_init(void);

#endif // __BKP_DATA_H
