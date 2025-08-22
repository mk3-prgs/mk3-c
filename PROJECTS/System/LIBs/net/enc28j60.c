#include "gd32f10x.h"
#include <stdint.h>
#include "net/enc28j60.h"
//
#include <xprintf.h>
/*
void hex_dump(uint8_t* bf, int len)
{
int i;
    if(bf != (uint8_t*)(0)) {
        xprintf("\n");
        for(i=0; i<len; i++) {
            if((i!=0) && !(i%32)) xprintf("\n");
            xprintf("%02x", bf[i]);
            }
        xprintf("\n");
        }
}
*/

volatile uint8_t  enc28j60_current_bank = 0;
volatile uint16_t enc28j60_rxrdpt = 0;

/*
void enc28j60_select(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

void enc28j60_release()
{
    //tst_bf[tst_st++] = '\n';
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
    //
    //if((tst_bf[0] == 0x19) && (tst_bf[1] == 0xff));
    //else hex_dump(tst_bf, tst_st);
}
*/
/*
uint8_t enc28j60_rxtx(uint8_t data)
{

	while(SPI_I2S_GetFlagStatus(SPI_N, SPI_I2S_FLAG_TXE)==RESET);
	SPI_I2S_SendData(SPI_N,data);

    tst_bf[tst_st++] = data;
    if(tst_st>=2048) tst_st=0;

	while(SPI_I2S_GetFlagStatus(SPI_N, SPI_I2S_FLAG_RXNE)==RESET);
	return SPI_I2S_ReceiveData(SPI_N);
}
*/

uint8_t enc28j60_rxtx(uint8_t data)
{
uint8_t d=0;
    //
    // Чистим буфер
///    spi_i2s_data_receive(SPI_N);
    //
    if(spi_i2s_flag_get(SPI_N, SPI_FLAG_TBE) == SET) {
        d = data;
        spi_i2s_data_transmit(SPI_N, d);
        while(spi_i2s_flag_get(SPI_N, SPI_I2S_INT_FLAG_RBNE) == RESET) {}
        d = spi_i2s_data_receive(SPI1);
        }
    //
    return(d);
}


#define enc28j60_rx() enc28j60_rxtx(0xff)
#define enc28j60_tx(data) enc28j60_rxtx(data)

// Generic SPI read command
uint8_t enc28j60_read_op(uint8_t cmd, uint8_t adr)
{
	uint8_t data;

	///enc28j60_select();
	SELECT=0;
	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
	if(adr & 0x80) // throw out dummy byte
		enc28j60_rx(); // when reading MII/MAC register
	data = enc28j60_rx();
	///enc28j60_release();
	SELECT=1;
	return data;
}

// Generic SPI write command
void enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
	///enc28j60_select();
	SELECT=0;
	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
	enc28j60_tx(data);
	///enc28j60_release();
	SELECT=1;
}

// Initiate software reset
void enc28j60_soft_reset()
{
	SELECT=0;
	enc28j60_tx(ENC28J60_SPI_SC);
	SELECT=1;
	enc28j60_current_bank = 0;
//
    vTaskDelay(1);
}


/*
 * Memory access
 */

// Set register bank
void enc28j60_set_bank(uint8_t adr)
{
	uint8_t bank;

	if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != enc28j60_current_bank)
		{
			enc28j60_write_op(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_op(ENC28J60_SPI_BFS, ECON1, bank);
			enc28j60_current_bank = bank;
		}
	}
}

// Read register
uint8_t enc28j60_rcr(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t enc28j60_rcr16(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr) |
		(enc28j60_read_op(ENC28J60_SPI_RCR, adr+1) << 8);
}

