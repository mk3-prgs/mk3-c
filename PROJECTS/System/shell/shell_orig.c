#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"
//
#include <string.h>
#include "ff.h"
#include "rtc_32f1.h"
//
#include "xprintf.h"
#include "usart.h"
#include "diskio.h"
//
#include "MX25L.h"
//
#include "mtc16201/mtc16201.h"
#include "mtc16201/pwm_out.h"
#include "mtc16201/pwm3_out.h"
//
#include "spi_io.h"

/*
#include "font.h"
extern font_t font_L;
extern font_t font_M;
extern font_t font_H;
#include "mt12864b.h"
*/
//#define USER_TEXT __attribute__((section(".b1text")))
//
/*---------------------------------------------------------------------------*/

void PWM_Init(void);

void init_spi(char* str)
{
    spi_io_Init();
}

void lcd(char* str)
{
    //PWM_Init();
    //lcd_init();
    //lcd_print_s("\tLCD mk3-c\ninited!");
    /*
    LCD_Init();
    set_color(0);
    LCD_Clear();
    set_color(1);
    //
    set_font(&font_L);
    set_font(&font_M);
    set_font(&font_H);
    g_marker(1,1);
    LCD_PutString("Привет!\n");

    int i;
    for(i=0; i<1024; i++) {
        lcd_putchar(i);
        vTaskDelay(100);
        }
    */
}

void spi(char *s)
{
char *ptr = &s[3];
long d;
int i;
    if(!xatoi(&ptr, &d)) {
        d=0;
        }
    //
    d &= 0xff;
    uint8_t b = d;
    //
    for(i=0; i<200; i++) {
        LC_IO=0;
        xprintf("out=0x%02x ", b);
        LC_IO=1;
        //
        CS_IO=0;
        b=spi_io_RW(b);
        CS_IO=1;
        //
        b &= 0x1f;
        xprintf("inp=0x%02x freq=%d cntrl=%d\r\n", b, FREQ, CNTRL);
        //
        vTaskDelay(500);
        }
}

void sat(char *s)
{
char *ptr = &s[4];
long d;

    if(!xatoi(&ptr, &d)) {
        d=0;
        }
    if(d<0) d=0;
    else if(d>10000) d=10000;
    //
    xprintf("sat=%d\r\n", d);
    //set_sat(d);
}

void led(char *s)
{
char *ptr = &s[4];
long d;

    if(!xatoi(&ptr, &d)) {
        d=0;
        }
    if(d<0) d=0;
    else if(d>10000) d=10000;
    //
    xprintf("led=%d\r\n", d);
    ///set_led(d);
}

void sol(char *s)
{
char *ptr = &s[4];
long d;
static uint16_t t=20;
static uint16_t p=400;

    if(xatoi(&ptr, &d)) {
        t = d;
        if(xatoi(&ptr, &d)) {
            p = d;
            }
        }
    //
    xprintf("sol: t=%d p=%d\n", t, p);
    //set_p_sol(p);
    //set_sol(t);
}

char* m_args[16];

int    m_argc;
char** m_argv;

void run(char* str)
{
xTaskHandle th=NULL;

uint32_t *ps= (void*)0x2000c000;
//void (*p)(int n, char **s);
void (*p)(void* arg);
int i;
    //
    m_argv = m_args;
    //
    for(i=0; i<16; i++) m_argv[i]=NULL;
    //
    char* pt = &str[3]; // ==3 !!!!!!!!!
    i=0;
    for(;;) {
        if(*pt == 0) break;
        else if(*pt == ' ') {
            *pt = '\0';
            pt++;
            if(*pt) {
                m_argv[i] = pt;
                if(i<7) i++;
                else break;
                }
            else break;
            }
        else pt++;
        }
    m_argc = i;
    //
    for(i=0; i<m_argc; i++) {
        xprintf("m_argv[%d]: %s\n", i, m_argv[i]);
        }
    //
    if(m_argc > 0) {
        if(load(m_argv[0])==0) {
            p = (void*)*(ps);
            ///xprintf("p=0x%08x\n", p);
            ///p(argc, argv);
            xTaskCreate( p, m_argv[0], configMINIMAL_STACK_SIZE*8, NULL, 2, &th);
            /*
            do {
                th = xTaskGetHandle("prg");
                vTaskDelay(100);
                } while(th);
            */
            xprintf("\n");
            }
        }
    else {
        p = (void*)*(ps);
        xTaskCreate( p, m_argv[0], configMINIMAL_STACK_SIZE*8, NULL, 2, &th);
        xprintf("\n");
        //xprintf("Error run()!\n");
        }
}

void md(char *s)
{
int32_t cnt=0x400;
char *ptr=(char*)0x2000c000;
int ofs=0x2000c000;

    for(; cnt > 0; ptr += 16, ofs += 16) {
        int l = 16;
        if(cnt<16) l=cnt;
        put_dump((BYTE*)ptr, ofs, l, DW_CHAR);
        cnt -= 16;
        }
}



