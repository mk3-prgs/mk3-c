#ifndef _SPI_IO_DEFINED
#define _SPI_IO_DEFINED

#include <gd32f10x.h>
#include "bit_field.h"

#ifdef OLD_BOARD

#define POWER_IO  GPIO_PIN_5
#define PWR_IO    GET_BIT_BB(GPIOB+12,5)

#define SCK_IO    GPIO_PIN_13
#define MOSI_IO   GPIO_PIN_15
#define MISO_IO   GPIO_PIN_14
#define N_CS_IO   GPIO_PIN_12
#define CS_IO     GET_BIT_BB(GPIOB+12,12)

#define N_LC_IO   GPIO_PIN_2
#define LC_IO     GET_BIT_BB(GPIOA+12,2)

#define FREQ_I    GPIO_PIN_9
#define CNTRL_I   GPIO_PIN_8

#define FREQ    GET_BIT_BB(GPIOB+8,9)
#define CNTRL   GET_BIT_BB(GPIOA+8,8)

#else

#define POWER_IO  GPIO_PIN_8
#define PWR_IO    GET_BIT_BB(GPIOB+12,8)

#define SCK_IO    GPIO_PIN_13
#define MOSI_IO   GPIO_PIN_15
#define MISO_IO   GPIO_PIN_14
#define N_CS_IO   GPIO_PIN_12

#define N_LC_IO   GPIO_PIN_7

#define CS_IO     GET_BIT_BB(GPIOB+12,12)
#define LC_IO     GET_BIT_BB(GPIOA+12,7)
#define PWR_IO    GET_BIT_BB(GPIOB+12,8)

#define FREQ_I    GPIO_PIN_9
#define CNTRL_I   GPIO_PIN_8

#define FREQ    GET_BIT_BB(GPIOB+8,9)
#define CNTRL   GET_BIT_BB(GPIOA+8,8)

#endif // OLD_BOARD

uint8_t spi_io_Init(void);
void spi_io_reInit(void);
uint8_t spi_io_RW(uint8_t data);
///uint8_t spi_dio(uint8_t* bf, int len);
#endif
