/*** fatfs code that uses the public API ***/

#include "diskio.h"
#include "xprintf.h"
#include "sdcard.h"
#include "sd.h"

static uint8_t sta=0;
sd_card_info_struct sd_cardinfo;

sd_error_enum sd_io_init(void);

DSTATUS sd_initialize(void)
{
sd_error_enum sd_error;
int i;
    if(sta==0) {
        //nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
        nvic_irq_enable(SDIO_IRQn, 0, 0);
        //
        i=5;
        do {
            sd_error = sd_io_init();
            } while((SD_OK != sd_error) && (--i));

        if(i){
            xprintf("\r\n Card init success!\r\n");
            sta=1;
            return RES_OK;
            }
        else {
            xprintf("Card init failed!\n");
            return STA_NODISK;
            }
        }
    else {
        return RES_OK;
        }
}


DSTATUS sd_status(void)
{
	if(sta==0) return STA_NOINIT;
	else return 0;
}

//DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
//SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
//SD_ReadBlock(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize);
DRESULT sd_read(BYTE *buff, DWORD sector, UINT count)
{
sd_error_enum sd_error;
UINT i;
DRESULT res=RES_OK; // RES_ERROR
int n=5;
	//
	//xprintf("sd_read: sector=%lu count=%u\n", sector, count);
	//
	//led_sd(1);
	//
	for(i=0; i<count; i++) {
        for(;;) {
            sd_error = sd_block_read((uint32_t*)(buff+512*i), 512*(sector+i), 512);
        //
            if(sd_error != SD_OK) {
                n--;
                if(n>0) continue;
                xprintf("Block %lu read fail! err: %s\n", (sector+i), err_to_str(sd_error));
                res = RES_ERROR;
                break;
                }
            else break;
            }
        //
        if(res == RES_ERROR) break;
        }
    //
    //led_sd(0);
    return(res);
}


#if _READONLY == 0

DRESULT sd_write(const BYTE *buff, DWORD sector, UINT count)
{
sd_error_enum sd_error;
UINT i;
DRESULT res=RES_OK; // RES_ERROR
int n=5;
    //
    //xprintf("sd_write: sector=%lu count=%u\n", sector, count);
    //led_sd(1);
    //
	for(i=0; i<count; i++) {
        for(;;) {
            sd_error = sd_block_write((uint32_t*)(buff+512*i), 512*(sector+i), 512);
            //
            if(SD_OK != sd_error) {
                n--;
                if(n>0) continue;
                xprintf("%d Block %lu write fail! err: %s\n", n, sector+i, err_to_str(sd_error));
                res = RES_ERROR;
                break;
                }
            else break;
            }
        if(res == RES_ERROR) break;
        }
    //
    //led_sd(0);
    //
    return(res);
}

#endif /* _READONLY */

//#define MMC_GET_TYPE		50	/* Get card type */
//#define MMC_GET_CSD			51	/* Read CSD */
//#define MMC_GET_CID			52	/* Read CID */
//#define MMC_GET_OCR			53	/* Read OCR */
//#define MMC_GET_SDSTAT		54	/* Read SD status */
//#define ISDIO_READ			55	/* Read data form SD iSDIO register */
//#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
//#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

DRESULT sd_ioctl(BYTE ctrl, void *buff)
{
	switch (ctrl) {
	case CTRL_SYNC:
		return RES_OK;

	case GET_SECTOR_SIZE:
		*(WORD*)buff = 512;
		return RES_OK;

	case GET_SECTOR_COUNT:
		*(DWORD*)buff = ((sd_cardinfo.card_csd.c_size + 1)*1024); //hw.sectors;
		return RES_OK;

	case GET_BLOCK_SIZE:
		*(DWORD*)buff = 1; //hw.erase_sectors;
		return RES_OK;

    case MMC_GET_TYPE:
		*(BYTE*)buff = (sd_scr[1] & 0x00700000) >> 20;
		return RES_OK;

    case MMC_GET_CSD:
        //xprintf("GET_CSD: %d\n", sizeof(sd_csd_struct));
        get_sd_csd((uint32_t*)buff);
		return RES_OK;

    case MMC_GET_CID:
        //xprintf("GET_CSD: %d\n", sizeof(sd_cid_struct));
		get_sd_cid((uint32_t*)buff);
		return RES_OK;
	}
	return RES_PARERR;
}


/*
 * FAT filestamp format:
 * [31:25] - year - 1980
 * [24:21] - month 1..12
 * [20:16] - day 1..31
 * [15:11] - hour 0..23
 * [10:5]  - minute 0..59
 * [4:0]   - second/2 0..29
 * so... midnight 2009 is 0x3a000000
 */
//DWORD get_fattime()
//{
//	int time = RTC_GetCounter();
//	int y, m, d;
//	epoch_days_to_date(time/DAY_SECONDS, &y, &m, &d);
//	time %= DAY_SECONDS;
//	return (y-1980)<<25 | m<<21 | d<<16 |
//		(time/3600)<<11 | (time/60%60)<<5 | (time/2%30);
//}

sd_error_enum sd_io_init(void)
{
sd_error_enum status = SD_OK;
uint32_t cardstate = 0;

    status = sd_init();

    if(SD_OK == status) {
        status = sd_card_information_get(&sd_cardinfo);
        }

    if(SD_OK == status){
        status = sd_card_select_deselect(sd_cardinfo.card_rca);
        }

    status = sd_cardstatus_get(&cardstate);

    if(cardstate & 0x02000000){
        xprintf("The card is locked!\n");
        //while (1){}
        return status;
        }

    if ((SD_OK == status) && (!(cardstate & 0x02000000))) {
        /* set bus mode */
        status = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
//        status = sd_bus_mode_config( SDIO_BUSMODE_1BIT );
        }
    if(SD_OK == status) {
        /* set data transfer mode */
        ///status = sd_transfer_mode_config( SD_DMA_MODE );
        status = sd_transfer_mode_config( SD_POLLING_MODE );
        }

    return status;
}

void SDIO_IRQHandler(void)
{
    sd_interrupts_process();
    ///LED_SD=0;
}

