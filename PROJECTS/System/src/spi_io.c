#include "spi_io.h"
#include "xprintf.h"

extern volatile uint32_t tm_spi_io;

#define PC6  GET_BIT_BB(GPIOC+12,6)
#define PC7  GET_BIT_BB(GPIOC+12,7)

uint8_t spi_io_RW(uint8_t data)
{
uint16_t d=0;
    //
    // Чистим буфер
    spi_i2s_data_receive(SPI1);
    //
    if(spi_i2s_flag_get(SPI1, SPI_FLAG_TBE) == SET) {
        d = data;
        spi_i2s_data_transmit(SPI1, d);

        tm_spi_io = 5;
        while(spi_i2s_flag_get(SPI1, SPI_I2S_INT_FLAG_RBNE) == RESET) {
            if(tm_spi_io == 0) { d=0; break; }
            }
        d = spi_i2s_data_receive(SPI1);
        }
    //
    return(d);
}
/*
uint8_t swp_nibl(uint8_t data)
{
    uint8_t l=0x0f & (data>>4);
    uint8_t h=0xf0 & (data<<4);
    return(h+l);
}

volatile uint8_t spi_txbf[8];
volatile uint32_t spi_txpt=0;
volatile int32_t spi_txlen=0;

volatile uint8_t spi_rxbf[8];
volatile uint32_t spi_rxpt=0;
volatile int32_t spi_rxlen=0;

uint8_t spi_dio(uint8_t* bf, int len)
{
int i;
    LC_IO=0;
    // Чистим буфер
    spi_i2s_data_receive(SPI1);
    //
    if(len>8) len=8;
    if(len == 0) { LC_IO=1; return(0);}
    spi_txlen=len;
    spi_rxlen=len;
    for(i=0; i<len; i++) { spi_txbf[i] = bf[i]; spi_rxbf[i] = 0; }
    //
    LC_IO=1;
    //
    spi_txpt=0;
    spi_rxpt=0;
    CS_IO=0;
    //
    spi_i2s_interrupt_enable(SPI1, SPI_I2S_INT_RBNE);
    spi_i2s_data_transmit(SPI1, spi_txbf[spi_txpt++]);
    spi_txlen--;
    //
    tm_spi0 = 20;
    while((spi_rxlen > 0)) {
        if(tm_spi0 == 0) {
            spi_i2s_interrupt_disable(SPI1, SPI_I2S_INT_TBE);
            spi_i2s_interrupt_disable(SPI1, SPI_I2S_INT_RBNE);
            xprintf("ERROR SPI1! tx=%d rx=%d\n", spi_txlen, spi_rxlen);
            break;
            }
        }
    CS_IO=1;
    //xprintf("txpt=%d rxpt=%d\n", spi_txpt, spi_rxpt);
    //
    for(i=0; i<len; i++) {
        bf[i] = swp_nibl(spi_rxbf[i]);
        }
    return(0);
}

void SPI1_IRQHandler(void)
{
    if(spi_i2s_interrupt_flag_get(SPI1, SPI_I2S_INT_FLAG_RBNE) == SET) {
        spi_rxbf[spi_rxpt++] = spi_i2s_data_receive(SPI1);
        spi_rxlen--;
        //
        if(spi_rxlen <= 0) {
            spi_i2s_interrupt_disable(SPI1, SPI_I2S_INT_RBNE);
            CS_IO=1;
            return;
            }
        //
        spi_i2s_data_transmit(SPI1, spi_txbf[spi_txpt++]);
        spi_txlen--;
        }
}
*/
static spi_parameter_struct spi;

#ifdef OLD_BOARD
uint8_t spi_io_Init(void)
{
    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_LC_IO);
    LC_IO=0;
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_CS_IO);
    CS_IO=1;
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SCK_IO | MOSI_IO);
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, MISO_IO);

    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, FREQ_I);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, CNTRL_I);

    rcu_periph_clock_enable(RCU_SPI1);

    spi_parameter_struct spi;
    spi_i2s_deinit(SPI1);

    spi_struct_para_init(&spi);
    /* configure SPI0 parameter */
    spi.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi.device_mode          = SPI_MASTER;
    spi.frame_size           = SPI_FRAMESIZE_8BIT;
    spi.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
    spi.nss                  = SPI_NSS_SOFT;
    spi.prescale             = SPI_PSC_2;
    spi.endian               = SPI_ENDIAN_MSB;
    //
    spi_init(SPI1, &spi);
    //
    /* Enable SPI0 CRC calculation */
    //SPI_CalculateCRC(SPI0, ENABLE);
    //
    // SS = 1
    spi_nss_internal_high(SPI1);
    //----------------------------------------------------------------
    uint16_t dummy = spi_i2s_data_receive(SPI1); dummy=dummy;
    //
    //spi_i2s_interrupt_enable(SPI0, SPI_I2S_INT_RBNE);
    //
    ///NVIC_EnableIRQ(SPI0_IRQn);
    //nvic_irq_enable(SPI0_IRQn, 1, 0);
    //
    // включим  SPI0
    spi_enable(SPI1);
    //
    return(0);
}
#else
uint8_t spi_io_Init(void)
{
    //
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_LC_IO);
    LC_IO=0;
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_CS_IO);
    CS_IO=1;
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SCK_IO | MOSI_IO);
    gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, MISO_IO);
    //
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, FREQ_I);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, CNTRL_I);
///
    ///RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI0, ENABLE);
    rcu_periph_clock_enable(RCU_SPI1);


    ///SPI_I2S_DeInit(SPI0);
    spi_i2s_deinit(SPI1);

    ///SPI_StructInit(&spi);
    spi_struct_para_init(&spi);

    /* configure SPI0 parameter */
    spi.device_mode          = SPI_MASTER;
    spi.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi.frame_size           = SPI_FRAMESIZE_8BIT;
    spi.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi.nss                  = SPI_NSS_SOFT;
    spi.prescale             = SPI_PSC_256;
    spi.endian               = SPI_ENDIAN_MSB;
    //
    spi_init(SPI1, &spi);
    //
    /* Enable SPI0 CRC calculation */
    //SPI_CalculateCRC(SPI0, ENABLE);
    //
    // SS = 1
    spi_nss_internal_high(SPI1);
    //----------------------------------------------------------------
    uint16_t dummy = spi_i2s_data_receive(SPI1); dummy=dummy;
    //
    //nvic_irq_enable(SPI0_IRQn, 1, 0);
    ///NVIC_SetPriority(SPI1_IRQn, 9);
    ///NVIC_EnableIRQ(SPI1_IRQn);
    ///spi_i2s_interrupt_enable(SPI1, SPI_I2S_INT_RBNE);
    //
    // включим  SPI0
    spi_enable(SPI1);
    //
    return(0);
}

void spi_io_reInit(void)
{
    spi_i2s_deinit(SPI1);
    spi_init(SPI1, &spi);
    spi_nss_internal_high(SPI1);
    spi_enable(SPI1);
}
#endif
