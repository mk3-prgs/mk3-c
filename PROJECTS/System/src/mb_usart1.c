#include "gd32f10x.h"
#include "usart/usart_bus.h"
#include "modbus/modbus_rtu.h"

static bool usart_de_set_callback(void)
{
    ///SET_DE_CB;
#ifdef OLD_BOARD
    gpio_bit_set(GPIOA, BIT(1));
#else
    gpio_bit_set(GPIOA, BIT(6));
#endif
    return(1);
}

static bool usart_de_clr_callback(void)
{
    ///CLR_DE_CB;
#ifdef OLD_BOARD
    gpio_bit_reset(GPIOA, BIT(1));
#else
    gpio_bit_reset(GPIOA, BIT(6));
#endif
    return(0);
}

//! Шина USART.
static usart_bus_t*  p_ubus;
//! Modbus.
static modbus_rtu_t* p_mbus;

/**
 * Обработчик прерывания USART.
 */
void USART1_IRQHandler(void)
{
    usart_bus_irq_handler(p_ubus);
}

/**
 * Обработчик прерывания канала DMA передачи данных USART.
 */
void DMA0_Channel6_IRQHandler(void)
{
    usart_bus_dma_tx_channel_irq_handler(p_ubus);
}

/**
 * Обработчик прерывания канала DMA приёма данных USART.
 */
void DMA0_Channel5_IRQHandler(void)
{
    usart_bus_dma_rx_channel_irq_handler(p_ubus);
}

/*
 * Функции для работы с USART.
 */

/**
 * Каллбэк получения байта.
 * @param byte Байт.
 * @return Флаг обработки.
 */
static bool usart_rx_byte_callback(uint8_t byte)
{
    return modbus_rtu_usart_rx_byte_callback(p_mbus, byte);
}

/**
 * Каллбэк получения сообщения.
 * @return Флаг обработки.
 */
static bool usart_rx_callback(void)
{
    return modbus_rtu_usart_rx_callback(p_mbus);
}

/**
 * Каллбэк отправки сообщения.
 * @return Флаг обработки.
 */
static bool usart_tx_callback(void)
{

    if(p_mbus->mode == MODBUS_RTU_MODE_SLAVE) return modbus_rtu_usart_tx_callback(p_mbus);
    else return true;
}

static bool usart_tc_callback(void)
{
    //xprintf("tx done\r\n");
    //CLR_DE;
    usart_de_clr_callback();
    if(p_mbus->mode == MODBUS_RTU_MODE_MASTER) {
        usart_bus_receiver_enable(p_ubus);
        }
    return true;
}

/**
 * Инициализирует USART1.
 */
void init_usart1(modbus_rtu_t* p_mb, usart_bus_t* p_ub, int speed)
{
    //
    p_mbus = p_mb;
    p_ubus = p_ub;
    //
    rcu_periph_clock_enable(RCU_GPIOA);

    ///RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    rcu_periph_clock_enable(RCU_USART1);

        // DMA.
    ///RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    rcu_periph_clock_enable(RCU_DMA0);
    //
    // Ножки GPIO.
    ///GPIO_InitTypeDef gpio_de = {
    ///    .GPIO_Pin = GPIO_Pin_1,
    ///    .GPIO_Speed = GPIO_Speed_10MHz,
    ///   .GPIO_Mode = GPIO_Mode_Out_PP
    ///    };
    ///GPIO_Init(GPIOA, &gpio_de);
#ifdef OLD_BOARD
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
#else
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
#endif // OLD_BOARD
    //CLR_DE;
    usart_de_clr_callback();

    ///GPIO_InitTypeDef gpio_tx = {
    ///    .GPIO_Pin = GPIO_Pin_2,
     ///   .GPIO_Speed = GPIO_Speed_10MHz,
     ///   .GPIO_Mode = GPIO_Mode_AF_PP
     ///   };
    ///GPIO_InitTypeDef gpio_rx = {
    ///    .GPIO_Pin = GPIO_Pin_3,
    ///    .GPIO_Speed = GPIO_Speed_10MHz,
     ///   .GPIO_Mode = GPIO_Mode_IN_FLOATING
    ///    };
    //
    ///GPIO_Init(GPIOA, &gpio_tx);
    ///GPIO_Init(GPIOA, &gpio_rx);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
    //
    // USART1: speed b/s, 8 бит данных, контроль чётности - "чётный", 1 стоп бит.
    /*
    USART_InitTypeDef usart_is = {
        .USART_BaudRate = speed,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None
        };
    //
    USART_Init(USART2, &usart_is);
    USART_Cmd(USART2, ENABLE);
    */
    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, speed);
    usart_word_length_set(USART1, USART_WL_8BIT);
    usart_stop_bit_set(USART1, USART_STB_1BIT);
    usart_parity_config(USART1, USART_PM_NONE);
    //usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
    //usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);
    //xprintf("USART1->CR1=%x\r\n", USART1->CR1);
    //xprintf("USART2->CR1=%x\r\n", USART2->CR1);
    //
    // Шина USART.
    usart_bus_init_t usartb_is = {
        .usart_device = (USART_TypeDef*)USART1, // Периферия.
        .dma_tx_channel = p_DMA1_Channel6,      // (dma.h) Канал DMA на передачу.
        .dma_rx_channel = p_DMA1_Channel5       // (dma.h) Канал DMA на приём.
        };
    //
    usart_bus_init(p_ubus, &usartb_is);

    // Установка каллбэков.
    usart_bus_set_rx_callback(p_ubus,      usart_rx_callback);
    usart_bus_set_tx_callback(p_ubus,      usart_tx_callback);
    usart_bus_set_rx_byte_callback(p_ubus, usart_rx_byte_callback);
    usart_bus_set_tc_callback(p_ubus,      usart_tc_callback);
    usart_bus_set_deset_callback(p_ubus,      usart_de_set_callback);
    usart_bus_set_declr_callback(p_ubus,      usart_de_clr_callback);


    // При обнаружении свободной линии - прекратить принимать данные.
    usart_bus_set_idle_mode(p_ubus, USART_IDLE_MODE_END_RX);
//
    // Разрешаем прерывания USART.
    NVIC_SetPriority(USART1_IRQn, 8);
    NVIC_EnableIRQ(USART1_IRQn);

    // Разрешаем прерывания DMA USART2.
    NVIC_SetPriority(DMA0_Channel5_IRQn, 8);
    NVIC_EnableIRQ(DMA0_Channel5_IRQn);

    NVIC_SetPriority(DMA0_Channel6_IRQn, 8);
    NVIC_EnableIRQ(DMA0_Channel6_IRQn);
//
}
