#include <sys/poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "io.h"

#define IOBF_LEN (0x10000)

float therm(void);
extern port_t s_ctx;

uint8_t crc(uint8_t *packet)
{
int i;
uint8_t checksum=0;
    //
    for(i=1;i<8;i++) {
        checksum += packet[i];
        }
    checksum = 0xff - checksum;
    checksum += 1;
    return(checksum);
}

int main(int argc, char *argv[])
{
char filename[256];
char device[256];
    //===================
    //
FILE *fl=NULL;
int i;
int ret=0;
//uint16_t len = 0;
port_t* ctx = &s_ctx;
uint8_t cmd[16];
//
int Conz=0;
uint8_t cs;
//int sConz=0;
//
struct tm *t_tm;
time_t tek_time;
char str[512];
    //
    ctx->s = -1;
    //
    strcpy(device, "/dev/ttyUSB1");
    //strcpy(filename, "stm32.hex");
    for(i=0; i<256; i++) filename[i] = 0;



    for(i=1; i< argc; i++) {
        if(strcmp("-f", argv[i]) == 0) strcpy(filename, argv[i+1]);
        else if(strcmp("-p", argv[i]) == 0) strcpy(device, argv[i+1]);
        }

    ctx->debug = 0;
    //
    strcpy(ctx->device, device);
    ctx->baud = 9600;
    ctx->data_bit = 8;
    ctx->stop_bit = 1;
    ctx->parity = 'N';
    //
    if(p_connect(ctx) != 0) {
        ret = 1;
        printf("Ошибка подключения через %s.\n", device);
        return(0);
        }
    //
    printf("mh-z14a: Подключение через %s.\n", device);
    //
    while(1) {
        //
        //========================================
        //
        memset(cmd, 0, 16);
        cmd[0] = 0xff;
        cmd[1] = 0x01;
        cmd[2] = 0x86;
        cmd[8] = crc(cmd);

        write(ctx->s, cmd, 9);
        //usleep(100000);
        memset(cmd, 0, 16);
        read(ctx->s, cmd, 9);
        Conz = (256 * ((int)cmd[2])) + (int)cmd[3];
        cs = crc(cmd);
        //
        printf("%5dppm <", Conz);
        for(i=0; i<9; i++) printf("%02x ", cmd[i]);
        printf("> 0x%02x\n", cs);
        //
        /*
        tek_time = time(NULL);
        t_tm = gmtime((const time_t*)&tek_time);
        sprintf(str, "%02d:%02d:%02d %02d-%02d-%04d;%d",
               t_tm->tm_hour,
               t_tm->tm_min,
               t_tm->tm_sec,
               t_tm->tm_mday,
               t_tm->tm_mon+1,
               t_tm->tm_year+1900,
               //
               Conz);
               //
        fl=fopen("/home/www/arch/co2.txt", "a");
        if(fl != NULL) {
            fprintf(fl, "%s\n", str);
            fclose(fl);
            fl=NULL;
            }
        printf("%s\n", str);
        */
        usleep(1000000);
        }
    //
    if(ctx->s > 0) p_close(ctx);
	return(ret);
}
