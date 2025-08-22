#ifndef MB2_SLAVE_H
#define MB2_SLAVE_H

#include "file_io.h"

void mb2_init(uint8_t slave, int speed);
int  mb2_read_holding_regs (uint8_t slave, uint16_t addr, uint16_t* buff, size_t size);
int  mb2_write_holding_regs(uint8_t slave, uint16_t addr, uint16_t* buff, size_t size);

#endif // MB_SLAVE_H
