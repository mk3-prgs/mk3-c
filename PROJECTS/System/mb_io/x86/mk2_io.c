#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../io.h"


port_t s_ctx;
port_t* ctx = &s_ctx;

static int nom=0;

int mk2_open(char *dev, int n, int speed)
{
struct serial_rs485 rs485conf;
//
    nom = n;
    ctx->file = -1;
    //ctx->response_timeout;
    //ctx->byte_timeout;
    ctx->debug = 0;
    //
    strcpy(ctx->device, dev);
    ctx->baud = speed;
    ctx->data_bit = 8;
    ctx->stop_bit = 1;
    ctx->parity = 'E';
    //
    if(p_connect(ctx) != 0) {
        printf("Error device %s.\n", dev);
        return(1);
        }
    //
    memset(&rs485conf, 0x0, sizeof(struct serial_rs485));
    ioctl(ctx->file, TIOCGRS485, &rs485conf);
    rs485conf.flags |= SER_RS485_ENABLED; // | SER_RS485_TXB8;
    ioctl(ctx->file, TIOCSRS485, &rs485conf);
    //
    return 0;
}

int mk2_rx(int addr, uint16_t *rx_bf, int len)
{
uint8_t bf[256];
int numb = len;
int i;
int ret;
    //
    len *= 2;
    //
    bf[0] = nom;
    bf[1] = addr;
    bf[2] = addr>>8;
    //
    bf[3] = len;
    bf[4] = -2; // cod
    bf[5] = 0x00;
    bf[6] = 0x00;
    //
    bf[7] = 0x00;
    for(i=0; i<7; i++) bf[7] += bf[i];
//==============  Запрос данных FT232RL  =====================
    p_set_parity(ctx, 'M');
    write(ctx->file, bf, 1);
    //printf("%02x ", bf[0]);
    //usleep(1000);
    p_set_parity(ctx, 'S'); //S
    ret = write(ctx->file, &bf[0], 8);
    /*
    printf("mk2_rx: [ ");
    for(i=0;i<8;i++) {
        printf("%02x ", bf[i]);
        }
    printf(" ]\n");
    */
    //============================================================
    for(i=0; i<256; i++) bf[i]=0;
    //
    i = 0;
    uint8_t ks=0;
    ret = 0;
    int res=0;
    //
    numb = len+1;
    //
    do {
        //
        if(i<5) {i++; usleep(50000);}
        else break;
        //
        res = read(ctx->file, &bf[ret], (numb)-ret);
        if(res>0) i=0;
        ret += res;
        } while(ret < (numb));
    //
    if(ret == numb) {
        //printf("mk2_rx<-: ret=%d numb=%d\n", ret, numb);
        for(i=0; i<ret; i++) {
            //printf("%02x ", bf[i]);
            if(i < (ret-1)) ks += bf[i];
            }
        //printf(" ks=%02x bf=%02x\n", ks , bf[ret-1]);
        //
        if(bf[ret-1] == ks) {
            for(i=0; i<len/2; i++) {
                uint16_t h = bf[2*i+1];
                h <<= 8;
                uint16_t l = bf[2*i];
                rx_bf[i] = l | h;
                }
            }
        ret = len;
        }
    else ret=0;
    //
    ret = !ret;
    return(ret);
}

int mk2_tx(int addr, uint16_t *tx_bf, int len)
{
uint8_t bf[256];
int numb = len;
int i;
int ret;
uint8_t *pt=&bf[5];
int n=5;
    len *= 2;
    //
    bf[0] = nom;
    bf[1] = addr;
    bf[2] = addr>>8;
    //
    bf[3] = len;
    bf[4] = 1; // cod
    //
    uint8_t *ps = (uint8_t*)tx_bf;
    for(i=0; i<len; i++) {
        //bf[5] = 0x00;
        //bf[6] = 0x00;
        *pt++ = *ps++;
        n++;
        }
    //
    *pt = 0x00;
    for(i=0; i < n; i++) *pt += bf[i];
//==============  Запрос данных FT232RL  =====================
    p_set_parity(ctx, 'M');
    write(ctx->file, bf, 1);
    //usleep(1000);
    p_set_parity(ctx, 'S'); //S
    ret = write(ctx->file, &bf[0], n+1);
    /*
    printf("mk2_tx:->\n");
    for(i=0; i<(n+1); i++) {
        printf("%02x ", bf[i]);
        }
    printf("\n");
    */
    //============================================================
    for(i=0; i<len; i++) bf[i]=0;
    //
    i = 0;
    //uint8_t ks=0;
    ret = 0;
    int res=0;
    //
    numb = 1;
    //
    do {
        //
        if(i<5) {i++; usleep(50000);}
        else break;
        //
        res = read(ctx->file, &bf[ret], (numb)-ret);
        if(res>0) i=0;
        ret += res;
        } while(ret < (numb));
    /*
    //if(ret>0) {
        printf("mk2_tx<-: ret=%d numb=%d\n", ret, numb);
        for(i=0; i<ret; i++) {
            printf("%02x ", bf[i]);
            }
        printf("\n");
    //    }
    */
    ret = !ret;
    return(ret);
}

