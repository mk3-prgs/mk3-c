#ifndef __SF_H
#define __SF_H
//
#include "ff.h"
#include "diskio.h"
#include "at45db.h"

//
typedef struct _sf_io_
{
	uint8_t *buff;		/* Pointer to the data buffer to store read data */
	uint32_t sector;	/* Start sector number (LBA) */
	uint32_t count;		/* Number of sectors to read (1..128) */
} sf_io;

DSTATUS sf_initialize(void);
DSTATUS sf_status(void);
DRESULT sf_ioctl(BYTE cmd, void *buff);

void sf_timerproc(void);

DRESULT sf_read(BYTE *buff, DWORD sector, UINT cnt);
DRESULT sf_write(const BYTE *buff, DWORD sector, UINT cnt);

#endif