void io(char *s)
{
//int wr = 1;
uint32_t addr;
uint32_t n;
    //
    char *ptr = s+3;
    char *sptr = s+3;
    //
    if(!xatoi(&ptr, (long*)&addr)) {
        addr = 0x0000;
        }
    //
    if(!xatoi(&ptr, (long*)&n)) {
        n = 1;
        }
    //
    if(ptr == sptr) {
        if(!strncmp(sptr, "rtc", 3)) { addr =  0x2800;  n = 10;}
        else if(!strncmp(sptr, "pwr", 3)) { addr =  0x7000;  n = 2;}
        else if(!strncmp(sptr, "rcc", 3)) { addr = 0x2100c; n = 8;}
        else if(!strncmp(sptr, "bkp", 3)) { addr =  0x6c2c; n = 3;}
        }
    //
    addr += 0x40000000;
    //
    xprintf("\n");
    do {
        xprintf("%08x: %08x\n", addr,  *(__IO uint32_t *)addr);
        addr += 4;
        } while(--n);
}

#define BKP_OFFSET (BKP_BASE - PERIPH_BASE)
#define CR_OFFSET  (BKP_OFFSET + 0x30)
#define DBP_BitNumber            0x08
#define CR_DBP_BB  (PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4))

void io_w(char *s)
{
int wr = 1;
uint32_t addr;
uint32_t data;
    //
    char *ptr = s+5;
    //
    if(!xatoi(&ptr, (long*)&addr)) {
        addr = 0x0000;
        }
    //
    if(!xatoi(&ptr, (long*)&data)) {
        data = 0;
        wr=0;
        }
    //
    addr += 0x40000000;
    //
    *(__IO uint32_t *) CR_DBP_BB = 1;
    //
    if(wr) *(__IO uint32_t *)addr = data;
    //
    *(__IO uint32_t *) CR_DBP_BB = 0;
    //
    xprintf("\n");
    xprintf("%08x: %08x\n", addr,  *(__IO uint32_t *)addr);
}

#include "bitbanding.h"
#define ALARM BIT_BAND_PER(GPIOA+12,GPIO_PIN_8)

void rst(char *s)
{
uint32_t data;
    //
    char *ptr = s+3;
    //
    if(*ptr) {
        if(!xatoi(&ptr, (long*)&data)) {
            data = 1;
            }
        }
    else data=1;
    //
    if(data) ALARM=1;
    else ALARM=0;
    //
}

#include "sdcard.h"


void card_info_get(void);

void sdi(char *s)
{
    s=s;
    card_info_get();
}

void sdr(char *s)
{
sd_error_enum sd_error;
long t;
char *p;
uint32_t beg=0;
uint32_t a=0;
uint8_t numb=1;
int n;
int i;
//
    p=&s[4];
    if(xatoi(&p, &t)) {
        beg = 512*t;
        if(xatoi(&p, &t)) {
            numb = (uint8_t)t;
            }
        }

    uint32_t* sd_buf = (uint32_t*)pvPortMalloc(512);
    if(sd_buf) {
        for(n=0; n<numb; n++) {
            a = 512*n + beg;
            sd_error = sd_block_read(sd_buf, a, 512);

            if(SD_OK != sd_error) {
                xprintf("Block %u read fail!\n", a/512);
                }
            else {
                uint8_t *pdata = (uint8_t *)sd_buf;
                xprintf("n=%d\n", n);
                for(i = 0; i < 512; i++) {
                    if((i%32) == 0) xprintf("\n%08x: ", a+i);
                    xprintf("%02x ", *pdata++);
                    }
                xprintf("\n");
                }
            }
        vPortFree(sd_buf);
        }
    xprintf("\n");
}

void sdw(char *s)
{
sd_error_enum sd_error;
long t;
char *p;
uint32_t a=0;
uint32_t beg=0;
uint8_t data=0x00;
int numb=1;
int i;
//
    p=&s[4];
    if(xatoi(&p, &t)) {
        beg = 512*t;
        if(xatoi(&p, &t)) {
            data = (uint8_t)t;
            if(xatoi(&p, &t)) {
                numb = (uint32_t)t;
                }
            }
        }

    //xprintf("beg=%u data=%02x numb=%u\n", beg, data, numb);

    uint32_t* sd_buf = (uint32_t*)pvPortMalloc(512);
    if(sd_buf) {
        memset((uint8_t*)sd_buf, data, 512);
        for(i=0; i<numb; i++) {
            a = beg + 512*i;
            sd_error = sd_block_write(sd_buf, a, 512);
            if(SD_OK != sd_error){
                xprintf("Block %u write fail!\n", a/512);
                }
            else{
                //xprintf("%d [%08x] write Ok!\n", i, a);
                }
            }
        vPortFree(sd_buf);
        }
}

