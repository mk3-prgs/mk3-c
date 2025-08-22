#define DECLARE_DATA
#include "dfdata.h"                // include all variables

#include <string.h>
#include "mtc16201/mtc16201.h"
//
int* d_tab[]=
{
(int*)&Tzad,     // 6  0
(int*)&Vzad,     // 7  1
(int*)&Thig,     // 8  2
(int*)&Tlov,     // 9  3
(int*)&Vhig,	 //10  4   и влажности
(int*)&Vlov,	 //11  5 ОТ ЗАДАНИЯ в 0.1 град. (-12,7...12,7)
(int*)&Tmax, 	 //12  6 Максимально-допустимая температура в камере (откл от 300)
(int*)&Vmax, 	 //13  7 Максимально-допустимая влажность в камере (откл от 300)
(int*)&dummy, 	 //14  8 время инкубации
(int*)&Nt,		 //15  9 номер датчика температуры
(int*)&Nv,	     //16 10 номер датчика влажности
(int*)&dTN,		 //17 11 Крутизна характеристики нагрева
(int*)&e_blkerr, //18 12 (сек) Время блокировки сообщения об АВАРИИ (не задействована)
(int*)&dTO,	     //19 13 Крутизна характеристики охлаждения
(int*)&dTV,	     //20 14 Крутизна характеристики увлажнения в сотых долях градуса
(int*)&dtOhl,	 //21 15 Период охладителя
(int*)&dtUvl,	 //22 16 Период увлажнителя
(int*)&tUvlMax,	 //23 17 (сек) Максимальное время подачи воды на увлажнитель
(int*)&kT,	     //24 18 (32 33) смещение по темрературе
(int*)&kV,	     //25 19 (34 35) смещение по влажности
(int*)&en_pov,	 //26 20 Разрешение управлять поворотом
(int*)&u_pov,    //27 21 не используется
(int*)&e_dtpov,  //28 22 (мин) Контрольное время между поворотами в минутах
(int*)&e_tipT,   //29 23 если == 0 ds1624, ==1 SHT1x
(int*)&p_Door,    //30 24 Полярность дискретных датчиков
(int*)&e_dtregim, //31 25 (мин) Время восстановления режима в камере при tink>tink_min
(int*)&e_blksig,  //32 26 (сек) Время блокировки сигнала АВАРИЯ
(int*)&e_zadupr,  //33 27 (сек) Задержка включения управления камерой
(int*)&tink_min,  //34 28 (час) Время для выхода камеры в режим после закладки
(int*)&dNob,      //35 29 Тип датчика оборотов
(int*)&Nobmin,    //36 30
(int*)&nomer,     //37 31 Номер камеры
(int*)&Init,      //38  !!! 32 не сохраняется
//===================================================================================
(int*)&speed_idx, //39 33
(int*)&prot_mk2,  //40 34
(int*)&m_sat,     //41 35
(int*)&m_led,     //42 36
(int*)&dummy, 	 //14  37 время инкубации
(int*)&dtNag,	 //21 38 Период нагревателя
(int*)&en_Cool,
//
//(int)&dtiT,	 //26 20 Период опроса датчиков
//(int)&dtDerror,  //30 24 (сек) Допустимое время на опрос датчиков
NULL
};

char* d_tab_text[]=
{
"Tzad",     "Зад. температура",
"Vzad",     "Зад. влажность  ",
"Thig",     "Откл.в+ по темп.",
"Tlov",     "Откл.в- по темп.",
"Vhig",     "Откл.в+ по влаж.",
"Vlov",     "Откл.в- по влаж.",
"Tmax",     "Макс. доп. темп.",
"Vmax",     "Макс. доп. влаж.",
"tink",     "Время инкубации ",
"Nt",       "Номер датч.темп.",
"Nv",       "Номер датч.влаж.",
"dTN",      "Крут.хар.нагрева",
"e_blkerr", "(сек)Время блк. ",
"dTO",      "Крут. хар. охл. ",
"dTV",      "Крут. хар. увл. ",
"dtOhl",    "Период охладит. ",
"dtUvl",    "Период увлажнит.",
"tUvlMax",  "Макс.время увл. ",
"kT",       "Смещ. по темп.  ",
"kV",       "Смещ. по влажн. ",
"en_pov",   "Разр. упр. пов. ",
"u_pov",    "Не используется ",
"e_dtpov",  "Контр.время пов.",
"e_tipT",   "Тип датч. темп. ",
"p_Door",   "Пол. датч. двери",
"e_dtregim","Время восст.реж.",
"e_blksig", "Время блок.АВАР.",
"e_zadupr", "Зад.вкл.упр.кам.",
"tink_min", "Время прогрева  ",
"dNob",     "Тип датч. об.   ",
"Nobmin",   "Мин. об. вентил.",
"nomer",    "Номер камеры    ",
"Init",     "Инициализация   ",
//===================================================================================
"speed_idx","Скорость обмена ",
"prot_mk2", "Протокол обмена ",
"m_sat",    "Контрастн. инд. ",
"m_led",    "Яркость подсвет.",
"tzakl",    "Вр. закладки    ",
"dtNag",    "Период нагрева  ",
"en_Cool",  "Режим охлаждения",
NULL, NULL
};

