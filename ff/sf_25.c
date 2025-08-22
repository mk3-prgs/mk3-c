#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ff.h"
#include "sf_25.h"
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
            io_queue =  xQueueCreate(1, sizeof(mf_io));
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

// Описание возможностей подключенной микросхемы.
w25qxx_t  w25_Info;

static int is_inited=0;

uint8_t mf_Init(void)
{
    if(!is_inited) {
        spi_df_Init();
        is_inited=1;
        }
    //
    Get_Identification(&w25_Info);
    ReadUniqID(&w25_Info);
    ReadStatusRegister(&w25_Info, 1);
    ReadStatusRegister(&w25_Info, 2);
    ReadStatusRegister(&w25_Info, 3);
    //
    return(0); // 0 - Ok
}


DSTATUS mf_initialize(void) /* Physical drive number (0) */
{
int ty=1;
	//
    ty = mf_Init();
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

DSTATUS mf_status(void)
{
	return(Stat);	/* Return disk status */
}

static uint32_t cur_page=0xffffffff;
static uint8_t rw_bf[4096];
static int erase=0;

static void set_cp(uint32_t sector)
{
uint32_t page = sector/8;
int i;
uint32_t addr;
uint8_t *pbf;
    if(page != cur_page) {
        if(erase != 0) {
            SectorErase(0x1000 * cur_page);
            erase=0;
            pbf = rw_bf;
            for(i=0; i<16; i++) {
                addr = (0x1000 * cur_page) + (256*i);
                Write(addr, pbf, 256);
                pbf += 256;
                }
            //xprintf("Write %d page.\n", 8*cur_page);
            }
        cur_page = page;
        Read(0x1000 * cur_page, rw_bf, 4096);
        //xprintf("Read %d page.\n", 8*cur_page);
        }
}

static void sync(void)
{
int i;
uint32_t addr;
uint8_t *pbf;
    if((cur_page != 0xffffffff)||(erase != 0)) {
        SectorErase(0x1000 * cur_page);
        erase=0;
        pbf = rw_bf;
        for(i=0; i<16; i++) {
            addr = (0x1000 * cur_page) + (256*i);
            Write(addr, pbf, 256);
            pbf += 256;
            }
        }
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
int i;
	if (Stat & STA_NOINIT) return RES_NOTRDY;	// Check if drive is ready
    while(count) {
        set_cp(sector);
        uint8_t *ps, *pd;
        ps = rw_bf + (512*(sector%8));
        pd = buff;
        for(i=0; i<512; i++) {
            *pd++ = *ps++;
            }
        //
        sector++;
        count--;
        buff+=512;
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
int i;
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status */
	if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect */
    //
    while(count) {
        set_cp(sector);
        uint8_t *ps, *pd;
        pd = rw_bf + (512*(sector%8));
        ps = (uint8_t*)buff;
        for(i=0; i<512; i++) {
            *pd++ = *ps++;
            erase=1;
            }
        //
        sector++;
        count--;
        buff+=512;
        }
    //
	return count ? RES_ERROR : RES_OK;	/* Return result */
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/

DRESULT mf_ioctl (
	BYTE cmd,		/* Control command code */
	void *buff		/* Pointer to the conrtol data */
)
{
DRESULT res;
	//BYTE n, csd[16];
	//DWORD *dp, st, ed,
DWORD csize;
DWORD *pt;
uint32_t b;
uint32_t e;
	//xprintf("========== disk_ioctl(%d, buff);\n", cmd);

	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		sync();
		///xprintf("=============================== SYNC\n");
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		csize = w25_Info.SectorCount * 8;
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
        pt = (DWORD*)buff;
        b = *pt++;
        e = *pt;
        xprintf("=======================   TRIM: b=%08x e=%08x\n", b, e);
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
		break;
	}

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

void mf_timerproc(void)
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

DRESULT mf_read (
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
    )
{
mf_io d_io;

portBASE_TYPE xStatus;
DRESULT ret=RES_OK;
    if(io_queue == 0) {
        return(RES_NOTRDY);
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
        ret = RES_NOTRDY;
        }
    //
    xQueueReceive(io_queue, &d_io, 0);
    return(ret);
}

DRESULT mf_write (
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
mf_io d_io;
//
portBASE_TYPE xStatus;
int ret=0;
    if(io_queue == 0) {
        return(RES_NOTRDY);
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
        ret = RES_NOTRDY;
        }
    //
    xQueueReceive(io_queue, &d_io, 0);
    return(ret);
}
