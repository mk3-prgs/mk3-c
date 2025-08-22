#include "shell.h"
#include "rtc_32f1.h"
#include "bkp_data.h"

extern uint32_t _sbss0[]; // start
extern uint32_t _ebss0[]; // end

extern ptentry parsetab[];

void help(char *str)
{
ptentry *p;
    xprintf("  Справка по командам:\r\n");
    for(p = parsetab; p->commandstr != NULL; ++p) {
        xprintf("    %s - %s\r\n", p->commandstr, p->help);
        }
}

void kill(char *s)
{
xTaskHandle th=NULL;
long t;
char *ptr=&s[5];
    //
    if(xatoi(&ptr, &t)) {
            th = (xTaskHandle)t;
            vTaskDelete(th);
            xprintf("task 0x%08x deleted!\n", t);
            }
    else {
        xprintf("Error!\n");
        }
}

extern int ink_exit; //dfdata.h

void ink(char *s)
{
char *pt=&s[4];
    if(!strcmp(pt, "start")) {
        uint32_t *ps= (void*)0x08040000;
        void (*p)(void* arg);
        p = (void*)*(ps);
        if((uint32_t)p != 0xffffffff) {
            xTaskCreate( p, "startup",  configMINIMAL_STACK_SIZE*2, NULL, 4, (xTaskHandle*)NULL);
            }
        }
    else if(!strcmp(pt, "stop")) {
        ink_exit=1;
        }
}

void run(char *s)
{
xTaskHandle th=NULL;
uint32_t *ps= (void*)0x08040000;
void (*p)(void* arg);
    p = (void*)*(ps);
    //
    xprintf("start address: 0x%08x\n", p);
    //
    xTaskCreate( p, "run", configMINIMAL_STACK_SIZE*4, NULL, 4, &th);
}

extern uint8_t ucHeap[];

void heap(char *s)
{
///FIL *fp = (FIL*)pvPortMalloc(sizeof(FIL));
/*
    xprintf("_sidata0:   0x%08x\n", _sidata0);
    xprintf("_beg_data0: 0x%08x\n", _beg_data0);
    xprintf("_end_data0: 0x%08x\n\n", _end_data0);

    xprintf("_sidata= 0x%08x\n", _sidata);
    xprintf("_sdata = 0x%08x\n", _sdata);
    xprintf("_edata = 0x%08x\n", _edata);
*/
    //xprintf("_sbss0  = 0x%08x\n", _sbss0);
    //xprintf("_ebss0  = 0x%08x\n", _ebss0);
///float pi=3.1415;
    ///if(fp) {
        size_t x = xPortGetFreeHeapSize();
        uint32_t y = configTOTAL_HEAP_SIZE;
        xprintf("Heap: 0x%08x total=%u free=%u\n", ucHeap, y, x);
        ///fp->pstr = fp->buf;
        ///memset(fp->buf, 0, 2048);
        ///f_printf(fp, "Heap: 0x%08x total=%u free=%u pi=%5.4f\n", ucHeap, y, x, pi);
        ///xputs((char*)fp->buf);
        ///vPortFree(fp);
        ///}
}

void ps(char *s)
{
char *bf=NULL;
    bf = (char*)pvPortMalloc(4096);
    memset(bf, 0, 4096);
    if(bf) {
        vTaskList(bf);
                //
                //cStatus,
                //uxCurrentPriority,
                //usStackHighWaterMark,
                //xTaskNumber
                //
        xprintf("Name\t\tStatus\tPrio\tStack\tNumber\thandle\n");
        xprintf("%s\r\n", bf);
        vPortFree(bf);
        }
}

void stat(char *s)
{
char *bf = (char*)pvPortMalloc(4096);
    if(bf) {
        vTaskGetRunTimeStats(bf);
        //xprintf("Stat\n");
        xprintf("%s\n", bf);
        if(bf) vPortFree(bf);
        }
}

void stime(char *s)
{
uint32_t t;
    g_time(&t);
    xprintf("%u\n", t);
}


void date(char *s)
{
RTCTIME rtc;
    rtc_gettime(&rtc);
/*
uint32_t tt = get_fattime();
    xprintf("%u\r\n", tt);
*/
//int i;
long t;
char *ptr=&s[5];
    //
    while(*ptr) {
            if((*ptr == ':') || (*ptr == '-')) *ptr=' ';
            ptr++;
            }
    if(ptr != &s[5]) {
        ptr=&s[5];
        if(xatoi(&ptr, &t)) {
            rtc.hour = t;
            if(xatoi(&ptr, &t)) {
                rtc.min = t;
                if(xatoi(&ptr, &t)) {
                    rtc.sec = t;
                    if(xatoi(&ptr, &t)) {
                        rtc.mday = t;
                        if(xatoi(&ptr, &t)) {
                            rtc.month = t;
                            if(xatoi(&ptr, &t)) { rtc.year = t; }
                            }
                        }
                    }
                }
            }
        rtc_settime(&rtc);
        //
        xprintf("Set time: %02u:%02u:%02u %02u-%02u-%04u\n",
                    rtc.hour, rtc.min, rtc.sec,
                    rtc.mday, rtc.month, rtc.year);
        }
    else {
        xprintf("%02u:%02u:%02u %02u-%02u-%04u\n",
                    rtc.hour, rtc.min, rtc.sec,
                    rtc.mday, rtc.month, rtc.year);
        }
}

