#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "xprintf.h"

#include "gd32f10x.h"
#include "usart/usart_bus.h"

#include "ff.h"
#include "rtc_32f1.h"

//! Шина USART.
static usart_bus_t*  p_ubus;

static bool usart2_de_set_callback(void)
{
    ///SET_DE_CB;
#ifdef OLD_BOARD
    gpio_bit_set(GPIOA, BIT(1));
#else
    ///gpio_bit_set(GPIOB, BIT(6));
#endif
    return(1);
}

static bool usart2_de_clr_callback(void)
{
    ///CLR_DE_CB;
#ifdef OLD_BOARD
    gpio_bit_reset(GPIOA, BIT(1));
#else
    ///gpio_bit_reset(GPIOB, BIT(6));
#endif
    return(0);
}

/**
 * Обработчик прерывания USART.
 */
void USART2_IRQHandler(void)
{
    usart_bus_irq_handler(p_ubus);
}

/**
 * Обработчик прерывания канала DMA передачи данных USART.
 */
void DMA0_Channel1_IRQHandler(void)
{
    usart_bus_dma_tx_channel_irq_handler(p_ubus);
}

/**
 * Обработчик прерывания канала DMA приёма данных USART.
 */
void DMA0_Channel2_IRQHandler(void)
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
bool usart_rx_byte_callback(uint8_t byte)
{
    return true;
}

/**
 * Каллбэк получения сообщения.
 * @return Флаг обработки.
 */
static int rx_done=0;
bool usart_rx_callback(void)
{
    rx_done=1;
    return true;
}

/**
 * Каллбэк отправки сообщения.
 * @return Флаг обработки.
*/
bool usart_tx_callback(void)
{
   return true;
}

static bool usart_tc_callback(void)
{
    usart2_de_clr_callback();
    usart_bus_receiver_enable(p_ubus);
    return true;
}

/**
 * Инициализирует USART1.
 */
void init_usart2(usart_bus_t* p_ub, int speed)
{
    p_ubus = p_ub;
    //
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_USART2);
    rcu_periph_clock_enable(RCU_DMA0);
    //
    // Ножки GPIO.
#ifdef OLD_BOARD
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
#else
    ///gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
#endif // OLD_BOARD
    usart2_de_clr_callback();
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_11);
    //
    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, speed);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);
    //usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
    //usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
    //
    // Шина USART.
    usart_bus_init_t usartb_is = {
        .usart_device = (USART_TypeDef*)USART2, // Периферия.
        .dma_tx_channel = p_DMA1_Channel1,      // (dma.h) Канал DMA на передачу.
        .dma_rx_channel = p_DMA1_Channel2       // (dma.h) Канал DMA на приём.
        };
    //
    usart_bus_init(p_ubus, &usartb_is);

    // Установка каллбэков.
    usart_bus_set_rx_callback(p_ubus,      usart_rx_callback);
    usart_bus_set_tx_callback(p_ubus,      usart_tx_callback);
    usart_bus_set_rx_byte_callback(p_ubus, usart_rx_byte_callback);
    usart_bus_set_tc_callback(p_ubus,      usart_tc_callback);
    usart_bus_set_deset_callback(p_ubus,   usart2_de_set_callback);
    usart_bus_set_declr_callback(p_ubus,   usart2_de_clr_callback);

    // При обнаружении свободной линии - прекратить принимать данные.
    usart_bus_set_idle_mode(p_ubus, USART_IDLE_MODE_END_RX);
//
    // Разрешаем прерывания USART.
    NVIC_SetPriority(USART2_IRQn, 8);
    NVIC_EnableIRQ(USART2_IRQn);

    // Разрешаем прерывания DMA USART2.
    NVIC_SetPriority(DMA0_Channel1_IRQn, 8);
    NVIC_EnableIRQ(DMA0_Channel1_IRQn);

    NVIC_SetPriority(DMA0_Channel2_IRQn, 8);
    NVIC_EnableIRQ(DMA0_Channel2_IRQn);
}

uint8_t mh_crc(uint8_t *packet)
{
int i;
uint8_t checksum=0;
    //
    for(i=1;i<8;i++) {
        checksum += packet[i];
        }
    checksum = 0xff - checksum;
    checksum += 1;
    return(checksum);
}

usart_bus_t ubus;
uint8_t mh_rx[16];
uint8_t mh_tx[16];

int CO2=0;

void mh_z14a(void* p)
{
uint16_t conz=0;
int init = 3;
int range=5000;
//
FIL d_file;
FIL* fp=&d_file;
UINT cnt=0;
RTCTIME rtc;
    //
    //
    init_usart2(&ubus, 9600);
    //
    while(1 ) {
        //
        if(init>0) {
            init--;
            mh_tx[0] = 0xff;
            mh_tx[1] = 0x01;
            mh_tx[2] = 0x99;
            mh_tx[3] = 0x00;
            //
            mh_tx[4] = range>>24;
            mh_tx[5] = range>>16;
            mh_tx[6] = range>>8;
            mh_tx[7] = range;
            //
            mh_tx[8] = mh_crc(mh_tx);
            }
        else {
            mh_tx[0] = 0xff;
            mh_tx[1] = 0x01;
            mh_tx[2] = 0x86;
            mh_tx[8] = mh_crc(mh_tx);
            }
        rx_done=0;
        usart_bus_send(&ubus, mh_tx, 9);
        //vTaskDelay(1000);
        usart_bus_recv(&ubus, mh_rx, 9);
        while(rx_done==0) { vTaskDelay(1); }
        /*
        xprintf("<");
        for(int i=0; i<9; i++) xprintf("%02x ", mh_rx[i]);
        xprintf(">\n");
        */
        conz = mh_rx[2];
        conz <<= 8;
        conz |= mh_rx[3];
        CO2 = conz;
        //
        //xprintf("CO2=%d\n", conz);
        //
        //
        int res = f_open(fp, "co2.txt", FA_OPEN_APPEND | FA_WRITE);
        if(res) {
            xprintf("CO2 dst file: ");
            return;
            }
        rtc_gettime(&rtc);
        char str[32];
        memset(str, 0, 32);
        xsprintf(str, "%02d:%02d:%02d %5d\n", rtc.hour, rtc.min, rtc.sec, CO2);
        int len = f_size(fp);
        f_lseek (fp, len);
        f_write(fp, str, strlen(str),  &cnt);
        f_close(fp);
        //
        vTaskDelay(5000);
        }
}
