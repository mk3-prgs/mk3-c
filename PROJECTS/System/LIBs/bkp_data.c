#include "bkp_data.h"

//#include <stdint.h>
//#include <stm32f10x_conf.h>
/*
void PWR_BackupAccessCmd(int en)
{
    if(en) pmu_backup_write_enable();
    else pmu_backup_write_disable();
    //
    rtc_lwoff_wait();
}
*/

uint16_t bkp_read_word(uint16_t adr)
{
uint16_t d;
    d = bkp_data_read(adr);
    return(d);
}
//
uint32_t bkp_read_dword(uint16_t adr)
{
uint32_t d;
    d = bkp_data_read(adr+1);
    d <<= 16;
    d |= bkp_data_read(adr);
    return(d);
}

void bkp_write_word(uint16_t adr, uint16_t d)
{
    pmu_backup_write_enable();
    //rtc_lwoff_wait();
    bkp_data_write(adr, d);
    pmu_backup_write_disable();
    //rtc_lwoff_wait();
}

void bkp_write_dword(uint16_t adr, uint32_t d)
{
    pmu_backup_write_enable();
    //rtc_lwoff_wait();
    bkp_data_write(adr,   d);
    bkp_data_write(adr+1, d>>16);
    pmu_backup_write_disable();
    //rtc_lwoff_wait();
}

/*
void bkp_init(void)
{
    if(bkp_read_word(e_init) != 0xaa55) {
        //
        bkp_write_dword(e_tink, 0);
        //
        bkp_write_word(e_init, 0xa55a);
        }
}
*/
