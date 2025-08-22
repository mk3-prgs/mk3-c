#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "rtc_32f1.h"
#include "diskio.h"
#include "ff.h"

#include <string.h>
#include "xprintf.h"

#include "mb_0.h"
#include "mb_1.h"
//
#include "file_io.h"
//
#include "usart.h"
//
mb_sys mb;
//
void    mb_putc(uint8_t c);
uint8_t mb_getc(void);
//
void flash(char* s) {}
void prg(char *s);
//
//static char cur_path[32];
//static FATFS FatFs[1];
static FIL file;
//extern FATFS FatFs[];

void put_rc(FRESULT rc);

void dbg(char *s)
{
char c;
    for(;;) {
        c = *s++;
        if(c == '\0') break;
        else if(c == '\n') x_putc('\r');
        x_putc(c);
        }
}

void file_io(void* arg)
{
TickType_t l_time;
TickType_t freq = 50;
FIL* fp=&file;
int res;
UINT cnt=0;
FILINFO Finfo;
    //
    mb.state=READY;
    //
    //
    /*
    speed = get_speed(speed_idx);
    //
    if(prot_mk2) {
        xprintf("nom: %d prot: mk2-c speed: %d\n", nomer,  get_speed(speed_idx));
        init_usart2(speed);
        }
    else {
        xprintf("nom: %d prot: modbus speed: %d\n", nomer, get_speed(speed_idx));
        mb_init(nomer, speed);
        }
    */
    //mb0_init(1, 115200);
    mb1_init(1, 115200);

    l_time = xTaskGetTickCount();
    //
    //state_t state=READY;
    //
    void* pf_in=NULL;
    void* pf_out=NULL;
    for(;;) {
        //
        if(mb.state != READY) {
            //
            if(mb.state == SH_IO) {
                //dbg("SH_IO\n");
                pf_out=xdev_out(mb_putc);
                pf_in =xdev_in(mb_getc);
                mb.state = READY;
                }
            else if(mb.state == SH_DONE) {
                xdev_out(pf_out);
                xdev_in(pf_in);
                //dbg("SH_DONE\n");
                mb.state = READY;
                }
            //
            else if(mb.state == F_WRITE) {
                res = f_open(fp, mb.f_name, FA_CREATE_ALWAYS | FA_WRITE);
                //
                if(res) {
                    mb.state=FF_ERROR;
                    put_rc(res);
                    }
                else mb.state=READY;
                //
                freq=5;
                //
                ///xprintf("\r\nF_WRITE: %s len=%d fp=%x state=%d\n", mb.f_name, mb.f_len, fp, state);
                ///LCD_Progress(-1, 30);
                }
            //
            else if(mb.state == F_READ) {
                res = f_stat(mb.f_name, &Finfo);
                if(res) {
                    mb.state=FF_ERROR;
                    }
                else {
                    //
                    mb.f_len = Finfo.fsize;
                    //
                    res = f_open(fp, mb.f_name, FA_OPEN_EXISTING | FA_READ);
                    if(res) {
                        mb.state=FF_ERROR;
                        }
                    else {
                        mb.state=READY;
                        freq=5;
                        }
                    }
                ///xprintf("\r\nF_READ: %s len=%d fp=%x state=%d\n", mb.f_name, mb.f_len, fp, state);
                ///LCD_Progress(-1, 30);
                }
            //
            else if(mb.state == DONE) {
                //
                res = f_close(fp);
                //
                if(res) mb.state=FF_ERROR;
                else mb.state = READY;
                //
                ///xprintf("\r\nDONE: %s res=%d state=%d\n", mb.f_name, res, state);
                ///LCD_Progress(-2, 30);
                //
                //
                freq=50;
                }
            //
            else if(mb.state == SEND) {
                //
                res = f_write(fp, mb.bf, mb.l_bf, &cnt);
                //
                if(res || (cnt != mb.l_bf)) {
                    mb.state=FF_ERROR;
                    freq=50;
                    }
                else mb.state = READY;
                //
                ///xprintf("SEND: id=%d state:%d\n", mb.id, mb.state);
                /*
                int x = (ppp-mb.f_len);
                x *= 10000;
                x /= ppp;
                x /= 100;
                //
                if(x>100) x=100;
                ///LCD_Progress(x, 30);
                //
                ///xprintf("ppp=%d f_len=%d x=%d\n", ppp, mb.f_len, x);
                */
                }
            //
            else if(mb.state == RECV) {
                //
                res = f_read(fp, mb.bf, 128, &cnt);
                mb.l_bf = cnt;
                mb.f_len -= mb.l_bf;
                //
                if(res) mb.state=FF_ERROR;
                else if(cnt == 0) {
                    mb.state = F_EOF;
                    freq=50;
                    }
                else mb.state = READY;
                //
                ///xprintf("RECV: id=%d state:%d\n", mb.id, mb.state);
                /*
                int x = (ppp-mb.f_len);
                x *= 10000;
                x /= ppp;
                x /= 100;
                //
                if(x>100) x=100;
                ///LCD_Progress(x, 30);
                */
                }
            //
            else if(mb.state == RUN) {
                //
                xsprintf((char*)mb.bf,"run prg.bin %s", mb.f_name);
                xsprintf("%s\n", (char*)mb.bf);
                //run((char*)mb.bf);
                //
                l_time = xTaskGetTickCount();
                mb.state = READY;
                }
            //
            else if(mb.state == SET_TIME) {
                //
                uint32_t ut = mb.f_len;
                s_time(ut);
                mb.state = READY;
                //
                }
            //
            else if(mb.state == FLASH) {
                //
                FILINFO Finfo;
                res = f_stat(mb.f_name, &Finfo);
                if(res) {
                    mb.state=FF_ERROR;
                    }
                else {
                    int f_len = Finfo.fsize;
                    if(f_len == mb.f_len) {
                        char str[32];
                        xsprintf(str, "flash %s", mb.f_name);
                        mb.state = READY;
                        //
                        flash(str);
                        //
                        }
                    else mb.state=FF_ERROR;
                    }
                }
            }
        //
        vTaskDelayUntil(&l_time, freq);
        }
}

static int mb_outptr=0;
static int mb_inptr=0;
static uint8_t *p_out = &mb.bf[0];
static uint8_t *p_in  = &mb.bf[64];

void x_out_done(void)
{
    mb_outptr=0;
    mb.id = SH_OUT_DONE;
}

void mb_putc(uint8_t c)
{
    while(mb.id == SH_OUT_DONE) {
        vTaskDelay(10);
        }
    //
    p_out[mb_outptr] = c;
    p_out[mb_outptr+1] = '\0';
    mb.l_bf++;
    //
    if(mb_outptr < 60) mb_outptr++;
    else {
        mb_outptr = 0;
        mb.id = SH_OUT_DONE;
        }
    //
    if((c == '\n')||(c=='\0')) {
        mb_outptr=0;
        mb.id = SH_OUT_DONE;
        }
}

uint8_t mb_getc(void)
{
uint8_t c;
    if(mb.state == SH_IN_READY) {
        //dbg("SH_IN_READY\n");
        c = p_in[mb_inptr];
        if(mb_inptr<62) mb_inptr++;
        else {
            c = 0;
            mb_inptr=0;
            mb.state = READY;
            }
        if((c == '\n')||(c == '\r')||(c=='\0')) {
            mb_inptr=0;
            mb.state = READY;
            }
        }
    else {
        c=0;
        mb_inptr=0;
        //mb.state = READY;
        }
    //
    return(c);
}
