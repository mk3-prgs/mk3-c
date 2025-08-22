#include "shell.h"

// Show disk status
void ds(char *s)
{
long t;
UINT s1;
BYTE b;
uint16_t s_s=0;
BYTE dev=cur_vol();
uint32_t d;

char *ptr = &s[3];
    if(xatoi(&ptr, &t)) {
        dev = t;
        }
    //p1 = 0;
    xprintf("Drive: %lu\n", dev);
    //
    char *Buff = (char*)pvPortMalloc(512);
    if(Buff != NULL) {
        if (disk_ioctl(dev, GET_SECTOR_COUNT, &d) == RES_OK)
            { xprintf("Sector count: %lu sectors\n", d); }

        if (disk_ioctl(dev, GET_SECTOR_SIZE, &s_s) == RES_OK)
            { xprintf("Sector size: %lu bytes\n", s_s); }

        if (disk_ioctl(dev, GET_BLOCK_SIZE, &d) == RES_OK)
            { xprintf("Block size: %lu sectors\n", d); }

        if (disk_ioctl(dev, MMC_GET_TYPE, &b) == RES_OK)
            { xprintf("Media type: %u\n", b); }


        if (disk_ioctl(dev, MMC_GET_CSD, Buff) == RES_OK)
            { xputs("CSD:\n"); put_dump(Buff, 0, 16, DW_CHAR); }

        if (disk_ioctl(dev, MMC_GET_CID, Buff) == RES_OK)
            { xputs("CID:\n"); put_dump(Buff, 0, 16, DW_CHAR); }

        if (disk_ioctl(dev, MMC_GET_OCR, Buff) == RES_OK)
            { xputs("OCR:\n"); put_dump(Buff, 0, 4, DW_CHAR); }

        if (disk_ioctl(dev, MMC_GET_SDSTAT, Buff) == RES_OK) {
            xputs("SD Status:\n");
            for (s1 = 0; s1 < 64; s1 += 16) put_dump(Buff+s1, s1, 16, DW_CHAR);
            }
        //
        vPortFree(Buff);
        }
}

void dd(char *s)
{
BYTE drv=cur_vol();
long sect=0;
long n_sect=1;
long ofs;
char *ptr = &s[3];
int res;
int i;
uint8_t *Buff=NULL;

    if(!xatoi(&ptr, &sect)) {
        sect = 0;
        }
    if(!xatoi(&ptr, &n_sect)) {
        n_sect = 1;
        }
    uint32_t s_size=0;
    disk_ioctl(drv, GET_SECTOR_SIZE, &s_size);
    xprintf("dtv: %u Sector size=%d\n", drv, s_size);
    if(s_size != 0) {
        Buff = (uint8_t*)pvPortMalloc(s_size);
        if(Buff) {
            for(i=0; i<n_sect; i++) {
                res = disk_read(drv, Buff, sect, 1);
                if(res) {
                    xprintf("PD#:%u sect:%lu\r\n", drv, sect++);
                    xprintf("rc=%d\r\n", res);
                    }
                else {
                    xprintf("PD#:%u sect:%lu\n", drv, sect++);
                    for(ptr=(char*)Buff, ofs = 0; ofs < s_size; ptr += 32, ofs += 32) {
                        put_dump((BYTE*)ptr, ofs, 32, DW_CHAR);
                        }
                    }
                }
            vPortFree(Buff);
            }
        else {
            xprintf("Error: pvPortMalloc()\n");
            }
        }
    else {
        xprintf("Error: sector size = %d\n", s_size);
        }
}

extern FATFS* FatFs[]; // [3];

void mount(char *s)
{
char str[32];
long l;
char *ptr=&s[6];
int vol=0;
//
    if(xatoi(&ptr, &l)) {
        vol = l;
        }
    else vol=cur_vol();
    //
    if(vol < FF_VOLUMES) {
        xsprintf(str, "%d:", vol);
        int res=f_mount(FatFs[vol], str, 0);
        if(res) put_rc(res);
        }
}

void umount(char *s)
{
char str[32];
long l;
char *ptr=&s[7];
int vol=0;
//
    if(xatoi(&ptr, &l)) {
        vol = l;
        }
    else vol=cur_vol();
    //
    if(vol < FF_VOLUMES) {
        xsprintf(str, "%d:", vol);
        int res=f_mount((void*)0, str, 0);
        if(res) put_rc(res);
        }
}

void di(char* s)
{
long l;
char *ptr=&s[3];
int vol=0;
//
    if(xatoi(&ptr, &l)) {
        vol = l;
        }
    else vol=cur_vol();

    int res = disk_initialize(vol);
    xprintf("disk:%u rc=%d\r\n", vol, res);
}

void dw(char *s)
{
BYTE drv=cur_vol();
DWORD sect=0;
DWORD n_sect=1;
//DWORD ofs;
char *ptr = &s[3];
int res=0;
DWORD i;
DWORD data;
uint8_t *Buff=NULL;

    if(!xatoi(&ptr, (long*)&sect)) {
        sect = 0;
        }
    if(!xatoi(&ptr, (long*)&n_sect)) {
        n_sect = 1;
        }
    if(!xatoi(&ptr, (long*)&data)) {
        data = 0xff;
        }
    //
    uint32_t s_size=0;
    disk_ioctl(drv, GET_SECTOR_SIZE, &s_size);
    if(s_size != 0) {
        Buff = (uint8_t*)pvPortMalloc(s_size);
        if(Buff) {
            memset(Buff, (uint8_t)data, s_size);
            xprintf("sect=%d n_sect=%d data=%d\n", sect, n_sect, data);
            //
            for(i=0; i<n_sect; i++) {
                res = disk_write(drv, Buff, sect++, 1);
                if(res) {
                    xprintf("rc=%d\r\n", res);
                    }
                else xprintf("sect=%4d\r\n", sect-1);
                }
            xprintf("rc=%d\r\n", res);
            //
            vPortFree(Buff);
            }
        else {
            xprintf("Error: pvPortMalloc()\n");
            }
        }
}
