#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <pthread.h>

#include <modbus.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//
#include "../file_io.h"
//
void view_mk2(uint8_t* bf);
//
int con_bf_len;
char con_bf[128];

void *con_input(void *param);

void hex_dump(uint32_t addr, uint8_t* bf, int len);
//
int  fmb_open(char* dev, int nom, int speed);
int  fmb_tx(int addr, uint16_t *bf, int len);
int  fmb_rx(int addr, uint16_t *bf, int len);
void fmb_close(void);
//
int  mk2_open(char* dev, int nom, int speed);
int  mk2_rx(int addr, uint16_t *rx_bf, int len);
int  mk2_tx(int addr, uint16_t *tx_bf, int len);
void mk2_close(void);
//
int  (*mb_open)(char* dev, int nom, int speed);
int  (*mb_rx)(int addr, uint16_t *bf, int len);
int  (*mb_tx)(int addr, uint16_t *bf, int len);
void (*mb_close)(void);

mb_sys mb;

/*
struct stat {
    dev_t         st_dev;      // устройство
    ino_t         st_ino;      // inode
    mode_t        st_mode;     // режим доступа
    nlink_t       st_nlink;    // количество жестких ссылок
    uid_t         st_uid;      // идентификатор пользователя-владельца
    gid_t         st_gid;      // идентификатор группы-владельца
    dev_t         st_rdev;     // тип устройства
                               // (если это устройство)
    off_t         st_size;     // общий размер в байтах
    blksize_t     st_blksize;  // размер блока ввода-вывода
                               // в файловой системе
    blkcnt_t      st_blocks;   // количество выделенных блоков
    time_t        st_atime;    // время последнего доступа
    time_t        st_mtime;    // время последней модификации
    time_t        st_ctime;    // время последнего изменения
};

int fstat(int fd, struct stat *buf);
*/

/*
char* state_text[]=
{
    "READY",
    "WRITE",
    "READ",
    "SEND",
    "RECV",
    "DONE",
    "FF_ERROR"
};

char* t_state(state_t st, char* str)
{
int i = (int)st;
     sprintf(str, "%s", state_text[i]);
     return(str);
}
*/

int mb_send(mb_sys* pmb)
{
int res=0;
int len;
    //printf("%s id=%d l_bf=%d [%d] state=%d [%d]\n", pmb->f_name, pmb->id, pmb->l_bf, pmb->f_len, pmb->state, (int)sizeof(state_t));
    //
    len = sizeof(mb_sys)/2;
    if(pmb->l_bf==0) {
        len = MB_SYS_HEAD_LEN/2;
        }

    //hex_dump(0x0000, (uint8_t*)pmb, 2*len);

    res=mb_tx(0x0000, (uint16_t*)pmb, len);
    //
    return(res);
}

int mb_recv(mb_sys* pmb)
{
int len = sizeof(mb_sys)/2;
int res=0;
    res=mb_rx(0x0000, (uint16_t*)pmb, len);
    return(res);
}

state_t mb_state(void)
{
int res=0;
int addr=(int)((void*)&(mb.state) - (void*)&mb);
state_t state;
    //
    res=mb_rx(addr/2, (uint16_t*)&state, sizeof(state_t)/2);
    //printf("mb_state: %d\n", state);
    //
    if(res) state=-1;
    //
    return(state);
}

state_t mb_id(void)
{
int res=0;
int addr=(int)((void*)&(mb.id) - (void*)&mb);
state_t state;
    //
    res=mb_rx(addr/2, (uint16_t*)&state, sizeof(state_t)/2);
    //printf("mb_state: %d\n", state);
    //
    if(res) state=-1;
    //
    return(state);
}

int mb_lbf(void)
{
int res=0;
int addr=(int)((void*)&(mb.l_bf) - (void*)&mb);
int lbf;
    //
    res=mb_rx(addr/2, (uint16_t*)&lbf, sizeof(int)/2);
    //printf("mb_state: %d\n", state);
    //
    if(res) lbf=0;
    //
    return(lbf);
}

