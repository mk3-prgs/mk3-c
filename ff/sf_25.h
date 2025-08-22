#ifndef __MF_H
#define __MF_H
//
#include "ff.h"
#include "flash.h"
#include "diskio.h"
#include "MX25L.h"
//
typedef struct _mf_io_
{
	uint8_t *buff;		/* Pointer to the data buffer to store read data */
	uint32_t sector;	/* Start sector number (LBA) */
	uint32_t count;		/* Number of sectors to read (1..128) */
} mf_io;

uint8_t mf_Init(void);
DSTATUS mf_initialize(void);
DSTATUS mf_status(void);
DRESULT mf_read(BYTE *buff, DWORD sector, UINT cnt);
DRESULT mf_write(const BYTE *buff, DWORD sector, UINT cnt);

DRESULT mf_ioctl(BYTE cmd, void *buff);
void mf_timerproc(void);

#endif
