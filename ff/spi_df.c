#include "spi_df.h"

uint8_t df_DetectParams(void);

extern volatile uint32_t tm_spi0;

uint8_t spi_df_RW(uint8_t data)
{
uint16_t d=0;
    //
    // Чистим буфер
    spi_i2s_data_receive(SPI0);
    //
    if(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == SET) {
        d = data;
        spi_i2s_data_transmit(SPI0, d);
        tm_spi0 = 5;
        while(spi_i2s_flag_get(SPI0, SPI_I2S_INT_FLAG_RBNE) == RESET) {
            if(tm_spi0 == 0) { d=0; break; }
            }
        d = spi_i2s_data_receive(SPI0);
        }
    //
    return(d);
}

/*
void SPI0_IRQHandler(void)
{
    if(spi_i2s_interrupt_flag_get(SPI0, SPI_I2S_INT_FLAG_RBNE) == SET) {
        SPI0_Rx_Data = spi_i2s_data_receive(SPI0);
        spi_i2s_interrupt_disable(SPI0, SPI_I2S_INT_RBNE);
        //CS_DF = 1;
        }
    //
    if(spi_i2s_interrupt_flag_get(SPI0, SPI_I2S_INT_FLAG_TBE) == SET) {
        spi_i2s_interrupt_disable(SPI0, SPI_I2S_INT_TBE);
        spi_i2s_data_transmit(SPI0, SPI0_Tx_Data);
        }
}
*/
static uint8_t spi_df_inited=0;

#ifdef OLD_BOARD

uint8_t spi_df_Init(void)
{
    if(spi_df_inited) return(0);
    //
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_CS_DF);
    CS_DF=1;
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SCK_DF | MOSI_DF);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, MISO_DF);

    rcu_periph_clock_enable(RCU_SPI0);

    spi_parameter_struct spi;
    spi_i2s_deinit(SPI0);

    spi_struct_para_init(&spi);
    /* configure SPI0 parameter */
    spi.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi.device_mode          = SPI_MASTER;
    spi.frame_size           = SPI_FRAMESIZE_8BIT;
    spi.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi.nss                  = SPI_NSS_SOFT;
    spi.prescale             = SPI_PSC_8;
    spi.endian               = SPI_ENDIAN_MSB;
    //
    spi_init(SPI0, &spi);
    //
    /* Enable SPI0 CRC calculation */
    //SPI_CalculateCRC(SPI0, ENABLE);
    //
    // SS = 1
    spi_nss_internal_high(SPI0);
    //----------------------------------------------------------------
    uint16_t dummy = spi_i2s_data_receive(SPI0); dummy=dummy;
    //
    //spi_i2s_interrupt_enable(SPI0, SPI_I2S_INT_RBNE);
    //
    ///NVIC_EnableIRQ(SPI0_IRQn);
    //nvic_irq_enable(SPI0_IRQn, 1, 0);
    //
    // включим  SPI0
    spi_enable(SPI0);
    spi_df_inited=1;
    //
    //return(df_DetectParams());
    return(0);
}
#else
uint8_t spi_df_Init(void)
{
    if(spi_df_inited) return(0);
    //
    gpio_pin_remap_config(GPIO_SPI0_REMAP, ENABLE);
    //
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_CS_DF);
    CS_DF=1;
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_CS_ENC);
    CS_ENC=1;
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, N_SPI_RST);
    SPI_RST=1;
    //
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SCK_DF | MOSI_DF);
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, MISO_DF);
    //
    rcu_periph_clock_enable(RCU_SPI0);

    spi_parameter_struct spi;
    spi_i2s_deinit(SPI0);

    spi_struct_para_init(&spi);
    /* configure SPI0 parameter */
    spi.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi.device_mode          = SPI_MASTER;
    spi.frame_size           = SPI_FRAMESIZE_8BIT;
    spi.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi.nss                  = SPI_NSS_SOFT;
    spi.prescale             = SPI_PSC_8;
    spi.endian               = SPI_ENDIAN_MSB;
    //
    spi_init(SPI0, &spi);
    //
    /* Enable SPI0 CRC calculation */
    //SPI_CalculateCRC(SPI0, ENABLE);
    //
    // SS = 1
    spi_nss_internal_high(SPI0);
    //----------------------------------------------------------------
    uint16_t dummy = spi_i2s_data_receive(SPI0); dummy=dummy;
    //
    //spi_i2s_interrupt_enable(SPI0, SPI_I2S_INT_RBNE);
    //
    ///NVIC_EnableIRQ(SPI0_IRQn);
    //nvic_irq_enable(SPI0_IRQn, 1, 0);
    //
    // включим  SPI0
    spi_enable(SPI0);
    spi_df_inited=1;
    //
    //return(df_DetectParams());
    return(0);
}
#endif // OLD_BOARD