int mb_send_lbf(int n)
{
int res=0;
int addr=(int)((void*)&(mb.l_bf) - (void*)&mb);
int lbf = n;
    //
    res=mb_tx(addr/2, (uint16_t*)&lbf, sizeof(int)/2);
    //
    return(res);
}

state_t mb_send_state(state_t state)
{
int res=0;
int addr=(int)((void*)&(mb.state) - (void*)&mb);
    //
    res=mb_tx(addr/2, (uint16_t*)&state, sizeof(state_t)/2);
    //
    return(res);
}

state_t mb_send_id(state_t state)
{
int res=0;
int addr=(int)((void*)&(mb.id) - (void*)&mb);
    //
    res=mb_tx(addr/2, (uint16_t*)&state, sizeof(state_t)/2);
    //
    return(res);
}

state_t mb_wait(state_t w_st)
{
state_t st;

    //printf("mb_wait() %d\n", w_st);

    do {
        usleep(3000);
        st = mb_state();
        } while(st == w_st);
    //
    //if(st != READY) {
    //    printf("mb_wait() %d: st=%d\n", w_st, st);
    //    }
    //
    return(st);
}

void usage(char* name)
{
    printf("Usage:\n  %s [-key [arg]] [file name]\n", name);
    printf("    -d dev      Modbus device [default: /dev/ttyUSB1].\n");
    printf("    -a n        Modbus address [default: 1].\n");
    printf("    -b n        Baud [default: 115200].\n");
    printf("    -m n        Protokol 0 - modbus, 1 - mk2-c [default: 0.]\n");
    printf("    -T          Set system time.\n");
    printf("    -s          Send file [default].\n");
    printf("    -r          Receive file.\n");
    printf("    -V          Target volume.\n");
    printf("    -f name     file  [default file name: mk.bin].\n");
//    printf("    -R nom      Read register.\n");
//    printf("    -W nom data Write register.\n");
    printf("    -p          Programm file.\n");
//    printf("    -S n        Save profile to file.\n");
//    printf("    -L n        Load profile from file.\n");
    printf("    -c          Console.\n");
}

static int nom=1;

