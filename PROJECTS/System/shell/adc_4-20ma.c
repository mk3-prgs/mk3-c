#include "shell.h"

uint32_t adc0, adc1;

void adc(char *s)
{
int res=0;
FIL file;
int adc_4 = 0;
int adc_20= 4095;
float k=3.3;
char* p;
char str[32];
UINT cnt;
long l;
//
    p=&s[4];
    if(xatoi(&p, &l)) {
        adc_4 = l;
        if(xatoi(&p, &l)) {
            adc_20 = l;
            if(xatof(&p, &k)) {
                res = 1;
                }
            }
        }
    if(!res) {
        xprintf("adc0=%d adc1=%d\n", adc0, adc1);
        xprintf("adc 4: 20: 3.3:\n");
        return;
        }
    //
    res = f_open(&file, "adc.dat", FA_CREATE_ALWAYS | FA_WRITE);
    if(res) {
        put_rc(res);
        return;
        }
    int d;
    d = (int)(10*k);
    xsprintf(str, "%d %d %d.%d\n", adc_4, adc_20, d/10, d%10);
    res = f_write(&file, str, strlen(str),  &cnt);
    f_close(&file);
}