void md(char *s)
{
long t;
char *pt=&s[3];
//
uint8_t* ptr=0;
int32_t cnt=0x200;
int ofs=0;
int l;
    //
    //
    if(xatoi(&pt, &t)) {
            ptr = (uint8_t*)t;
            ofs = t;
            if(xatoi(&pt, &t)) {
                cnt = 0x200 * t;
                }
            }
    //
    for(; cnt > 0; ptr += 32, ofs += 32) {
        if(cnt<32) l=cnt;
        else l=32;
        put_dump((BYTE*)ptr, ofs, l, 1);
        cnt -= 32;
        if((cnt % 0x200)==0) xprintf("\n");
        }
}

void io(char *s)
{
int wr = 0;
uint32_t addr=0x4000000;
uint32_t data;
long l;
    //
    char *ptr = s+2;
    //
    if(xatoi(&ptr, &l)) {
        addr = l;
        if(xatoi(&ptr, &l)) {
            data = l;
            wr=1;
            }
        }
    if(wr) *(__IO uint32_t *)addr = data;
    xprintf("%08x: %08x\n", addr,  *(__IO uint32_t *)addr);
}

//=============================================================================
//
//

void PWR_BackupAccessCmd(int en);
/*
RCU_HXTAL      = RCU_REGIDX_BIT(CTL_REG_OFFSET, 16U),
RCU_LXTAL      = RCU_REGIDX_BIT(BDCTL_REG_OFFSET, 0U),
RCU_IRC8M      = RCU_REGIDX_BIT(CTL_REG_OFFSET, 0U),
RCU_IRC40K     = RCU_REGIDX_BIT(RSTSCK_REG_OFFSET, 0U),
RCU_PLL_CK     = RCU_REGIDX_BIT(CTL_REG_OFFSET, 24U),
*/
void rtc_init(char* str)
{
char* s = &str[9];
rcu_osci_type_enum s_sin;

    if(!strcmp(s, "HTAL")) {
        s_sin = RCU_HXTAL;
        }
    else if(!strcmp(s, "LXTAL")) {
        s_sin = RCU_LXTAL;
        }
    else if(!strcmp(s, "IRC40K")) {
        s_sin = RCU_IRC40K;
        }
    else s_sin = RCU_LXTAL;
    //
    xprintf("s=%s [%d]\n", s, s_sin);
    //
    rtc_configuration(s_sin);
    rtc_counter_set(0);
    rtc_lwoff_wait();
    //
}

//rtc_sync();

void bkp(char *s)
{
int i;
uint16_t d=0;
long t;
int op=0;
int adr=BKP_DATA_0;
uint16_t data=0;
//
char *ptr=NULL;
//
    if(strlen(s) > 4) {
        ptr = &s[4];
        if(!strcmp(ptr, "init")) {
            op=3;
            }
        else if(xatoi(&ptr, &t)) {
            //
            t++;
            //
            if(t<BKP_DATA_0) t=BKP_DATA_0;
            else if(t>BKP_DATA_41) t=BKP_DATA_41;
            //
            adr = t;
            op=1;
            if(xatoi(&ptr, &t)) {
                data = t;
                op=2;
                }
            }
        }
    else op=0;
    //
    if(op == 0) {
        for(i=BKP_DATA_0; i <= BKP_DATA_41; i++) {
            d = bkp_read_word(i);
            xprintf("  BKP_DATA_%02d: %04x (%5d)\n", i-1, d, d);
            }
        }
    else if(op == 1) {
            d = bkp_read_word(adr);
            xprintf("  BKP_DATA_%02d: %04x (%5d)\n", adr-1, d, d);
            }
    else if(op == 2) {
        bkp_write_word(adr, data);
        d = bkp_read_word(adr);
        xprintf(" BKP_DATA_%02d: %04x (%5d)\n", adr-1, d, d);
        }
    else if(op == 3) {
        for(i=BKP_DATA_0; i <= BKP_DATA_41; i++) {
            bkp_write_word(i, 0);
            xprintf(" BKP_DATA_%02d: =0\n", i-1);
            }
        }
}

char* tab[]=
{
"RTC_INTEN", //                   REG32(RTC + 0x00U)      //interrupt enable register
"RTC_CTL",   //                   REG32(RTC + 0x04U)      // control register
"RTC_PSCH",  //                   REG32(RTC + 0x08U)      // prescaler high register
"RTC_PSCL",  //                   REG32(RTC + 0x0CU)      // prescaler low register
"RTC_DIVH",  //                   REG32(RTC + 0x10U)      // divider high register
"RTC_DIVL",  //                   REG32(RTC + 0x14U)      // divider low register
"RTC_CNTH",  //                   REG32(RTC + 0x18U)      // counter high register
"RTC_CNTL",  //                   REG32(RTC + 0x1CU)      // counter low register
"RTC_ALRMH", //                   REG32(RTC + 0x20U)      // alarm high register
"RTC_ALRML", //                   REG32(RTC + 0x24U)      // alarm low register
};

void rtc_en(char *s)
{
    // wait RTC registers synchronized flag set
    rtc_register_sync_wait();
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

void rtc_ds(char *s)
{
    rtc_interrupt_disable(RTC_INT_SECOND);
}

void r_rtc(char *s)
{
uint32_t base =  (RTC + 0x00U);
    xprintf("%9s[0x%08x]=0x%08x\n", tab[0], base, RTC_INTEN); base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[1], base, RTC_CTL);   base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[2], base, RTC_PSCH);  base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[3], base, RTC_PSCL);  base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[4], base, RTC_DIVH);  base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[5], base, RTC_DIVL);  base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[6], base, RTC_CNTH);  base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[7], base, RTC_CNTL);  base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[8], base, RTC_ALRMH); base+=4;
    xprintf("%9s[0x%08x]=0x%08x\n", tab[9], base, RTC_ALRML);
}

