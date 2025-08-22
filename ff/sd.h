#ifndef _SD_H_
#define _SD_H_

//#include <stdint.h>
//#include "sdio_sd.h"
#include "diskio.h"

typedef struct __sdif__ {
	int initialized;
	int sectors;
	int erase_sectors;
	int capabilities;
} sdif;

void sd_deinit(void);
DSTATUS sd_initialize(void);
DSTATUS sd_status(void);
DRESULT sd_read(BYTE *buff, DWORD sector, UINT cnt);
DRESULT sd_write(const BYTE *buff, DWORD sector, UINT cnt);
DRESULT sd_ioctl(BYTE ctrl, void *buff);
void sd_timerproc(void);

#endif
