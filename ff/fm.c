#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "xprintf.h"
//
#include "fm.h"
//
DSTATUS fm_initialize(void)
{
DSTATUS res=0;

    return(res);
}

DSTATUS fm_status(void)
{
DSTATUS res=0;

    return(res);
}

DRESULT fm_ioctl(BYTE cmd, void *buff)
{
DRESULT res=0;
DWORD csize;
	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		csize = (FMC_END_ADDR - FMC_START_ADDR) / FMC_PAGE_SIZE;
		*(DWORD*)buff = csize; // << 11;
		res = RES_OK;
		break;

    case GET_SECTOR_SIZE:
		*(WORD*)buff = FMC_PAGE_SIZE;
        res = RES_OK;
        break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		*(DWORD*)buff = 1;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
		break;
	}
	return res;
}

DRESULT fm_read(BYTE *buff, DWORD sector, UINT cnt)
{
DRESULT res=RES_OK;

    int page = sector;
    int numb = (FMC_PAGE_SIZE * cnt)/4;
    fmc_read(page, (uint32_t*)buff, numb);

    return(res);
}

DRESULT fm_write(const BYTE *buff, DWORD sector, UINT cnt)
{
DRESULT res=RES_OK;

    int page = sector;
    int numb = (FMC_PAGE_SIZE * cnt)/4;
    fmc_program(page, (uint32_t*)buff, numb);

    return(res);
}

