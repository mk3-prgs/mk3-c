#include <FreeRTOS.h>
#include "task.h"
#include "xprintf.h"

#include "i2c_bus.h"

#define illegalT -5000

static I2C_TypeDef* i2c;
static int errds;

int getT(int bus, int nom)
{
int t = illegalT;
int ret;
uint8_t ds_bf[8];
//int bus=0;
    //
    //bus = nom / 10;
    nom = nom % 10;
    //
    if(bus > 0) i2c = I2C_2;
    else        i2c = I2C_1;
    //
    ds_bf[0] = 0xaa;
    ret = I2C_Master_BufferWrite(i2c, ds_bf, 1, Polling, 0x48+nom);
    if(ret == Error) {
        I2C_LowLevel_Init(bus);
        t=illegalT;
        goto start_izm;
        }
    //
    ret = I2C_Master_BufferRead(i2c, ds_bf, 2, Polling, 0x48+nom);
    if(ret == Error) {
        I2C_LowLevel_Init(bus);
        t=illegalT;
        }
    else {
        t = (((int)ds_bf[0])<<8) + (int)ds_bf[1];
        t >>= 3;
        //
        t *= 3125;
        t /= 100;
        t += 5;
        t /= 10;
        //
        }
start_izm:;
    //
    ds_bf[0] = 0xee;
    ret = I2C_Master_BufferWrite(i2c, ds_bf, 1, Polling, 0x48+nom);
    if(ret == Error) {
        I2C_LowLevel_Init(bus);
        }
    //
    if(t==illegalT) errds=1;
    else errds=0;
    //
    return(t);
}
