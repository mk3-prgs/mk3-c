#include "usart.h"

void usart_init(int speed)
{
/* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    ///rcu_periph_clock_enable(RCU_GPIOA);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, speed);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
    //
    // Разрешаем прерывания USART.
    NVIC_SetPriority(USART0_IRQn, 9);
    NVIC_EnableIRQ(USART0_IRQn);
    //
    usart_interrupt_enable(USART0, USART_INT_RBNE);
}

static uint8_t rx_bf[16];
static int pt_rxbf=0;
static int st_rxbf=0;
static int pt_in=0;

void USART0_IRQHandler(void)
{
    if(usart_flag_get(USART0, USART_FLAG_RBNE)) {
        usart_flag_clear(USART0, USART_FLAG_RBNE);
        //
        rx_bf[pt_rxbf++] = usart_data_receive(USART0);
        pt_rxbf &= 0x0f; // 16 - bytes
        st_rxbf++;
        }
}

uint8_t usart_rx(void)
{
uint8_t c=0;
/*
    while(RESET == usart_flag_get(USART0, USART_FLAG_RBNE)) {}
    //
    ///if(usart_flag_get(USART0, USART_FLAG_RBNE) != RESET) {
        c = usart_data_receive(USART0);
    ///   }
    //
    return(c);
*/
    if(st_rxbf == 0) c=0;
    else {
        c = rx_bf[pt_in++];
        pt_in &= 0x0f;
        st_rxbf--;
        }
    return(c);
}

void usart_tx(char c)
{
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    usart_data_transmit(USART0, c);
    //
}

void usart_print(char *s)
{
    while(*s) usart_tx(*s++);
}

///void LCD_PutChar(char ch);

void x_putc(uint8_t c)
{
    usart_tx(c);
}

uint8_t x_getc(void)
{
uint8_t c;
    c = usart_rx();
    return(c);
}

