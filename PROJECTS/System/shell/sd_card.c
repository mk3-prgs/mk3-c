#include "sdcard.h"
#include "shell.h"

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
    //ChipErase();
    //xprintf("Erase %08x\n", sector);
    xprintf("Erase All!\n");
}

