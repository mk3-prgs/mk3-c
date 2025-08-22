#include <gd32f10x.h>

void usart_init(int speed);
uint8_t usart_rx(void);
void usart_tx(char c);
void usart_print(char *s);

void x_putc(uint8_t c);
uint8_t x_getc(void);

uint8_t usart_hit(void);
