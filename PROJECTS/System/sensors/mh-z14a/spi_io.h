
#ifndef SPI_IO_H
#define SPI_IO_H

#include <stdint.h>

int inp_init(void);
int out_init(void);
void inp_close(void);
void out_close(void);
uint8_t Bit_Reverse(uint8_t x);
void io_flush(void);

#endif
