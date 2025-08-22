#include "shell.h"
#include "main.h"
#include "mtc16201/pwm3_out.h"
//
//#define USER_TEXT __attribute__((section(".b1text")))
//
/*---------------------------------------------------------------------------*/

void PWM_Init(void);

void beep(char *s)
{
int i;
int n=100;
int w=800;
int m=1;
long t;
char *ptr=&s[5];
    if(xatoi(&ptr, &t)) {
        w = t;
        if(xatoi(&ptr, &t)) {
            n = t;
            if(xatoi(&ptr, &t)) {
                m = t;
                }
            }
        }
    //
    int k;
    for(k=0; k<m; k++) {
        ///xprintf("%d beep: %d %d\n", k, w, n);
        //
        gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
        //
        //int e = w+200;
        //for(; w<e; w+=10) {
        //    xprintf("w=%d\n", w);
            for(i=0; i<n; i++) {
                BEEP=0;
                usleep(w);
                BEEP=1;
                usleep(w);
                }
        w += 10;
        n -= 5;
        }
    BEEP=0;

    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ,  GPIO_PIN_0);
}

void spi_ini(char* str)
{
    spi_io_Init();
    //
    // POWER (SOL)
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    // PC6 PC7
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
    //
    // CNTPL
    gpio_init(GPIOA, GPIO_MODE_IPD, 0, GPIO_PIN_8);
}

extern int m_sat;
extern int m_led;

int lcd_inited=0;

void lcd(char* s)
{
//char* ptr=&s[4];

    if(lcd_inited == 0) {
        PWM_Init();
        //lcd_init();
        set_sat(m_sat);
        set_led(m_led);
        //lcd_print_s("\t");
        lcd_inited=1;
        }
    //lcd_print_s("\t");
    //lcd_print_s(ptr);
}

typedef struct __h_sim__
{
    char c;
    uint8_t h;
} h_sim;

h_sim t_sim[]=
{
    {'0', 0xdd}, //0
    {'1', 0x90}, //1
    {'2', 0xc7}, //2
    {'3', 0xd3}, //3
    {'4', 0x9a}, //4
    {'5', 0x5b}, //5
    {'6', 0x5f}, //6
    {'7', 0xd0}, //7
    {'8', 0xdf}, //8
    {'9', 0xdb}, //9
    {'a', 0xde}, //a
    {'b', 0x1f}, //b
    {'c', 0x07}, //c
    {'d', 0x97}, //d
    {'e', 0x4f}, //e
    {'f', 0x4e}  //f
};

uint8_t sim(char c)
{
int i;
    for(i=0;i<16;i++) {
        if(c == t_sim[i].c) return(~t_sim[i].h);
        }
    return 0xff;
}

uint32_t rbit(uint32_t val)
{
uint32_t output;
#if defined(__arm__) || defined (__aarch64__)
   asm("rbit %0,%1" : "=r"(output) : "r"(val));
#else
    output = val;
#endif
   return(output);
}

#define PC6  GET_BIT_BB(GPIOC+12,6)
#define PC7  GET_BIT_BB(GPIOC+12,7)

static int spi_inited=0;

