#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <time.h>

#include "accel.h"
#include <math.h>

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"

int file_i2c=-1;

int set_slave(uint8_t addr)
{
int res;
    if (ioctl(file_i2c, I2C_SLAVE, addr) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        res=1;
        }
    else res=0;
    return(res);
}

void delay_ms(int ms)
{
struct timespec tv;
    tv.tv_sec = 0;
    tv.tv_nsec = 1000000*ms;
    //
    nanosleep(&tv, NULL);
}

int font=0;

FontDef* pFont[]=
{
&Font_6x8,
&Font_7x10,
&Font_11x18,
&Font_16x26
};

void set_font(int f)
{
    font=f;
}

void print(char* s)
{
    ssd1306_WriteString(s, *pFont[font], White);
}

void ssd1306_Init(void);
extern uint8_t s_SSD1306_Buffer[SSD1306_BUFFER_SIZE];

uint8_t lcd_addr=0x3c;

int get_hum(void)
{
int fd;
uint8_t bf[32];
int h;
    fd = open("/sys/class/hwmon/hwmon2/humidity1_input", O_RDONLY);
    memset(bf, 0, 32);
    read(fd, bf, 30);
    close(fd);
    //
    sscanf((char*)bf, "%d", &h);
    return(h);
}

int get_temp(void)
{
int fd;
uint8_t bf[32];
int h;
    fd = open("/sys/class/hwmon/hwmon2/temp1_input", O_RDONLY);
    memset(bf, 0, 32);
    read(fd, bf, 30);
    close(fd);
    //
    sscanf((char*)bf, "%d", &h);
    return(h);
}

int main()
{
const char * devName = "/dev/i2c-2";

int X,Y,Z;
int aX,aY,aZ;
int sX,sY,sZ;
char str[64];
int D=10;

int i=0;
time_t tt;
struct tm *lt;
    //
    sX=0;
    sY=0;
    sZ=0;
    aX=0;
    aY=0;
    aZ=0;
    // Open up the I2C bus
    file_i2c = open(devName, O_RDWR);
    if (file_i2c == -1) {
        perror(devName);
        exit(1);
        }
    //
    set_slave(lcd_addr);
    //
    ssd1306_Init();
    ssd1306_Fill(Black);
    memset(s_SSD1306_Buffer, 0x55, sizeof(s_SSD1306_Buffer));
    ssd1306_UpdateScreen();
    //
    //ssd1306_TestAll();
    //
    //ssd1306_TestFPS();
    //ssd1306_TestFonts();
    //ssd1306_TestCircle();
    ///
    i = accelInit(2, 0x53, TYPE_ADXL345);
//	i = accelInit(1, 0x68, TYPE_MPU6050);
//	i = accelInit(1, 0x6a, TYPE_LSM9DS1);
//	i = accelInit(1, 0x69, TYPE_BMI160);
	if(i != 0) {
		exit(1); // problem - quit
        }

    for(;;) {
        //
        accelReadAValues(&X, &Y, &Z);
        sX -= aX;
        sX += X;
        aX = sX / D;
        //
        sY -= aY;
        sY += Y;
        aY = sY / D;
        //
        sZ -= aZ;
        sZ += Z;
        aZ = sZ / D;
        //
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        tt = time(NULL);
        lt = localtime(&tt);
        sprintf(str, " %02d:%02d:%02d %02d-%02d-%04d ",
                        lt->tm_hour,
                        lt->tm_min,
                        lt->tm_sec,
                        lt->tm_mday,
                        lt->tm_mon+1,
                        (lt->tm_year+1900)
                        );
        ssd1306_WriteString(str, Font_6x8, White);
        //
        sprintf(str, "   t=%4.1f~C  v=%4.1f%%", ((double)get_temp())/1000.0, ((double)get_hum())/1000.0);
        ssd1306_SetCursor(0, 8);
        ssd1306_WriteString(str, Font_6x8, White);
        //
        //ssd1306_SetCursor(x, y);
        set_font(2);
        double a = 90 * asin(aX/256.0)/(M_PI_2);
        ssd1306_SetCursor(10, 18);
        sprintf(str, "x=%4.1f~", a);
        print(str);
        //
        a = 90 * asin(aY/256.0)/(M_PI_2);
        ssd1306_SetCursor(10, 18+20);
        sprintf(str, "y=%4.1f~", a);
        print(str);
        /*
        int d = aZ+34;
        a = 90 * asin(d/256.0)/(M_PI_2);
        ssd1306_SetCursor(10, 18+22);
        sprintf(str, "z=%4.1f~ %d", a, d);
        print(str);
        */
        //ssd1306_WriteString(str, Font_7x10, White);
        ssd1306_UpdateScreen();
        }
    //
    return 0;
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte)
{
uint8_t bf[4];
    set_slave(lcd_addr);
    bf[0] = 0x00;
    bf[1] = byte;
    if(write(file_i2c, bf, 2) != 2) {
        perror("ssd1306: Failed to write command to the i2c bus");
        exit(1);
        }
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size)
{
uint8_t bf[2048];
int len;
    set_slave(lcd_addr);
    bf[0] = 0x40;
    memcpy(&bf[1], buffer, buff_size);
    len = buff_size + 1;
    if(write(file_i2c, bf, len) != len) {
        perror("ssd1306: Failed to write data to the i2c bus");
        exit(1);
        }
}