void mk2_close(void)
{
    p_close(ctx);
}

void view_mk2(uint8_t* bf)
{
int32_t x;
uint16_t d;
    // Sost
    d = bf[0]; d <<= 8; // = d;
    d |= bf[1]; // = d>>8;
    printf("Sost=0x%04x\n", d);
    //
    d =  bf[2]; d <<= 8; //= Ttek>>8;
    d |= bf[3]; x = (int)d;   //= Ttek;
    printf("Ttek=%04d\n", x);
    //
    //bf[4] = Tist>>8;
    //bf[5] = Tist;
    d =  bf[4]; d <<= 8;
    d |= bf[5]; x = (int)d;
    printf("Tist=%04d\n", x);
    //
    //bf[6] = Tzad>>8;
    //bf[7] = Tzad;
    d =  bf[6]; d <<= 8;
    d |= bf[7]; x = (int)d;
    printf("Tzad=%04d\n", x);
    //
    //bf[8] = Tmax>>8;
    //bf[9] = Tmax;
    d =  bf[8]; d <<= 8;
    d |= bf[9]; x = (int)d;
    printf("Tmax=%04d\n", x);
    //
    //bf[10] = Thig;
    if(0x80 & bf[10]) x = 0xffffff00 | bf[10];
    else x = bf[10];
    printf("Thig=%d\n", x);
    //
    //bf[11] = Tlov;
    if(0x80 & bf[11]) x = 0xffffff00 | bf[11];
    else x = bf[11];
    printf("Tlov=%d\n", x);
    //
    d =  bf[12]; d <<= 8; //= Ttek>>8;
    d |= bf[13]; x = (int)d;   //= Ttek;
    printf("Vtek=%04d\n", x);
    //
    //bf[4] = Tist>>8;
    //bf[5] = Tist;
    d =  bf[14]; d <<= 8;
    d |= bf[15]; x = (int)d;
    printf("Vist=%04d\n", x);
    //
    //bf[6] = Tzad>>8;
    //bf[7] = Tzad;
    d =  bf[16]; d <<= 8;
    d |= bf[17]; x = (int)d;
    printf("Vzad=%04d\n", x);
    //
    //bf[8] = Tmax>>8;
    //bf[9] = Tmax;
    d =  bf[18]; d <<= 8;
    d |= bf[19]; x = (int)d;
    printf("Vmax=%04d\n", x);
    //
    //bf[10] = Thig;
    if(0x80 & bf[20]) x = 0xffffff00 | bf[20];
    else x = bf[20];
    printf("Vhig=%d\n", x);
    //
    //bf[11] = Tlov;
    if(0x80 & bf[21]) x = 0xffffff00 | bf[21];
    else x = bf[21];
    printf("Vlov=%d\n", x);

    //
    //bf[22] = dTN>>8;
    //bf[23] = dTN;
    d =  bf[22]; d <<= 8;
    d |= bf[23]; x = (int)d;
    printf("dTN=%04d\n", x);
    //
    //bf[24] = dTO>>8;
    //bf[25] = dTO;
    d =  bf[24]; d <<= 8;
    d |= bf[25]; x = (int)d;
    printf("dTO=%04d\n", x);
    //
    //bf[26] = tink>>8;
    //bf[27] = tink;
    d =  bf[26]; d <<= 8;
    d |= bf[27]; x = (int)d;
    printf("tink=%04d\n", x);
    //
    //bf[28] = Nobv>>8;
    //bf[29] = Nobv;
    d =  bf[28]; d <<= 8;
    d |= bf[29]; x = (int)d;
    printf("Nobv=%04d\n", x);
    //
    x = bf[30];
    printf("Nt=%d\n", x);
    //
    x = bf[31];
    printf("Nv=%d\n", x);
    //
    x = bf[32];
    printf("Pnag=%d\n", x);
    //
    x = bf[33];
    printf("Pohl=%d\n", x);
    //
    x = bf[34];
    printf("Puvl=%d\n", x);
    //
    d =  bf[35]; d <<= 8;
    d |= bf[36]; x = (int)d;
    printf("dtOhl=%04d\n", x);
    //
    d =  bf[37]; d <<= 8;
    d |= bf[38]; x = (int)d;
    printf("dtUvl=%04d\n", x);
}

