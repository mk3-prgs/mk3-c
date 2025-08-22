#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"

#include <gd32f10x.h>

#include "xprintf.h"
#include "find_ds.h"
#include "bit_field.h"
#include "i2c_bus.h"

#include "sht3x.h"
#include "ds1820.h"
#include "sens.h"

//#include "spi_io.h"
int getT(int nom); //ds1624.c

void term(char *s)
{
int t0;
//int i;
    //
    //for(i=0; i<10; i++) {
        t0 = getT(7);
        //t1 = getT(1, 7);
        xprintf("%2d.%02d°C\n", t0/100, t0%100);
        //vTaskDelay(1500);
        //}
}

static sht3x_sensor_t* sensor = NULL;    // sensor device data structure
static int is_inited=0;

void sht_init(char *s)
{
    if(is_inited==0) {
        I2C_LowLevel_Init(0);
        is_inited=1;
        }
    //
    if(sensor == NULL) sensor = sht3x_init_sensor(0, SHT3x_ADDR_2);
    /*
    if(sensor != NULL) {
        //sht3x_start_measurement(sensor, sht3x_periodic_1mps, sht3x_high);
        sht3x_start_measurement(sensor, sht3x_single_shot, sht3x_high);
        vTaskDelay(sht3x_get_measurement_duration(sht3x_high));
        }
    */
}

void spi(char *s);
void spi_ini(char *s);
int getT(int nom);

void sht(char *s)
{
//sht3x_raw_data_t raw_data;
float temp=0.0;
float hum=0.0;
int t,h;
char *pt=&s[4];
int n=1;
long d;
int t0;
int t1;
char c;
    //
    if(xatoi(&pt, &d)) {
            n = d;
            }
    //
    spi_ini("");
    sht_init("");
    ow_init();
    //
    for(;n>0;n--) {
        //
        if(sensor != NULL) {
            //
            t0 = getT(7)+125;
            t1  = rTemp();
            if(t1<0) { c='-'; t1=-t1; }
            else c=' ';
            vTaskDelay(1);
            sTemp();
            if(sht3x_get_results(sensor, &temp, &hum)) {
                t = (int)(100*temp);
                h = (int)(100*hum);
                xprintf("%2d.%02d°C %c%2d.%02d°C %2d.%02d°C %2d.%02d%%  dt=%d\n", t0/100, t0%100, c, t1/100, t1%100, t/100, t%100, h/100, h%100, t-t0);
                }
            else {
                xprintf("%2d.%02d°C %c%2d.%02d°C\n", t0/100, t0%100, c, t1/100, t1%100);
                }
            //
            sht3x_start_measurement(sensor, sht3x_single_shot, sht3x_high);
            }
        //
        vTaskDelay(1000);
        }
}

void ttt(char *s)
{
char *pt=&s[4];
int n=1;
long d;
int t0;
int t1;
char a;
char b;
    //
    if(xatoi(&pt, &d)) {
            n = d;
            }
    ow_init();
    //
    for(;n>0;n--) {
        //
        t0 = getT(7);
        if(t0<0) { a='-'; t0=-t0; }
        else a=' ';
        //
        t1  = rTemp();
        if(t1<0) { b='-'; t1=-t1; }
        else b=' ';
        //
        vTaskDelay(1);
        sTemp();
        xprintf("%c%2d.%02d°C %c%2d.%02d°C\n", a, t0/100, t0%100, b, t1/100, t1%100);
        vTaskDelay(1000);
        }
}
