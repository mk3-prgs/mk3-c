#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define PID ("/var/run/boiler")


static uint8_t mode_inp = SPI_MODE_1 | SPI_CS_HIGH;;
static uint8_t mode_out = SPI_MODE_1 | SPI_CS_HIGH;
static uint8_t bits = 8;
static uint32_t speed = 1000000;

static int fd_inp;
static int fd_out;
//static uint8_t out_buf[8] = {0};
//static uint8_t inp_buf[8] = {0};

union {
    uint64_t kl_d;
    uint8_t buf[8];
} out;

union {
    uint32_t in_d;
    uint8_t buf[4];
} inp;

int inp_init(void)
{
int ret = 0;
    //
    fd_inp = open("/dev/spidev3.0", O_RDWR);
    if (fd_inp < 0) return(-1);

    // spi mode
    ret = ioctl(fd_inp, SPI_IOC_WR_MODE, &mode_inp);
    if (ret == -1) {ret=-2; goto end_0; }

    ret = ioctl(fd_inp, SPI_IOC_RD_MODE, &mode_inp);
    if (ret == -1) {ret=-3; goto end_0; }

    // bits per word
    ret = ioctl(fd_inp, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {ret=-4; goto end_0; }

    ret = ioctl(fd_inp, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {ret=-5; goto end_0; }

    // max speed hz
    ret = ioctl(fd_inp, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {ret=-6; goto end_0; }

    ret = ioctl(fd_inp, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {ret=-7; goto end_0; }
    //
    printf("spi mode: %d\n", mode_inp);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    //
    return(0);
    //
 end_0:;
    close(fd_inp);
    return(ret);
}

int out_init(void)
{
int ret = 0;
    //
    fd_out = open("/dev/spidev3.0", O_RDWR);
    if (fd_out < 0) return(-1);

    // spi mode
    ret = ioctl(fd_out, SPI_IOC_WR_MODE, &mode_out);
    if (ret == -1) {ret=-2; goto end_0; }

    ret = ioctl(fd_out, SPI_IOC_RD_MODE, &mode_out);
    if (ret == -1) {ret=-3; goto end_0; }

    // bits per word
    ret = ioctl(fd_out, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {ret=-4; goto end_0; }

    ret = ioctl(fd_out, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {ret=-5; goto end_0; }

    // max speed hz
    ret = ioctl(fd_out, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {ret=-6; goto end_0; }

    ret = ioctl(fd_out, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {ret=-7; goto end_0; }
    //
    return(0);
//    printf("spi mode: %d\n", mode);
//    printf("bits per word: %d\n", bits);
//    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
 end_0:;
    close(fd_out);
    return(ret);
}

void inp_close(void)
{
    close(fd_inp);
}

void out_close(void)
{
    close(fd_out);
}
//
uint8_t Bit_Reverse(uint8_t x)
{
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}
//
void io_flush(void)
{
int i;
    for(i=0;i<6;i++) out.buf[i] = Bit_Reverse(out.buf[i]);
    for(i=3;i<6;i++) out.buf[i] = ~out.buf[i];
    //led_buf[5]++;
    write(fd_out, out.buf, 6);
    //read( fd_inp, inp.buf, 4);
    //inp.in_d |= 0x01010101;
}

/*
void demon(void *arg)
{
int fd;
char str[64];
//RTIME now, previous;
int n=0;
int d;
int st=0;
    //
    // Arguments: &task (NULL=self),
    //           start time,
    //            period (here: 1 s)
    //
    rt_task_set_periodic(NULL, TM_NOW, 300 * 1000 * 1000);
    //previous = rt_timer_read();
    //
    while (1) {
        rt_task_wait_period(NULL);
        //now = rt_timer_read();
        //previous = now;
        //
        d = (n++)%(16);
        //
        if((n%8)==0) n++;
        //
        out.kl_d = (0x0000000000000001l << d);
        out.kl_d |= ((uint64_t)(~inp.in_d)) << 16;
        out.kl_d ^= 0xffffffffff000000l;
        //
        //fd = open("/dev/ttyS0", O_RDWR);
        //sprintf(str, "%08x\n", (uint32_t)(~inp.in_d));
        //write(fd, str, strlen(str));
        //close(fd);
        //r = (0x80>>d);
        //
        adc[0] = 0x1234;
        fd = open("/dev/at91adc0", O_RDONLY);
        n = read(fd, adc, 8192);
        close(fd);
        //
        fd = open("/dev/mtc", O_WRONLY);
        //sprintf(str, "\rInp=%08x %02d\nOut=%08x", inp.in_d, (n++)%256, (uint32_t)out.kl_d);
        sprintf(str, "\tADC=%04x %03d\nst=%d", adc[0], n, st++);
        write(fd, str, strlen(str));
        close(fd);
        //
        io_flush();
        }
}
*/
