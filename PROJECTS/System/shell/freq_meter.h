#ifndef _FREQ_METER_DEFINED
#define _FREQ_METER_DEFINED

#include <gd32f10x.h>
#include "bit_field.h"
//
#define FREQ GET_BIT_BB(GPIOB+8,9)
#define FREQ_P GPIOB
#define FREQ_I GPIO_PIN_9
//
#define CNTRL_I GPIO_PIN_8
#define CNTRL_P GPIOA
#define CNTRL   GET_BIT_BB(GPIOA+8,8)
//
void fm_init(void);
uint32_t r_Nob(void);
//
#endif
