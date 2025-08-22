/*************************************************************************************
*
*  Драйвер Atmel DataFlash
*
*  by:      BSVi
*  version: 1.00
*
*************************************************************************************/

#ifndef _DATA_FLASH_H
#define _DATA_FLASH_H

#include "spi_df.h"

// В этом файле даны краткие описания функций. Подробную информацию
// о принимаемых и возвращаемых параметрах конкретной функции можно в
// "шапке" этой функции в файле dataflash.cpp

// Описание возможностей подключенной микросхемы.
typedef struct _df_Info_t
{
    char*     name;        // тип микросхемы
    uint16_t  pages;       // Количество страниц
    uint16_t  page_size;   // Размер страницы в байтах
    uint8_t   page_bit;    // Длина адреса страницы
    uint8_t   chip_id;     // Код устройства
} df_Info_t;

// Возвращает S_OK если микросхема подключена. После вызова этой функции,
// парамтеры подключеной микросхемы можно узнать из структуры df_Info
uint8_t df_Init();

uint8_t df_GetStatus();
uint8_t df_DetectParams();

// Проверка выполнилась ли предыдущая операция. Сейчас драйвер сам проверяет
// выполнение операций. Нет необходимости использовать эту функцию.
uint8_t df_isReady();

// Запись и чтение в буффер. См. dataflash.cpp
void df_Write(uint8_t BufferNo, uint16_t Addr, uint16_t Count, uint8_t *BufferPtr );
void df_WriteByte(uint8_t BufferNo, uint16_t Addr, uint8_t Data );

void df_Read( uint8_t BufferNo, uint16_t Addr, uint16_t Count, uint8_t *BufferPtr );
uint8_t df_GetChar(uint8_t BufferNo, uint16_t Addr );

// Чтение непосредственно из флэш-памяти
void df_FlashRead( uint16_t PageAdr, uint16_t Addr, uint16_t Count, uint8_t *BufferPtr );

void df_PageFunc( uint8_t PageCmd, uint16_t PageAdr );
/*****************************************************************************
*  Коды комманд для использования с df_PageFunc
******************************************************************************/
#define DF_FLASH_TO_BUF1 		0x53	/* Main memory page to buffer 1 transfer  */
#define DF_FLASH_TO_BUF2 		0x55	/* Main memory page to buffer 2 transfer */
#define DF_BUF1_TO_FLASH_WITH_ERASE   	0x83	/* Buffer 1 to main memory page program with built-in erase */
#define DF_BUF2_TO_FLASH_WITH_ERASE   	0x86	/* Buffer 2 to main memory page program with built-in erase */
#define DF_BUF1_TO_FLASH_WITHOUT_ERASE  0x88	/* Buffer 1 to main memory page program without built-in erase */
#define DF_BUF2_TO_FLASH_WITHOUT_ERASE  0x89	/* Buffer 2 to main memory page program without built-in erase */

#endif /* _DATA_FLASH_H */
