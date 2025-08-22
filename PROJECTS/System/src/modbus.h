#ifndef MODBUS_H
#define	MODBUS_H

#include <stm32f10x.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t func;
    uint8_t addr;
    int len;
    uint8_t buff[256];
} q_rsp;


#endif	/* DMA_H */

