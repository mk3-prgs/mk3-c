#ifndef __FILE_IO_H
#define __FILE_IO_H

typedef enum
{
    READY = 0,
    F_WRITE,
    F_READ,
    SEND,
    RECV,
    DONE,
    F_EOF,
    RUN,
    FF_ERROR,
    SET_TIME,
    FLASH,
//    S_PRF,
//    L_PRF,
    SH_IO,
    SH_DONE,
    SH_OUT_DONE,
    SH_IN_READY
}  state_t;

typedef struct __mb_sys__
{
    char f_name[16]; // 16
    uint32_t f_len;  // 4
    state_t state;   // 4
    uint32_t id;     // 4
    uint32_t l_bf;   // 4
    uint8_t bf[128]; //
    //uint8_t bf[216]; // 128 НЕ Увеличивать
                     // sizeof(mb_sys) должен быть не более 248 байт
} mb_sys;

typedef struct {
    uint8_t func;
    uint8_t addr;
    int len;
    uint8_t buff[64];
} q_rsp;

#define MB_SYS_HEAD_LEN (32)

#endif