// Главная функция.
int main(int argc, char* argv[])
{
int i;
int f_d  = -1;
struct stat fs;
int res;
int op=0; // read
int vol=0;
char name[]="mk.bin";
char *f_name=name;
//
char dev[]="/dev/ttyUSB1";
char* p_dev = dev;
char cmd[64];
char* ps;
int mk2=0;
int speed=115200;
//
    if(argc < 2) {
        usage(argv[0]);
        return(0);
        }
//
    for(i=1; i<argc; i++) {
        ps = argv[i];
        if(*ps == '-') {
            ps++;
            if(*ps == 's')      { op = 0; }
            else if(*ps == 'x') { op = 8; }
            else if(*ps == 'y') { op = 9; }
            else if(*ps == 'c') { op = 10; }
            else if(*ps == 'r') { op = 1; }
            else if(*ps == 'T') { op = 5; }
            else if(*ps == 'p') { op = 2; }
            else if(*ps == 'f') {
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') f_name = ps;
                    else {
                        printf("Err: %s -f ? name\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'd') {
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') p_dev = ps;
                    else {
                        printf("Err: %s -d ? dev\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'V') {
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        vol = atoi(ps);
                        }
                    else {
                        printf("Err: %s -V ?\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'a') {
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        nom = atoi(ps);
                        }
                    else {
                        printf("Err: %s -a ?\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'b') {
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        speed = atoi(ps);
                        }
                    else {
                        printf("Err: %s -b ?\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'm') {
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        mk2 = atoi(ps);
                        }
                    else {
                        printf("Err: %s -m ?\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            }
        else {
            if(i < argc) {
                ps = argv[i];
                if(*ps != '-') f_name = ps;
                else {
                    printf("Err: %s -f ? name\n", argv[0]);
                    return(-1);
                    }
                }
            }
        }
    //
    memset(mb.f_name, 0, 16);
    strncpy(mb.f_name, f_name, 15);
    mb.f_len = 0;
    mb.id=0;

    memset(cmd, 0, 32);
    if(op==0) {  // Отправить файл
        strncpy(cmd, "send", 31);
        }
    else if(op == 1) { // Принять файл
        strncpy(cmd, "recv", 31);
        //printf("Receive file: %s\n", f_name);
        }
    else if(op == 2) { // запрограммировать mk2-c_mcs51
        //printf("Programm %s\n", f_name);
        strncpy(cmd, "prg", 31);
        }
    else if(op == 3) {
        //printf("Read registers:\n");
        strncpy(cmd, "r_reg", 31);
        }
    else if(op == 4) {
        //printf("Write registers:\n");
        strncpy(cmd, "w_reg", 31);
        }
    else if(op == 5) {
        //printf("Write registers:\n");
        strncpy(cmd, "set_time", 31);
        }
    else if(op == 6) {
        //printf("Write registers:\n");
        strncpy(cmd, "s_prf", 31);
        }
    else if(op == 7) {
        //printf("Write registers:\n");
        strncpy(cmd, "l_prf", 31);
        }
    else if(op == 8) {
        //printf("Write registers:\n");
        strncpy(cmd, "r_mk2", 31);
        }
    else if(op == 9) {
        //printf("Write registers:\n");
        strncpy(cmd, "w_mk2", 31);
        }
    else if(op == 10) {
        //printf("Write registers:\n");
        strncpy(cmd, "console", 31);
        }
    //
    //
    //printf("cmd: [%s] mb=%s file=%s op=%d nom=%d a_reg=%d w_reg=%d mk2=%d\n", cmd, p_dev, f_name, op, nom, a_reg, w_reg, mk2);
    //
    ///return(0);
    //
    if(mk2) {
        mb_open  = mk2_open;
        mb_rx    = mk2_rx;
        mb_tx    = mk2_tx;
        mb_close = mk2_close;
        }
    else {
        mb_open  = fmb_open;
        mb_rx    = fmb_rx;
        mb_tx    = fmb_tx;
        mb_close = fmb_close;
        }
    //
    //==================================================================
    //
    if(mb_open(p_dev, nom, speed)) {
        printf("Error mb_open(\"%s\"); in %s.\n", p_dev, argv[0]);
        return(-1);
        }
    //p_start(p_dev);
    //
    if(!strcmp(cmd, "exit")) {
        return(0);
        }

    else if(!strcmp(cmd, "r_mk2")) {
        int len = 40/2;
        uint16_t tst[40/2];
        memset(tst, 0x00, len);
        int res = mb_rx(0x002e, tst, len);
        /*
        uint16_t *pt = (uint16_t*)tst;
        for(i=0; i<(38/2); i++) {
            uint16_t h = 0xff00 & (*pt << 8);
            uint16_t l = 0x00ff & (*pt >> 8);
            *pt = h | l;
            pt++;
            }
        */
        if(!res) {
            uint8_t *pt = (uint8_t*)tst;
            printf("res=%d [ ", res);
            for(i=0; i<2*len; i++) printf("%02x ", *pt++);
            printf("]\n");
            //
            pt = (uint8_t*)tst;
            view_mk2(pt);
            }
        else {
            printf("Error read!\n");
            }
        //hex_dump(0x2000, tst, len);
        }

    else if(!strcmp(cmd, "w_mk2")) {
        mb.l_bf = 0;
        mb.f_len = -1;
        mb.id = -1;
        uint16_t* pt = (uint16_t*)&mb;
        for(i=0; i<MB_SYS_HEAD_LEN/2; i++) {
            if(!(i%16)) printf("\n");
            printf("%04x ", *pt++);
            }
        printf("\n");
        mb_send(&mb);
        //
        mb_recv(&mb);
        pt = (uint16_t*)&mb;
        for(i=0; i<MB_SYS_HEAD_LEN/2; i++) {
            if(!(i%16)) printf("\n");
            printf("%04x ", *pt++);
            }
        printf("\n");
        }
    else if(!strcmp(cmd, "frun")) {
        //
        memset(mb.f_name, 0, 16);
        strncpy(mb.f_name, f_name, 15);
        mb.f_len = 0;
        mb.id=0;
        mb.l_bf = 0;
        mb.state=RUN;
        mb_send(&mb);
        if(mb_wait(RUN)) return(0);
        //
        }
    else if((!strcmp(cmd, "send")) || (!strcmp(cmd, "prg"))) {
        //
        FILE* fp=NULL;
        fp = fopen(f_name, "rb");
        if(fp==NULL) {
            printf("%s Ошибка открытия файла %s.\n", argv[0], f_name);
            return(-2);
            }
        //
        f_d = fileno(fp);
        fstat(f_d, &fs);
        f_d = -1;
        //
        printf("Send file: %s Size: %d\n", f_name, (int)fs.st_size);
        //
        memset(mb.f_name, 0, 16);
        //strncpy(mb.f_name, f_name, 15);
        sprintf(mb.f_name, "%d:/%s", vol, f_name);
        mb.f_len = (int)fs.st_size;
        mb.id=0;
        mb.l_bf = 0;
        mb.state=F_WRITE;
        mb_send(&mb);
        if(mb_wait(F_WRITE)) return(-1);
        //
        int l_blk=0;
        //uint8_t* pt_bf=bf;
        for(;;) {
            //
            if(mb.f_len >= 128) l_blk=128;
            else if(mb.f_len>0) l_blk=mb.f_len;
            else {
                res=0;
                break;
                }
            //
            res = fread(mb.bf, 1, l_blk, fp);
            if(res<=0) {
                printf("Error read!\n");
                res=-1;
                break;
                }
            //
            //hex_dump(mb.bf, l_blk);
            //
            mb.l_bf = l_blk;
            mb.state=SEND;
            //
            if((res=mb_send(&mb)) != 0) {
                printf("Error mb_send(): res=%d\n", res);
                res=-1;
                break;
                }
            if(mb_wait(SEND)) {
                res=-1;
                break;
                }
            //
            printf("="); fflush(stdout);
            //
            mb.f_len -= l_blk;
            mb.id++;
            }
        if(fp) fclose(fp);
        //
        printf("\n");
        //
        mb.id=0;
        mb.l_bf = 0;
        mb.state=DONE;
        mb_send(&mb);
        mb_wait(DONE);
        //
        if((res == 0) && (!strcmp(cmd, "prg"))) {
            //
            memset(mb.f_name, 0, 16);
            strncpy(mb.f_name, f_name, 15);
            mb.f_len = (int)fs.st_size;
            //
            mb.id=0;
            mb.l_bf = 0;
            mb.state=FLASH;
            mb_send(&mb);
            usleep(5000000);
            mb_wait(FLASH);
            }
        }
    //
    else if(!strcmp(cmd, "recv")) {
        //
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
        f_d = open(f_name, O_RDWR | O_CREAT, mode);
        if(f_d<0) {
            printf("%s Ошибка открытия файла %s.\n", argv[0], f_name);
            return(-2);
            }
        //
        memset(mb.f_name, 0, 16);
        strncpy(mb.f_name, f_name, 15);
        mb.f_len = 0;
        mb.id=0;
        mb.l_bf = 0;
        mb.state=F_READ;
        //
        mb_send(&mb);
        if(mb_wait(F_READ)) return(-1);
        //
        state_t st;
        for(;;) {
            mb_send_state(RECV);
            st = mb_wait(RECV);
            if(st == F_EOF) {
                mb_send_state(DONE);
                mb_wait(DONE);
                //
                /// Done!
                //
                break;
                }
            else if(st != READY) {
                printf("Error: %d\n", st);
                break;
                }
            //
            mb.l_bf=0;
            if((res=mb_recv(&mb)) != 0) {
                printf("Error mb_read(): res <=0 \n");
                break;
                }
            //
            if(mb.l_bf>0) {
                res = write(f_d, mb.bf, mb.l_bf);
                if(res <= 0) {
                    printf("Error write(): res <=0 \n");
                    break;
                    }
                }
            else {
                mb_send_state(DONE);
                mb_wait(DONE);
                //
                mb_close();
                close(f_d);
                f_d =-1;
                return(0);
                }
            //
            printf("\rосталось: %6d        ", mb.f_len); fflush(stdout);
            //printf("\rst=%04d    ", st_pck); fflush(stdout);
            }
        printf("\n");fflush(stdout);
        }
    else if(!strcmp(cmd, "set_time")) {
        //
        uint32_t ut = (uint32_t)time(NULL);
        ut += (3*3600);
        memset(mb.f_name, 0, 16);
        mb.f_len = ut;
        mb.id=0;
        mb.l_bf = 0;
        mb.state=SET_TIME;
        mb_send(&mb);
        if(mb_wait(SET_TIME)) return(-1);
        //
        printf("utc: %u\n", ut);
        }
    else if(!strcmp(cmd, "console")) {
        //
        mb.state=SH_IO;
        memset(mb.bf, 0, 128);
        mb.l_bf=0;
        mb_send(&mb);
        if(mb_wait(SH_IO)) {
            printf("ERROR\n");
            return(-1);
            }
        //
        con_bf_len=0;
        pthread_t th;
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        pthread_create(&th, &attr, con_input, NULL);
        //
        for(;;) {
            //
            if(con_bf_len < 0) {
                //
                mb.state=SH_DONE;
                mb.l_bf=0;
                memset(mb.bf, 0, 128);
                mb_send(&mb);
                if(mb_wait(SH_DONE)) {
                    printf("ERROR\n");
                    return(-1);
                    }
                //
                printf("done\n");
                //
                pthread_join(th, NULL);
                //
                break;
                }
            else if(con_bf_len > 0) {
                //
                memset(mb.bf, 0, 128);
                char* dst = (char*)&mb.bf[64];
                char* src = (char*)con_bf;
                strncpy(dst, src, 64);
                memset(con_bf, 0, 64);
                con_bf_len=0;
                mb.state = SH_IN_READY;
                mb.l_bf=128;
                mb_send(&mb);
                if(mb_wait(SH_IN_READY)) {
                    printf("ERROR\n");
                    return(-1);
                    }
                }
            else {
                mb_recv(&mb);
                if(mb.id == SH_OUT_DONE) {
                    //
                    int l = mb.l_bf;
                    if(l>63) l=63;
                    memset(cmd, 0, 64);
                    strncpy(cmd, (char*)mb.bf, l);
                    mb_send_id(READY);
                    //
                    printf("%s", cmd);
                    fflush(stdout);
                    }
                }
            }
        }
    else {
        state_t st = mb_state();
        printf("state=%d\n", st);
        }
    //
    mb_close();
    if(f_d >= 0) close(f_d);
    res=0;
    //
    return(res);
}

void *con_input(void *param)
{
int i;
char cmd[64];
//
    for(;;) {
        //
        //printf("\n/# "); fflush(stdout);
        memset(cmd, 0, 64);
        fgets(cmd, 63, stdin);
        //fputs(cmd, stdout);
        //
        for(i=0; i<63; i++) {
            if((cmd[i] == '\r')||(cmd[i] == '\n')) { cmd[i] = '\0'; break; }
            }
        //
        //printf("n=%d\n", i);
        //
        if(!strcmp(cmd, "quit")) {
            con_bf_len = -1;
            break;
            }
        else {
            //
            while(con_bf_len > 0) usleep(1000);
            //
            sprintf(con_bf, "%s\r", cmd);
            con_bf_len = strlen(con_bf);
            //
            //if(con_bf_len>0) printf("->%s\n", con_bf);
            }
        }
    //
    pthread_exit(0);
}

void hex_dump(uint32_t addr, uint8_t* bf, int len)
{
int i;
    if(bf != (uint8_t*)(0)) {
        printf("\n");
        for(i=0; i<len; i++) {
            if(!(i%32)) { printf("\n%04x ", addr); addr+=32; }
            printf("%02x ", bf[i]);
            }
        printf("\n");
        }
}
