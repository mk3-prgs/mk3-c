#ifndef __FM_H
#define __FM_H
//
#include "ff.h"
#include "flash.h"
#include "diskio.h"
//
typedef struct _fm_io_
{
	uint8_t *buff;		/* Pointer to the data buffer to store read data */
	uint32_t sector;	/* Start sector number (LBA) */
	uint32_t count;		/* Number of sectors to read (1..128) */
} fm_io;

DSTATUS fm_initialize(void);
DSTATUS fm_status(void);
DRESULT fm_ioctl(BYTE cmd, void *buff);

DRESULT fm_read(BYTE *buff, DWORD sector, UINT cnt);
DRESULT fm_write(const BYTE *buff, DWORD sector, UINT cnt);

#endif
