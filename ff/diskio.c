#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_SF		0
#define DEV_FM		2	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_SD		1	/* Example: Map MMC/SD card to physical drive 1 */

#include "sd.h"
#include "sf.h"
#include "fm.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv)
{
	DSTATUS stat=0;

	switch (pdrv) {
        case DEV_SF :
            stat = sf_status();
            return stat;

        case DEV_FM :
            return 0;

        case DEV_SD :
            stat = sd_status();
            return stat;
        }
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv)
{
	DSTATUS stat=0;

	switch (pdrv) {
        case DEV_SF :
            stat= sf_initialize();
            return stat;

        case DEV_FM :
            return 0;

        case DEV_SD :
            stat= sd_initialize();
            return stat;
        }

	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res=RES_PARERR;

	switch (pdrv) {
        case DEV_SF :
            res = sf_read(buff, sector, count);
            break;

        case DEV_FM :
            res = fm_read(buff, sector, count);
            break;

        case DEV_SD :
            res = sd_read(buff, sector, count);
            break;
        }
	return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res=RES_PARERR;

	switch (pdrv) {
        case DEV_SF :
            res = sf_write(buff, sector, count);
            break;

        case DEV_FM :
            res = fm_write(buff, sector, count);
            break;

        case DEV_SD :
            res = sd_write(buff, sector, count);
            break;
        }

	return res;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res=0;

	switch (pdrv) {
        case DEV_SF :
            res = sf_ioctl(cmd, buff);
            return res;

        case DEV_FM :
            res = fm_ioctl(cmd, buff);
            return res;

        case DEV_SD :
            res = sd_ioctl(cmd, buff);
            return res;
        }

	return RES_PARERR;
}

void disk_timerproc(void)
{
    sf_timerproc();
}

