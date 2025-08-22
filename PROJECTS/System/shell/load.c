#include "shell.h"

int load(char* f_name)
{
int vol=0;
FIL fp;
int res;
uint8_t* pbf;
uint32_t cnt;
void *p = (void*)0x2000c000;
char name[32];
    //
    vol = cur_vol();
    xsprintf(name, "%d:/%s", vol, f_name);
    //xprintf("path: %s\n", name);
    //
    res = f_open(&fp, name, FA_READ);
    if(res) {
        //xprintf("Файл %s не найден!\n", name);
        return(1);
        }
    //
    uint32_t s_size=0;
    disk_ioctl(vol, GET_SECTOR_SIZE, &s_size);

    pbf = pvPortMalloc(s_size);
    if(pbf == NULL) {
        xprintf("load(); Мало памяти!\n");
        f_close(&fp);
        return(1);
        }
    //
    for(;;) {
        res = f_read(&fp, pbf, s_size, (UINT*)&cnt);
        if(res) {
            xprintf("Ошибка чтения файла %s!\n", name);
            res=1;
            break;
            }
        else {
            if(cnt>0) {
                memcpy(p, pbf, cnt);
                p += cnt;
                //st+=cnt;
                }
            else {
                res=0;
                break;
                }
            }
        }
    //
    vPortFree(pbf);
    f_close(&fp);
    //
    return(res);
}

