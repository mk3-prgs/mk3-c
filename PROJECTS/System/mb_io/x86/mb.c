#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <unistd.h>
///#include <modbus/modbus-rtu.h>
#include <time.h>
#include <string.h>

#include <modbus.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//
#include "../file_io.h"
//
#define DTAB_LEN (32)
//
void hex_dump(uint8_t* bf, int len);
int mb_open(char* dev, int nom);
int mb_tx(int addr, uint8_t *bf, int len);
int mb_rx(int addr, uint8_t *bf, int len);
void mb_close(void);

// Контекст Modbus.
static modbus_t* modbus = NULL;

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
    //printf("%s id=%d %d [%d] %d", pmb->f_name, pmb->id, pmb->l_bf, pmb->f_len, (int)sizeof(state_t));
    //hex_dump(pmb->bf, pmb->l_bf);
    //
    len = sizeof(mb_sys);
    if(pmb->l_bf==0) {
        len = MB_SYS_HEAD_LEN;
        }
    res=mb_tx(0x0000, (uint8_t*)pmb, len);
    //
    return(res);
}

int mb_recv(mb_sys* pmb)
{
int res=0;
    res=mb_rx(0x0000, (uint8_t*)pmb, sizeof(mb_sys));
    return(res);
}

state_t mb_state(void)
{
int res=0;
int addr=(int)((void*)&(mb.state) - (void*)&mb);
state_t state;
    //
    res=mb_rx(addr/2, (uint8_t*)&state, sizeof(state_t));
    //
    if(res) state=-1;
    //
    return(state);
}

state_t mb_send_state(state_t state)
{
int res=0;
int addr=(int)((void*)&(mb.state) - (void*)&mb);
    //
    res=mb_tx(addr/2, (uint8_t*)&state, sizeof(state_t));
    //
    return(res);
}

state_t mb_wait(state_t w_st)
{
state_t st;

    do {
        usleep(10000);
        st = mb_state();
        } while(st == w_st);
    //
    //if(st != READY) {
    printf("mb_wait(): state=%d\n", st);
    //    }
    return(st);
}

void usage(char* name)
{
    printf("Usage:\n  %s [-key [arg]] [file name]\n", name);
    printf("    -d dev      Modbus device [default: /dev/ttyUSB1].\n");
    printf("    -a n        Modbus address [default: 1].\n");
    printf("    -T          Set system time.\n");
    printf("    -s          Send file [default].\n");
    printf("    -r          Receive file.\n");
    printf("    -f name     file  [default file name: mk.bin].\n");
    printf("    -R nom      Read register.\n");
    printf("    -W nom data Write register.\n");
    printf("    -p          Programm file.\n");
    printf("    -S n        Save profile to file.\n");
    printf("    -L n        Load profile from file.\n");
    //
}