void sdl(char *s)
{
sd_error_enum sd_error;
long t;
char *p;
uint8_t lock = SD_UNLOCK;
    //
    p=&s[4];
    if(xatoi(&p, &t)) {
        if(t != 0) lock = SD_LOCK;
        }
    //
    sd_error = sd_lock_unlock(lock);
    if(SD_OK != sd_error){
        xprintf("Unlock failed!\n");
        }
    else{
        if(lock == SD_UNLOCK) xprintf("The card is unlocked!\n");
        else xprintf("The card is locked!\n");
        }
}

void sde(char *s)
{
sd_error_enum sd_error;
long t;
char *p;
uint32_t beg=1;
uint32_t end=2;
    //
    p=&s[4];
    if(xatoi(&p, &t)) {
        beg = t;
        if(xatoi(&p, &t)) {
            end = t;
            }
        }
    if(beg < end) end=beg;
    //
    sd_error = sd_erase(512*beg, 512*end);
    if(SD_OK != sd_error){
        xprintf("Erase failed!\n");
        }
    else{
        xprintf("Erase %08x - %08x success!\n", 512*beg, 512*end + 511);
        }
}

extern sd_card_info_struct sd_cardinfo;

void card_info_get(void)
{
//
uint8_t sd_spec, sd_spec3, sd_spec4, sd_security;
uint32_t block_count, block_size;
uint16_t temp_ccc;

    sd_spec = (sd_scr[1] & 0x0F000000) >> 24;
    sd_spec3 = (sd_scr[1] & 0x00008000) >> 15;
    sd_spec4 = (sd_scr[1] & 0x00000400) >> 10;
    //
    xprintf("Version:\n");
    if(2 == sd_spec) {
        if(1 == sd_spec3) {
            if(1 == sd_spec4) {
                xprintf("   ## Card version 4.xx ##\n");
                }
            else {
                xprintf("   ## Card version 3.0x ##\n");
                }
            }
        else {
            xprintf("   ## Card version 2.00 ##\n");
            }
        }
    else if(1 == sd_spec) {
        xprintf("   ## Card version 1.10 ##\n");
        }
    else if(0 == sd_spec) {
        xprintf("   ## Card version 1.0x ##\n");
        }
    xprintf("sd_security:\n");
    sd_security = (sd_scr[1] & 0x00700000) >> 20;
    if(2 == sd_security) {
        xprintf("   ## SDSC card ##\n");
        }
    else if(3 == sd_security) {
        xprintf("   ## SDHC card ##\n");
        }
    else if(4 == sd_security) {
        xprintf("   ## SDXC card ##\n");
        }

    block_count = (sd_cardinfo.card_csd.c_size + 1)*1024;
    block_size = 512;
    xprintf("## Device size is %dKB ##\n", sd_card_capacity_get());
    xprintf("## Block size is %dB ##\n", block_size);
    xprintf("## Block count is %d ##\n", block_count);

    xprintf("Partial:\n");
    if(sd_cardinfo.card_csd.read_bl_partial) {
        xprintf("   ## Partial blocks for read allowed ##\n" );
        }
    if(sd_cardinfo.card_csd.write_bl_partial) {
        xprintf("   ## Partial blocks for write allowed ##\n" );
        }

    xprintf("\n");
    temp_ccc = sd_cardinfo.card_csd.ccc;
    xprintf("## CardCommandClasses is: %x ##\n", temp_ccc);
    if((SD_CCC_BLOCK_READ & temp_ccc) && (SD_CCC_BLOCK_WRITE & temp_ccc)){
        xprintf("   ## Block operation supported ##\n");
        }
    if(SD_CCC_ERASE & temp_ccc){
        xprintf("   ## Erase supported ##\n");
        }
    if(SD_CCC_WRITE_PROTECTION & temp_ccc){
        xprintf("   ## Write protection supported ##\n");
        }
    if(SD_CCC_LOCK_CARD & temp_ccc) {
        xprintf("   ## Lock unlock supported ##\n");
        }
    if(SD_CCC_APPLICATION_SPECIFIC & temp_ccc){
        xprintf("   ## Application specific supported ##\n");
        }
    if(SD_CCC_IO_MODE & temp_ccc){
        xprintf("   ## I/O mode supported ##\n");
        }
    if(SD_CCC_SWITCH & temp_ccc){
        xprintf("   ## Switch function supported ##\n");
        }
    //
}

void sqe(char *s)
{
//long t;
//char *p;
//uint32_t sector=0;
    s=s;
    //
    //p=&s[4];
    //if(xatoi(&p, &t)) {
    //    sector = t;
    //    }
    //
    //SectorErase(sector);
    ChipErase();
    //xprintf("Erase %08x\n", sector);
    xprintf("Erase All!\n");
}

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