void spi(char *s)
{
uint8_t bf[4];
char *ptr = &s[4];
long d;
uint32_t data=0;
uint32_t test;
uint32_t s_test;
uint32_t tmp=0;
int n;
int k;
int i;
    //
    //SOL = 1;
    if(spi_inited==0) {
        spi_ini("spi_ini");
        spi_inited=1;
        }
    //
    //
    if(*ptr == 't') {
        for(k=0;k<10;k++) {

            test=1;
            //
            for(n=0;n<32;n++) {
                data = test;
                bf[0] = data>>0;
                bf[1] = data>>8;
                bf[2] = data>>16;
                bf[3] = data>>24;
                //spi_dio(bf, 4);
                //
                test <<= 1;
                //
                xprintf("data=%08x %02x%02x%02x%02x\n", data, bf[3], bf[2], bf[1], bf[0]);
                vTaskDelay(100);
                }
            }
        return;
        }
    //
    s_test = 0;
    data = 0;
    //
    if(*ptr == 'd') {
        for(k=0;k<(32*100);k++) {

            if(data != 0) data <<= 1;
            else data = 1;
            //
            tmp = data;
            //xprintf("data=%08x tmp=%08x\n", data, tmp);
            //
            bf[0] = tmp;
            bf[1] = tmp>>8;
            bf[2] = tmp>>16;
            bf[3] = tmp>>24;
            //
            ///spi_dio(bf, 4);
            //
            test  = bf[1]; test <<= 8;
            test |= bf[2]; test <<= 8;
            test |= bf[3]; test <<= 8;
            test += bf[0];
            //
            //xprintf("data=%08x %08x\n", data, test);
            /*
            n=0;
            test >>= 8;
            for(i=0;i<24;i++) {
                if(0x01 & test) {
                    xprintf("i=%d\n", i+8);
                    }
                test >>= 1;
                }
            */
            if(test != s_test) {
                //if((0x01 & test)==0) {
                    tmp = data;
                    for(i=0; i<32; i++) {
                        if(0x01 & (tmp>>i)) {
                            xprintf("O=%d %08x\n", i, data);
                            }
                        }
                //    }
                s_test = test;
                }
            vTaskDelay(25);
            }
        return;
        }
    //
    data = 0;
    if(xatoi(&ptr, &d)) {
          data = d;
          }
    tmp = rbit(data);

    bf[0] = tmp>>0;
    bf[1] = tmp>>8;
    bf[2] = tmp>>16;
    bf[3] = tmp>>24;
    //
    //CS_IO=0;
    ///spi_dio(bf, 4);
    //
    test  = bf[1]; test <<= 8;
    test |= bf[2]; test <<= 8;
    test |= bf[3]; test <<= 8;
    test += bf[0];

    xprintf("data=%08x %08x %02x %02x %02x %02x\n", data, test, bf[3], bf[2], bf[1], bf[0]);
    //
    //SOL = 0;
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
    set_sat(d);
    m_sat = d;
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
    set_led(d);
    m_led = d;
}

void sol_init(char *s)
{
    PWM3_Init();
}

void sol(char *s)
{
char *ptr = &s[4];
long l=0;
uint16_t d=0;

    if(xatoi(&ptr, &l)) {
        d = l;
        }
    xprintf("sol: %d\n", d);
    //PWR_IO = (d != 0);
    set_sol(d);
    //
}

void io_n(char *s)
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

void viev_data(int n);
int restore_data(int n);
int save_data(int n);
void edit_data(int n, int data);
void init_data(void);
int length_data(void);

void rst(char *s)
{
/*
long l;
uint32_t data;
    //
    data = 1;
    char *ptr = s+3;
    //
    if(*ptr) {
        if(!xatoi(&ptr, &l)) {
            data = l;
            }
        }
*/
    //if(data) ALARM=1;
    //else ALARM=0;
    //
}

void prf(char* s)
{
long t;
int op=0;
char *ptr=NULL;
int n,data;
//
    if(strlen(s) > 4) {
        ptr = &s[4];
        if(!strcmp(ptr, "init")) {
            op=3;
            }
        if(!strcmp(ptr, "rd")) {
            op=1;
            }
        if(!strcmp(ptr, "wr")) {
            op=2;
            }
        else if(xatoi(&ptr, &t)) {
            n = t;
            op=4;
            if(xatoi(&ptr, &t)) {
                data = t;
                op=5;
                }
            }
        }
    else op=0;
    //
    if(op == 0) {
        viev_data(-1);
        }
    else if(op == 1) {
        restore_data(0);
        }
    else if(op == 2) {
        save_data(0);
        }
    else if(op == 3) {
        init_data();
        save_data(0);
        }
    else if(op == 4) {
        viev_data(n);
        }
    else if(op == 5) {
        edit_data(n, data);
        }
}

//extern volatile uint32_t dt_cap;

void cap(char* s)
{
    //xprintf("dt_cap=%d\n", dt_cap);
}