// Главная функция.
int main(int argc, char* argv[])
{
int i;
int fd=-1;
struct stat fs;
int res;
int op=0; // read
char name[]="ink.bin";
char *f_name=name;
int f_ops = O_RDONLY;
//
char dev[]="/dev/ttyUSB1";
char* p_dev = dev;
char cmd[32];
char* ps;
int nom=1;
int a_reg=0;
int w_reg=0;
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
            if(*ps == 's')      { op = 0; f_ops = O_RDONLY; }
            else if(*ps == 'r') { op = 1; f_ops = O_WRONLY | O_CREAT; }
            else if(*ps == 'T') { op = 5; }
            else if(*ps == 'R') {
                op = 3;
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        a_reg = atoi(ps);
                        }
                    else {
                        printf("Err: %s -R ?\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'W') {
                op = 4;
                i++;
                if(i < argc) {
                    ps = argv[i];
                    a_reg = atoi(ps);
                    i++;
                    if(i < argc) {
                        ps = argv[i];
                        w_reg = atoi(ps);
                        }
                    }
                else {
                    printf("Err: %s -W ? ?\n", argv[0]);
                    return(-1);
                    }
                }
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
            else if(*ps == 'p') {
                op = 2;
                }
            else if(*ps == 'S') {
                op = 6;
                a_reg=0;
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        a_reg = atoi(ps);
                        }
                    else {
                        printf("Err: %s -R ?\n", argv[0]);
                        return(-1);
                        }
                    }
                else { usage(argv[0]); return(-1); }
                }
            else if(*ps == 'L') {
                op = 7;
                a_reg=0;
                i++;
                if(i < argc) {
                    ps = argv[i];
                    if(*ps != '-') {
                        a_reg = atoi(ps);
                        }
                    else {
                        printf("Err: %s -R ?\n", argv[0]);
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
    ///printf("mb=%s file=%s op=%d nom=%d a_reg=%d w_reg=%d\n", p_dev, f_name, op, nom, a_reg, w_reg);
    //
    ///return(0);
    //
    //
    memset(mb.f_name, 0, 16);
    strncpy(mb.f_name, f_name, 15);
    mb.f_len = 0;
    mb.id=0;

    memset(cmd, 0, 32);
    if(op==0) {  // Отправить файл
        strncpy(cmd, "send", 31);
        //
        fd = open(f_name, f_ops, 0644);
        if(fd<0) {
            printf("%s Ошибка открытия файла %s.\n", argv[0], f_name);
            return(-2);
            }
        fstat(fd, &fs);
        printf("Send file: %s Size: %d\n", f_name, (int)fs.st_size);
        mb.f_len = (int)fs.st_size;
        }
    else if(op == 1) { // Принять файл
        strncpy(cmd, "recv", 31);
        //
        fd = open(f_name, f_ops, 0644);
        if(fd<0) {
            printf("%s Ошибка открытия файла %s.\n", argv[0], f_name);
            return(-2);
            }
        printf("Receive file: %s\n", f_name);
        }
    else if(op == 2) { // запрограммировать mk2-c_mcs51
        printf("Programm %s\n", f_name);
        //
        fd = open(f_name, f_ops, 0644);
        if(fd<0) {
            printf("%s Ошибка открытия файла %s.\n", argv[0], f_name);
            return(-2);
            }
        fstat(fd, &fs);
        printf("Send file: %s Size: %d\n", f_name, (int)fs.st_size);
        mb.f_len = (int)fs.st_size;
        //
        strncpy(cmd, "prg", 31);
        }
    else if(op == 3) { // запрограммировать mk2-c_mcs51
        //printf("Read registers:\n");
        strncpy(cmd, "r_reg", 31);
        }
    else if(op == 4) { // запрограммировать mk2-c_mcs51
        //printf("Write registers:\n");
        strncpy(cmd, "w_reg", 31);
        }
    else if(op == 5) { // запрограммировать mk2-c_mcs51
        //printf("Write registers:\n");
        strncpy(cmd, "set_time", 31);
        }
    else if(op == 6) { // запрограммировать mk2-c_mcs51
        //printf("Write registers:\n");
        strncpy(cmd, "s_prf", 31);
        }
    else if(op == 7) { // запрограммировать mk2-c_mcs51
        //printf("Write registers:\n");
        strncpy(cmd, "l_prf", 31);
        }
    //
    //==================================================================
    //
    if(mb_open(p_dev, nom)) {
        printf("Error mb_open(\"%s\"); in %s.\n", p_dev, argv[0]);
        if(fd>=0) close(fd);
        return(-1);
        }

    //for(;;) {
    //    memset(cmd, 0, 32);
    //    printf("> ");
    //    fflush(stdout);
    //    fgets(cmd, 32, stdin);
    //    for(i=0; i<32; i++) {if((cmd[i]=='\r')||(cmd[i]=='\n')) { cmd[i]='\0'; break; }}
    //    printf("cmd: [%s]\n", cmd);
        //
        if(!strcmp(cmd, "exit")) {
            return(0);
            }
        else if(!strcmp(cmd, "frun")) {
            memset(mb.f_name, 0, 16);
            strncpy(mb.f_name, f_name, 15);
            mb.f_len = (int)fs.st_size;
            mb.id=0;
            mb.l_bf = 0;
            mb.state=RUN;
            mb_send(&mb);
            if(mb_wait(RUN)) return(0);
            }

        else if((!strcmp(cmd, "send")) || (!strcmp(cmd, "prg"))) {
            memset(mb.f_name, 0, 16);
            strncpy(mb.f_name, f_name, 15);
            mb.f_len = (int)fs.st_size;
            mb.id=0;
            mb.l_bf = 0;
            mb.state=F_WRITE;
            mb_send(&mb);
            if(mb_wait(F_WRITE)) return(-1);
            //
            off_t offset = 0;
            off_t pos;
            //
            for(;;) {
                if(mb.f_len <= 0) {
                    //printf("mb.f_len == 0\n");
                    break;
                    }
                offset += 128;
                pos = lseek(fd, offset, SEEK_SET);
                if(pos <= 0) {
                    printf("\nEnd file!\n");
                    printf("\npos=%d err=%d %s\n", (int)pos, errno, strerror(errno));
                    break;
                    }
                res = read(fd, mb.bf, 128);
                //
                if(res < 0) {
                    printf("\nError read(): res <=0 \n");
                    printf("\npos=%d err=%d %s\n", (int)pos, errno, strerror(errno));
                    break;
                    }
                else if(res == 0) {
                    printf("\nDone!\n");
                    printf("\npos=%d err=%d %s\n", (int)pos, errno, strerror(errno));
                    break;
                    }
                mb.l_bf = res;
                mb.state=SEND;
                mb.f_len -= res;
                //
                if((res=mb_send(&mb)) != 0) {
                    printf("\nError mb_send(): res <=0 \n");
                    break;
                    }
                //
                if(mb_wait(SEND)) break;
                //
                printf("\r%6d send: %6d        ", (int)offset, mb.f_len); fflush(stdout);
                mb.id++;
                }
            //
            printf("\n");
            //
            mb.id=0;
            mb.l_bf = 0;
            mb.state=DONE;
            mb_send(&mb);
            mb_wait(DONE);
            //
            if(!strcmp(cmd, "prg")) {
                //
                memset(mb.f_name, 0, 16);
                strncpy(mb.f_name, f_name, 15);
                mb.f_len = 0;
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
            memset(mb.f_name, 0, 16);
            strncpy(mb.f_name, f_name, 15);
            mb.f_len = 0;
            mb.id=0;
            mb.l_bf = 0;
            mb.state=F_READ;
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
                    res = write(fd, mb.bf, mb.l_bf);
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
                    close(fd);
                    fd =-1;
                    return(0);
                    }
                //
                printf("\rосталось: %6d        ", mb.f_len); fflush(stdout);
                //printf("\rst=%04d    ", st_pck); fflush(stdout);
                }
            printf("\n");fflush(stdout);
            }
        else if(!strcmp(cmd, "r_reg")) {
            int16_t reg;
            //
            if(a_reg>DTAB_LEN) a_reg=DTAB_LEN;
            else if(a_reg<0) a_reg=0;
            //
            res=mb_rx(0x1000+a_reg, (uint8_t*)&reg, 2);
            if(res==0) printf("%d : %d\n", a_reg, reg);
            else printf("Error!\n");
            }
        else if(!strcmp(cmd, "s_prf")) {
            mb.id = a_reg;
            mb.state=S_PRF;
            mb_send(&mb);
            if(mb_wait(S_PRF)) return(-1);
            }
        else if(!strcmp(cmd, "l_prf")) {
            mb.id = a_reg;
            mb.state=L_PRF;
            mb_send(&mb);
            if(mb_wait(L_PRF)) return(-1);
            }
        else if(!strcmp(cmd, "w_reg")) {
            int16_t reg;
            //
            if(a_reg>DTAB_LEN) a_reg=DTAB_LEN;
            else if(a_reg<0) a_reg=0;
            //
            res =  mb_tx(0x1000+a_reg, (uint8_t*)&w_reg, 2);
            res += mb_rx(0x1000+a_reg, (uint8_t*)&reg, 2);
            if(res==0) printf("%d : %d\n", a_reg, reg);
            else printf("Error!\n");
            }
        else if(!strcmp(cmd, "set_time")) {
            //
            uint32_t ut = (uint32_t)time(NULL);
            ut -= (3*3600);
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
        else {
            state_t st = mb_state();
            printf("state=%d\n", st);
            }
    //
    mb_close();
    if(fd >= 0) close(fd);
    res=0;
    return(res);
}

int mb_open(char* dev, int nom)
{
    ///modbus_new_rtu(const char *device, int baud, char parity, int data_bit, int stop_bit, int rts);
    modbus = modbus_new_rtu(dev, 115200, 'N', 8, 1, 0);
    if(!modbus){
        printf("Error creating modbus context!\n");
        return(-1);
        }
    modbus_set_debug(modbus, 0);
    // Режим - RS-232.
    modbus_rtu_set_serial_mode(modbus, MODBUS_RTU_RS232);
    // Установим адрес ведомого.
    modbus_set_slave(modbus, nom);
    //
    //modbus_get_response_timeout(modbus, &response_timeout);
    //response_timeout.tv_sec = 0;
    //response_timeout.tv_usec = 1000000;
    //modbus_set_response_timeout(modbus, 0, 500000); //&response_timeout);
    //
    // Соединимся.
    if(modbus_connect(modbus) == -1){
        printf("Error connecting to slave!: %d\n", errno);
        printf(modbus_strerror(errno));
        modbus_close(modbus);
        modbus_free(modbus);
        return(-1);
        }
    return(0);
}

int mb_tx(int addr, uint8_t *bf, int len)
{
    if(modbus_write_registers(modbus, addr, len/2, (uint16_t*)bf) == -1) {
        printf("Error writing reg hold register: %d!\n", errno);
        printf(modbus_strerror(errno));
        return(-1);
        }
    return(0);
}

int mb_rx(int addr, uint8_t *bf, int len)
{
    if(modbus_read_registers(modbus, addr, len/2, (uint16_t*)bf) == -1) {
        printf("Error reading reg hold register: %d!\n", errno);
        printf(modbus_strerror(errno));
        return(-1);
        }
    return(0);
}

void mb_close(void)
{
    modbus_close(modbus);
    modbus_free(modbus);
}

void hex_dump(uint8_t* bf, int len)
{
int i;
    if(bf != (uint8_t*)(0)) {
        printf("\n");
        for(i=0; i<len; i++) {
            if((i!=0) && !(i%32)) printf("\n");
            printf("%02x", bf[i]);
            }
        printf("\n");
        }
}
