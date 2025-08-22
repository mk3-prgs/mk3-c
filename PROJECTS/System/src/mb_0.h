#ifndef MB0_SLAVE_H
#define MB0_SLAVE_H

#include "file_io.h"

void mb0_init(uint8_t saddr, int speed);
int  mb0_read_holding_regs (uint8_t slave, uint16_t addr, uint16_t* buff, size_t size);
int  mb0_write_holding_regs(uint8_t slave, uint16_t addr, uint16_t* buff, size_t size);

#endif // MB_SLAVE_H
