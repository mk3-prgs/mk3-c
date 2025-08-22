#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ff.h"
#include "sq.h"
#include "diskio.h"
//
#include "xprintf.h"

w25qxx_t desc;

bool W25qxx_Init(w25qxx_t* desc);

static sq_io d_io;
//disk_io d_wr;
static QueueHandle_t io_queue=0;

static void cr_que(int d)
{
    if(d) {
        if(io_queue == 0) {
            io_queue =  xQueueCreate(1, sizeof(sq_io));
            xprintf("Create io_queue = %u\n", io_queue);
            }
        }
    else {
        vQueueDelete(io_queue);
        io_queue=0;
        ///xprintf("Delete io_queue = %u\n", io_queue);
        }
}

static volatile DSTATUS Stat = 0x01;	/* Physical drive status */
//extern df_Info_t df_Info;

DSTATUS sq_initialize(void) /* Physical drive number (0) */
{
int ty=1;
	//
    ///ty = df_Init();
    if(Stat != 0) ty = !W25qxx_Init(&desc);
    else ty = 0;
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
    ///xprintf("sf_initialize: %s %u pages Stat:%x\n", df_Info.name, df_Info.pages, Stat);
    //
	return Stat;
}


/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS sq_status(void)
{
	return(Stat);	/* Return disk status */
}

/*-----------------------------------------------------------------------*/
/* Read sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT qu_disk_read (
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
    )
{
    ///xprintf("========== disk_read(buff:0x%08x sec:%d cnt:%d);\n", buff, sector, count);
	if (Stat & STA_NOINIT) return RES_NOTRDY;	// Check if drive is ready

	//if (!(CardType & CT_BLOCK)) sector *= 512;	// LBA ot BA conversion (byte addressing cards)
//	deselect();
    //count *= 2;
    if(count) {
        do {
            ///df_PageFunc(DF_FLASH_TO_BUF1, sector++);
            ///df_Read(1, 0, 512, buff);
            Read(512 * sector, buff, 512);
            sector++;
            count--;
            buff += 512;
            } while(count);
        }
    //
	return count ? RES_ERROR : RES_OK;	/* Return result */
}



/*-----------------------------------------------------------------------*/
/* Write sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT qu_disk_write (
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
    ///xprintf("========== disk_write(buff[0]=%02x sec:%d, cnt:%d);\n", buff[0], sector, count);

	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status */
	if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect */
/*
	if (!(CardType & CT_BLOCK)) sector *= 512;	// LBA ==> BA conversion (byte addressing cards)

	deselect();
*/
    count *= 2;
    uint32_t addr = 512 * sector;
    if(count) {
        do {
            //xprintf("    erase: %08x\n", addr);
            //SectorErase(addr);
            //vTaskDelay(10);
            ///xprintf("    write: %08x bf:%08x cnt:%d\n", addr, buff, count);
            Write(addr, buff, 256);
            //
            addr += 256;
            count--;
            buff += 256;
            } while(count);
        }
    //
	return count ? RES_ERROR : RES_OK;	/* Return result */
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/