// Write register
void enc28j60_wcr(uint8_t adr, uint8_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16(uint8_t adr, uint16_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfc(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfs(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_buffer(uint8_t *buf, uint16_t len)
{
	SELECT=0;
	enc28j60_tx(ENC28J60_SPI_RBM);
	while(len--) *(buf++) = enc28j60_rx();
	SELECT=1;
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_buffer(uint8_t *buf, uint16_t len)
{
	SELECT=0;
	enc28j60_tx(ENC28J60_SPI_WBM);
	while(len--) enc28j60_tx(*(buf++));
	SELECT=1;
}

// Read PHY register
uint16_t enc28j60_read_phy(uint8_t adr)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_bfs(MICMD, MICMD_MIIRD);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY)
		;
	enc28j60_bfc(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16(MIRD);
}

// Write PHY register
void enc28j60_write_phy(uint8_t adr, uint16_t data)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_wcr16(MIWR, data);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY)
		;
}


/*
 * Init & packet Rx/Tx
 */

void enc28j60_init(uint8_t *macadr)
{
	// INT
//	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    SELECT=1;
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    RESET=1;

    gpio_init(SPI_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SCK_E | MOSI_E);
    gpio_init(SPI_PORT, GPIO_MODE_IN_FLOATING, 0, MISO_E);
/*
	SPI_InitTypeDef SPI_InitStruct;

	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;

	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI_N, &SPI_InitStruct);
	SPI_Cmd(SPI_N, ENABLE);
*/

///
    gpio_pin_remap_config(GPIO_SPI0_REMAP, ENABLE);
    rcu_periph_clock_enable(RCU_SPI0);
    spi_i2s_deinit(SPI0);

    spi_parameter_struct spi;
    spi_struct_para_init(&spi);
    /* configure SPI0 parameter */
    spi.prescale             = SPI_PSC_2;
    spi.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi.device_mode          = SPI_MASTER;
    spi.frame_size           = SPI_FRAMESIZE_8BIT;
    spi.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi.nss                  = SPI_NSS_SOFT;
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
	RESET=0;
	vTaskDelay(5);
    RESET=1;
///////////////////////////////////////////////////////////////////////////////
	// Reset ENC28J60
	enc28j60_soft_reset();

	// Setup Rx/Tx buffer
	enc28j60_wcr16(ERXST, ENC28J60_RXSTART);
	enc28j60_rcr16(ERXST);

	enc28j60_wcr16(ERXRDPT, ENC28J60_RXSTART);
	enc28j60_wcr16(ERXND, ENC28J60_RXEND);
	enc28j60_rxrdpt = ENC28J60_RXSTART;

	// Setup MAC
	enc28j60_wcr(MACON1, MACON1_TXPAUS| // Enable flow control
		MACON1_RXPAUS|MACON1_MARXEN); // Enable MAC Rx

	enc28j60_wcr(MACON2, 0); // Clear reset

	enc28j60_wcr(MACON3, MACON3_PADCFG0| // Enable padding,
		MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX); // Enable crc & frame len chk

	enc28j60_wcr16(MAMXFL, ENC28J60_MAXFRAME);

	enc28j60_wcr(MABBIPG, 0x15); // Set inter-frame gap
	enc28j60_wcr(MAIPGL, 0x12);
	enc28j60_wcr(MAIPGH, 0x0c);

	enc28j60_wcr(MAADR5, macadr[0]); // Set MAC address
	enc28j60_wcr(MAADR4, macadr[1]);
	enc28j60_wcr(MAADR3, macadr[2]);
	enc28j60_wcr(MAADR2, macadr[3]);
	enc28j60_wcr(MAADR1, macadr[4]);
	enc28j60_wcr(MAADR0, macadr[5]);

	// Setup PHY
	enc28j60_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
	enc28j60_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback

	enc28j60_write_phy(PHLCON, // Configure LED ctrl
		PHLCON_LACFG2 | PHLCON_LBCFG2 | PHLCON_LBCFG1 | PHLCON_LBCFG0 | PHLCON_LFRQ0 | PHLCON_STRCH);

	// Enable Rx packets
	enc28j60_bfs(ECON1, ECON1_RXEN);
	//
	//enc28j60_bfs(EIE, EIE_INTIE | EIE_TXIE);
}

void enc28j60_send_packet(uint8_t *data, uint16_t len)
{
	while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error
		if(enc28j60_rcr(EIR) & EIR_TXERIF)
		{
			enc28j60_bfs(ECON1, ECON1_TXRST);
			enc28j60_bfc(ECON1, ECON1_TXRST);
		}
	}

	enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_buffer((uint8_t*)"\x00", 1);
	enc28j60_write_buffer(data, len);

	enc28j60_wcr16(ETXST, ENC28J60_TXSTART); // 0x04
	enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len); // 0x06

	enc28j60_bfs(ECON1, ECON1_TXRTS); // Request packet send
}

uint16_t enc28j60_recv_packet(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0, rxlen, status, temp;

	if(enc28j60_rcr(EPKTCNT))
	{
		enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

		enc28j60_read_buffer((void*)&enc28j60_rxrdpt, sizeof(enc28j60_rxrdpt));
		enc28j60_read_buffer((void*)&rxlen, sizeof(rxlen));
		enc28j60_read_buffer((void*)&status, sizeof(status));

		if(status & 0x80) //success
		{
			len = rxlen - 4; //throw out crc
			if(len > buflen) len = buflen;
			enc28j60_read_buffer(buf, len);
		}

		// Set Rx read pointer to next packet
		temp = (enc28j60_rxrdpt - 1) & ENC28J60_BUFEND;
		enc28j60_wcr16(ERXRDPT, temp);

		// Decrement packet counter
		enc28j60_bfs(ECON2, ECON2_PKTDEC);
	}

	return len;
}
