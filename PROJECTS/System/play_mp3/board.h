/*************************************************************************************************
 * @file		board.h
 *
 * @brief		Модуль описания периферии контроллера
 *
 * @version		v1.0
 * @date		03.09.2013
 * @author		Mike Smith
 *
 *************************************************************************************************/
#ifndef BOARD_H_
#define BOARD_H_

#include "bit_field.h"
/*
#define T_P1 GET_BIT_BB(GPIOC+12,0)
#define T_P2 GET_BIT_BB(GPIOC+12,1)
#define T_P3 GET_BIT_BB(GPIOC+12,2)
#define T_P4 GET_BIT_BB(GPIOC+12,3)
#define T_P5 GET_BIT_BB(GPIOC+12,4)
#define T_P6 GET_BIT_BB(GPIOC+12,5)


#define TEST_PIN1_LOW	(T_P1=0)
#define TEST_PIN1_HIGH	(T_P1=1)

#define TEST_PIN2_LOW	(T_P2=0)
#define TEST_PIN2_HIGH	(T_P2=1)

#define TEST_PIN3_LOW	(T_P3=0)
#define TEST_PIN3_HIGH	(T_P3=1)

#define TEST_PIN4_LOW	(T_P4=0)
#define TEST_PIN4_HIGH	(T_P4=1)

#define TEST_PIN5_LOW	(T_P5=0)
#define TEST_PIN5_HIGH	(T_P5=1)

#define TEST_PIN6_LOW	(T_P6=0)
#define TEST_PIN6_HIGH	(T_P6=1)


//================================================================================================

// отображение прерывания DMA DAC
#define DAC_DMA_TEST_LOW	TEST_PIN1_LOW
#define DAC_DMA_TEST_HIGH	TEST_PIN1_HIGH

// отображение декодирования порции сэмплов
#define DECODE_TEST_LOW		TEST_PIN2_LOW
#define DECODE_TEST_HIGH	TEST_PIN2_HIGH

// светодиод
#define LED_ON				TEST_PIN3_LOW
#define LED_OFF				TEST_PIN3_HIGH

#define DATA_READ_MARKER_HIGH	TEST_PIN4_HIGH
#define DATA_READ_MARKER_LOW	TEST_PIN4_LOW
*/
#define TEST7     GET_BIT_BB(GPIOC+12,7)
#define TEST6     GET_BIT_BB(GPIOC+12,6)
#define TEST5     GET_BIT_BB(GPIOC+12,5)
#define TEST4     GET_BIT_BB(GPIOC+12,4)
#define TEST3     GET_BIT_BB(GPIOC+12,3)
#define TEST2     GET_BIT_BB(GPIOC+12,2)
#define TEST1     GET_BIT_BB(GPIOC+12,1)
#define TEST0     GET_BIT_BB(GPIOC+12,0)
#endif