FRESULT sq_ioctl (
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
		csize = 32768; //desc.SectorCount; //df_Info.pages;
		*(DWORD*)buff = csize; // << 11;
		res = RES_OK;
		break;

    case GET_SECTOR_SIZE:
		*(WORD*)buff = 512; //desc.SectorSize;
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

void sq_timerproc(void)
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

FRESULT sq_read (
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
    )
{
portBASE_TYPE xStatus;
int ret=0;
    if(io_queue == 0) {
        return(FR_NOT_ENABLED);
        }

    d_io.buff = buff;
    d_io.count = count;
    d_io.drv = 0; //0
    d_io.sector = sector;
    d_io.rw = 0;
    xStatus = xQueueSend(io_queue, &d_io, 1000);
    //
    if(xStatus == pdPASS) {
        ret = qu_disk_read(buff, sector, count);
        }
    else {
        ret = FR_TIMEOUT;
        }
    //
    xQueueReceive(io_queue, &d_io, 0);
    return(ret);
}

FRESULT sq_write (
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
portBASE_TYPE xStatus;
int ret=0;
    if(io_queue == 0) {
        return(FR_NOT_ENABLED);
        }

    d_io.buff = (BYTE*)buff;
    d_io.count = count;
    d_io.drv = 0; //0
    d_io.sector = sector;
    d_io.rw = 1;
    xStatus = xQueueSend(io_queue, &d_io, 1000);
    //
    if(xStatus == pdPASS) {
        ret = qu_disk_write(buff, sector, count);
        }
    else {
        ret = FR_TIMEOUT;
        }
    //
    xQueueReceive(io_queue, &d_io, 0);
    return(ret);
}

static int is_inited=0;

bool W25qxx_Init(w25qxx_t* desc)
{
	desc->Lock = 1;
	uint32_t id;

	xprintf("w25qxx Init Begin...\r\n");

    if(is_inited==0) { spi_df_Init(); is_inited=1; }
	id = Get_Identification();


	xprintf("w25qxx ID:0x%X\r\n", id);

	switch (id & 0x000000FF)
	{
	case 0x20: // 	w25q512
		desc->ID = W25Q512;
		desc->BlockCount = 1024;

		xprintf("w25qxx Chip: w25q512\r\n");

		break;
	case 0x19: // 	w25q256
		desc->ID = W25Q256;
		desc->BlockCount = 512;

		xprintf("w25qxx Chip: w25q256\r\n");

		break;
	case 0x18: // 	w25q128
		desc->ID = W25Q128;
		desc->BlockCount = 256;

		xprintf("w25qxx Chip: w25q128\r\n");

		break;
	case 0x17: //	w25q64
		desc->ID = W25Q64;
		desc->BlockCount = 128;

		xprintf("w25qxx Chip: w25q64\r\n");

		break;
	case 0x16: //	w25q32
		desc->ID = W25Q32;
		desc->BlockCount = 64;

		xprintf("w25qxx Chip: w25q32\r\n");

		break;
	case 0x15: //	w25q16
		desc->ID = W25Q16;
		desc->BlockCount = 32;

		xprintf("w25qxx Chip: w25q16\r\n");

		break;
	case 0x14: //	w25q80
		desc->ID = W25Q80;
		desc->BlockCount = 16;

		xprintf("w25qxx Chip: w25q80\r\n");

		break;
	case 0x13: //	w25q40
		desc->ID = W25Q40;
		desc->BlockCount = 8;

		xprintf("w25qxx Chip: w25q40\r\n");

		break;
	case 0x12: //	w25q20
		desc->ID = W25Q20;
		desc->BlockCount = 4;

		xprintf("w25qxx Chip: w25q20\r\n");

		break;
	case 0x11: //	w25q10
		desc->ID = W25Q10;
		desc->BlockCount = 2;

		xprintf("w25qxx Chip: w25q10\r\n");

		break;
	default:

		xprintf("w25qxx Unknown ID\r\n");

		desc->Lock = 0;
		return false;
	}
	desc->PageSize = 256;
	desc->SectorSize = 0x1000;
	desc->SectorCount = desc->BlockCount * 16;
	desc->PageCount = (desc->SectorCount * desc->SectorSize) / desc->PageSize;
	desc->BlockSize = desc->SectorSize * 16;
	desc->CapacityInKiloByte = (desc->SectorCount * desc->SectorSize) / 1024;

	///W25qxx_ReadUniqID(per);
	ReadUniqID(desc);

	ReadStatusRegister(desc, 1);
	ReadStatusRegister(desc, 2);
	ReadStatusRegister(desc, 3);

	xprintf("w25qxx Page Size: %d Bytes\n", desc->PageSize);
	xprintf("w25qxx Page Count: %d\n", desc->PageCount);
	xprintf("w25qxx Sector Size: %d Bytes\n", desc->SectorSize);
	xprintf("w25qxx Sector Count: %d\n", desc->SectorCount);
	xprintf("w25qxx Block Size: %d Bytes\n", desc->BlockSize);
	xprintf("w25qxx Block Count: %d\n", desc->BlockCount);
	xprintf("w25qxx Capacity: %d KiloBytes\n", desc->CapacityInKiloByte);
	//
	xprintf("w25qxx Status Registers: ");
    xprintf("r1=%02x ", desc->StatusRegister1);
    xprintf("r2=%02x ", desc->StatusRegister2);
    xprintf("r3=%02x ", desc->StatusRegister3);
    xprintf("\n");
    //
	xprintf("w25qxx UniqID: ");
	for(int i=0;i<4;i++) {
        xprintf("%02x ", desc->UniqID[i]);
        }
    xprintf("\n");
    //
    xprintf("w25qxx Init Done\n");
	desc->Lock = 0;
	return true;
}
