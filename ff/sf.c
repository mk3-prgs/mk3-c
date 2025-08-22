#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ff.h"
#include "sf.h"
#include "diskio.h"
//
#include "xprintf.h"


//static sf_io d_io;
//disk_io d_wr;
static QueueHandle_t io_queue=0;

static void cr_que(int d)
{
    if(d) {
        if(io_queue == 0) {
            io_queue =  xQueueCreate(1, sizeof(sf_io));
            ///xprintf("Create io_queue = %u\n", io_queue);
            }
        }
    else {
        vQueueDelete(io_queue);
        io_queue=0;
        ///xprintf("Delete io_queue = %u\n", io_queue);
        }
}

static volatile DSTATUS Stat = 0x01;	/* Physical drive status */
extern df_Info_t df_Info;

DSTATUS sf_initialize(void) /* Physical drive number (0) */
{
int ty=1;
	//
    ty = df_Init();
    //
    Stat &= ~(STA_NODISK | STA_PROTECT);
    //
    if(ty == 0) {			/* OK */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT flag */
        }
    else {			/* Failed */
		Stat = STA_NOINIT;
        }
    //
    cr_que(1);
    //
    //xprintf("sf_initialize: %s %u pages Stat:%x\n", df_Info.name, df_Info.pages, Stat);
    //
	return Stat;
}


/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS sf_status(void)
{
	return(Stat);	/* Return disk status */
}



/*-----------------------------------------------------------------------*/
/* Read sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT sf_disk_read (
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
    )
{
    //xprintf("========== disk_read(buff, %d, %d);\n", sector, count);
	if (Stat & STA_NOINIT) return RES_NOTRDY;	// Check if drive is ready

	//if (!(CardType & CT_BLOCK)) sector *= 512;	// LBA ot BA conversion (byte addressing cards)
//	deselect();
    if(count) {
        do {
            df_PageFunc(DF_FLASH_TO_BUF1, sector++);
            df_Read(1, 0, 512, buff);
            count--;
            buff+=512;
            } while(count);
        }
    //
	return count ? RES_ERROR : RES_OK;	/* Return result */
}



/*-----------------------------------------------------------------------*/
/* Write sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT sf_disk_write (
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
    //xprintf("========== disk_write(buff, %d, %d);\n", sector, count);

	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status */
	if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect */
/*
	if (!(CardType & CT_BLOCK)) sector *= 512;	// LBA ==> BA conversion (byte addressing cards)

	deselect();
*/
    if(count) {
        do {
            df_Write(1, 0, 512, (BYTE*)buff);
            df_PageFunc(DF_BUF1_TO_FLASH_WITH_ERASE, sector++);
            count--;
            buff+=512;
            } while(count);
        }
    //
	return count ? RES_ERROR : RES_OK;	/* Return result */
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/

DRESULT sf_ioctl (
	BYTE cmd,		/* Control command code */
	void *buff		/* Pointer to the conrtol data */
)
{
	DRESULT res;
	//BYTE n, csd[16];
	//DWORD *dp, st, ed,
	DWORD csize;

	//xprintf("========== disk_ioctl(%d, buff);\n", cmd);

	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		//if (select())
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		csize = df_Info.pages;
		*(DWORD*)buff = csize; // << 11;
		res = RES_OK;
		break;

    case GET_SECTOR_SIZE:
		*(WORD*)buff = 512;
        res = RES_OK;
        break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		*(DWORD*)buff = 1;
		res = RES_OK;
		break;

	case CTRL_TRIM :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	//deselect();

	return res;
}


/*-----------------------------------------------------------------------*/
/* Device timer function                                                 */
/*-----------------------------------------------------------------------*/
/* This function must be called from timer interrupt routine in period
/  of 1 ms to generate card control timing.
*/

#define	MMC_CD		0 /* Card detect (yes:true, no:false, default:true) */
#define	MMC_WP		0 /* Write protected (yes:true, no:false, default:false) */

void sf_timerproc(void)
{
//	WORD n;
	BYTE s;

	//n = Timer1;						/* 1kHz decrement timer stopped at 0 */
	//if(n) Timer1 = --n;

	//n = Timer2;
	//if(n) Timer2 = --n;

	s = Stat;
    /*
	if(MMC_WP) {
		s |= STA_PROTECT;
        }
	else {
		s &= ~STA_PROTECT;
        }
    //
	if(MMC_CD) {
		s &= ~STA_NODISK;
        }
	else {
		s |= (STA_NODISK | STA_NOINIT);
        }
    */
    s &= ~STA_NODISK;
    s &= ~STA_PROTECT;
	Stat = s;
	//(STA_NODISK | STA_NOINIT | STA_PROTECT)
}
///==============================================================================

DRESULT sf_read (
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
    )
{
sf_io d_io;

portBASE_TYPE xStatus;
int ret=0;
    if(io_queue == 0) {
        return(FR_NOT_ENABLED);
        }

    d_io.buff = buff;
    d_io.count = count;
    d_io.sector = sector;

    xStatus = xQueueSend(io_queue, &d_io, 1000);
    //
    if(xStatus == pdPASS) {
        ret = sf_disk_read(buff, sector, count);
        }
    else {
        ret = FR_TIMEOUT;
        }
    //
    xQueueReceive(io_queue, &d_io, 0);
    return(ret);
}

DRESULT sf_write (
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
sf_io d_io;
//
portBASE_TYPE xStatus;
int ret=0;
    if(io_queue == 0) {
        return(FR_NOT_ENABLED);
        }

    d_io.buff = (BYTE*)buff;
    d_io.count = count;
    d_io.sector = sector;
    xStatus = xQueueSend(io_queue, &d_io, 1000);
    //
    if(xStatus == pdPASS) {
        ret = sf_disk_write(buff, sector, count);
        }
    else {
        ret = FR_TIMEOUT;
        }
    //
    xQueueReceive(io_queue, &d_io, 0);
    return(ret);
}
