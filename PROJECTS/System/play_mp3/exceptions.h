/*************************************************************************************************
 * @file		exceptions.h
 *
 * @brief		Заголовок модуля обработки исключений
 *
 * @version		v1.0
 * @date		03.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include "gd32f10x.h"

// макрос отладочной остановки ядра
#define DBG_HALT(numb) \
	do { \
		if (CoreDebug->DHCSR & 1) \
		{ \
			__asm( "BKPT %0\n"::"M"(numb) ); \
		} \
	} while(0)

// инициализация модуля
void hard_fault_ini(void);


#endif /* EXCEPTIONS_H_ */
