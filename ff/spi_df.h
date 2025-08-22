#ifndef _SPI_DF_DEFINED
#define _SPI_DF_DEFINED

#include "gd32f10x.h"
#include "bit_field.h"

#ifdef OLD_BOARD
#define SCK_DF    GPIO_PIN_5
#define MOSI_DF   GPIO_PIN_7
#define MISO_DF   GPIO_PIN_6
#define N_CS_DF   GPIO_PIN_4
#define CS_DF     GET_BIT_BB(GPIOA+12,4)
#else
#define SCK_DF    GPIO_PIN_3
#define MOSI_DF   GPIO_PIN_5
#define MISO_DF   GPIO_PIN_4
#define N_CS_DF   GPIO_PIN_15
#define CS_DF     GET_BIT_BB(GPIOA+12,15)

#define N_CS_ENC  GPIO_PIN_0
#define N_SPI_RST GPIO_PIN_3
#define CS_ENC    GET_BIT_BB(GPIOC+12,0)
#define SPI_RST   GET_BIT_BB(GPIOC+12,3)

// PC0 PC1 PC3
#define LCD_CS    GET_BIT_BB(GPIOC+12,1)
#define LCD_A0    GET_BIT_BB(GPIOC+12,0)

#define P_LCD_CS  GPIO_PIN_1
#define P_LCD_A0  GPIO_PIN_0
#endif // OLD_BOARD

uint8_t spi_df_Init(void);
uint8_t spi_df_RW(uint8_t data);

#endif
