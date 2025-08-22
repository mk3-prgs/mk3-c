#ifndef _BIT_FIELD_H
#define _BIT_FIELD_H

#include "gd32f10x.h"

#define GET_BIT_BB(port,bit) (*(uint32_t *)(PERIPH_BB_BASE + ((port - PERIPH_BASE) * 32) + (bit * 4)))

///#define BIT_BAND_PER(REG,BIT_MASK) (*(volatile uint32_t*)(PERIPH_BB_BASE + 32*((uint32_t)((REG) )- PERIPH_BASE) + 4*((uint32_t)(MASK_TO_BIT(BIT_MASK)))))
///#define BIT_BAND_SRAM(RAM,BIT) (*(volatile uint32_t*)(SRAM_BB_BASE+32*((uint32_t)((void*)(RAM))-SRAM_BASE)+4*((uint32_t)(BIT))))

#endif