static uint32_t speed_tab[]=
{
1200,   //0
2400,   //1
4800,   //2
9600,   //3
19200,  //4
38400,  //5
57600,  //6
115200,  //7
1152000  //8
};

uint32_t get_speed(uint8_t idx)
{
    if(idx>8) idx=8;
    return(speed_tab[idx]);
}

void init_data(void)
{
Tzad = 3780;
Vzad = 2900;
Thig =   5;
Tlov =  -5;
Vhig =   20;
Vlov =  -20;
Tmax = 3830;
Vmax = 3500;
//--------------------------
Nt=       7;
Nv=       7;
//--------------------------
dTN     = 50;
dTO     = 100;
dTV     = 100;
dtNag   =  2000;
dtOhl   = 20000;
dtUvl   = 20000;
tUvlMax = 5000;
//
kT=	    0;
kV=	    0;
//
en_pov = 1;
u_pov  = 0;
e_dtpov= 65;
//
e_tipT = 0;
p_Door = 1;
//
e_blkerr  = 30;
e_dtregim = 60;
e_blksig  = 120;
e_zadupr  = 60;
tink_min  = 5;
//
dNob=    	4; //4;
Nobmin=     280; //280;
nomer=   	1;
speed_idx= 	5;
Init = 0;
prot_mk2 = 1;
m_sat=9500;
m_led=5000;
en_Cool=0;
}

/*
void init_ff(void)
{
    disk_initialize(0);
    f_mount(&FatFs[0], "0:", 0);
}
*/

int length_data(void)
{
    return((sizeof(d_tab) / sizeof(int))-1);
}

void edit_data(int n, int data)
{
int *pt;
//
    if(n >= length_data()) n = length_data()-1;
    else if(n<0) n=0;
    //
    pt = d_tab[n];
    *pt = data;
    xprintf("%s: %d\n", d_tab_text[2*n+1], *pt);
}

void viev_data(int n)
{
int i;
int d;
int* ptr=NULL;
char* name=NULL;
//
    if(n<0) {
        for(i=0; i<length_data(); i++) {
            ptr = (int*)d_tab[i];
            d = *ptr;
            name = d_tab_text[2*i + 1];
            xprintf("%02d %s: %d\n", i, name, d);
            }
        }
    else if(n < length_data()) {
        ptr = (int*)d_tab[n];
        d = *ptr;
        name = d_tab_text[2*n + 1];
        xprintf("%02d %s: %d\n", n, name, d);
        }
//
}

int restore_data(int n)
{
FIL file;
FIL* fp=&file;
int res=1;
UINT cnt=0;
//
char f_name[16];
int32_t *bf=NULL;
int i;
int* ptr;
uint16_t crc;
uint16_t crc_r;
int tab_len = length_data();
    //
    bf = pvPortMalloc(64*sizeof(int32_t));
    if(bf) {
        xsprintf(f_name, "p%02d.prf", n);
        res = f_open(fp, f_name, FA_READ);
        if(!res) {
            f_read(fp, bf, (tab_len+1)*sizeof(int), &cnt);
            f_close(fp);
            //
            for(i=0; i<tab_len; i++) {
                ptr = (int*)d_tab[i];
                *ptr = bf[i];
                }
            //
            crc = modbus_rtu_calc_crc(bf, tab_len*sizeof(int));
            //xprintf("cnt=%d crc=%04x\n", cnt, crc);
            crc_r = bf[tab_len];
            if(crc != crc_r) res=1;
            }
        vPortFree(bf);
        }
    //
    return(res);
}

int save_data(int n)
{
FIL file;
FIL* fp=&file;
int res=0;
UINT cnt=0;
char f_name[16];
int32_t *bf;
int i;
int* ptr;
int tab_len = length_data();
//
    bf = pvPortMalloc(64*sizeof(int32_t));
    if(bf) {
        xsprintf(f_name, "p%02d.prf", n);
        f_unlink(f_name);
        //
        res = f_open(fp, f_name, FA_CREATE_ALWAYS | FA_WRITE);
        if(!res) {
            for(i=0; i<tab_len; i++) {
                ptr = (int*)d_tab[i];
                bf[i] = *ptr;
                }
            uint16_t crc = modbus_rtu_calc_crc(bf, tab_len*sizeof(int));
            bf[tab_len] = crc;
            //
            f_write(fp, bf, (tab_len+1)*sizeof(int), &cnt);
            //xprintf("cnt=%d crc=%04x\n", cnt, crc);
            //
            f_close(fp);
            }
        vPortFree(bf);
        }
    //
    return(res);
}

/*
// Table of CRC values for high–order byte
static const uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

// Table of CRC values for low–order byte
static const uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};

uint16_t modbus_rtu_calc_crc(const void* data, size_t size)
{
    uint8_t crc_hi = 0xff;
    uint8_t crc_lo = 0xff;

    size_t index = 0;

    for(; size != 0; size --){
        index = crc_lo ^ *(uint8_t*)data ++;
        crc_lo = crc_hi ^ table_crc_hi[index];
        crc_hi = table_crc_lo[index];
    }
    return ((uint16_t)crc_hi << 8) | crc_lo;
}
*/
