#include "FreeRTOS.h"
#include "task.h"
#include "ff.h"

#include <string.h>
#include "xprintf.h"

#include "mtc16201/mtc16201.h"
#include "mtc16201/pwm_out.h"

void put_rc (FRESULT rc);

int getT(int bus, int nom);

static char f_name[]="arch.txt";

void test_ds(void* arg)
{
TickType_t l_time;
TickType_t freq = 5000;
FIL file;
FIL* fp=&file;
int t,h;
char str[32];
int res;
uint32_t tt=0;
    //
    PWM_Init();
    lcd_init();
    lcd_print_s("\tLCD\ninited!");
    //
    l_time = xTaskGetTickCount();
    //
    for(;;) {
        t = getT(0, 7);
        h = getT(0, 0);
        //
        xsprintf(str, "\tt=%d\nh=%d", t, h);
        lcd_print_s(str);
        //
        fp=&file;
        res = f_open(fp, f_name, FA_OPEN_APPEND | FA_WRITE);
        //
        if(res) {
            put_rc(res);
            xprintf("f_name: %s Error OPEN_APPEND!\r\n", f_name);
            fp=&file;
            res = f_open(fp, f_name, FA_CREATE_ALWAYS | FA_WRITE);
            if(res == 0) {
                f_close(fp);
                }
            else {
                put_rc(res);
                xprintf("f_name: %s Error CREATE_ALWAYS!\r\n", f_name);
                fp=NULL;
                }
            }
        //
        if(fp) {
            memset(str, 0, 32);
            xsprintf(str, "%d %d %d\r\n", tt, t, h);
            int len = strlen(str);
            UINT cnt=0;
            f_write(fp, str, len, &cnt);
            f_close(fp);
            }
        //
        tt++;
        vTaskDelayUntil(&l_time, freq);
        }
}
