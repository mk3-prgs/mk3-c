#ifndef __SQ_H
#define __SQ_H
//
#include "ff.h"
#include "MX25L.h"
//
typedef struct _sq_io_
{
    BYTE rw;
    BYTE drv;		/* Physical drive number (0) */
	BYTE *buff;		/* Pointer to the data buffer to store read data */
	DWORD sector;	/* Start sector number (LBA) */
	UINT count;		/* Number of sectors to read (1..128) */
} sq_io;

DSTATUS sq_initialize(void);
DSTATUS sq_status(void);
FRESULT sq_ioctl(BYTE cmd, void *buff);

void sq_timerproc(void);

FRESULT sq_read(BYTE *buff, DWORD sector, UINT cnt);
FRESULT sq_write(const BYTE *buff, DWORD sector, UINT cnt);

#endif
