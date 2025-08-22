#ifndef _DFDATA_H_DEFINED
#define _DFDATA_H_DEFINED

#include "ff.h"
#include "diskio.h"
#include "xprintf.h"

#include <stdint.h>
#include "rtc_32f1.h"

#include "file_io.h"

#ifndef DECLARE_DATA                    // if DECLARE is not defined
	#define DECLARE_DATA  extern
#endif

#define MDAY 35
//
extern mb_sys mb;
extern uint16_t set_rtc[8];
//
//DECLARE_DATA RTCTIME rtc;
DECLARE_DATA int dummy;
//
DECLARE_DATA uint8_t time_korr;
DECLARE_DATA int prot_mk2;
//
DECLARE_DATA int ink_exit; 	//
DECLARE_DATA int blk_fwdg; 	//

DECLARE_DATA int Tzad; 	// Отклонение от 300
DECLARE_DATA int Vzad; 	// Отклонение от 300
DECLARE_DATA uint32_t tzakl;	// время закладки (начало инкубации в минутах)
DECLARE_DATA uint32_t tink;	    // время инкубации (минуты)
//DECLARE_DATA int t_ar; 	    // время для просмотра архива
DECLARE_DATA int Nt;		// номер датчика температуры
DECLARE_DATA int Nv;		// номер датчика влажности
//---------------------------
DECLARE_DATA int Thig; 	// Предельные отклонения
DECLARE_DATA int Tlov; 	//    по температуре
DECLARE_DATA int Vhig; 	//    и влажности
DECLARE_DATA int Vlov; 	// ОТ ЗАДАНИЯ в 0.1 град. (-12,7...12,7)
//---------------------------
DECLARE_DATA int e_blkerr; // (сек) Время блокировки сообщения об АВАРИИ
DECLARE_DATA int dTN;	// Крутизна характеристики нагрева
DECLARE_DATA int dTO;	// Крутизна характеристики охлаждения
DECLARE_DATA int dTV;	//
DECLARE_DATA int dtNag;	// Период охладителя
DECLARE_DATA int dtOhl;	// Период охладителя
DECLARE_DATA int dtUvl;	// Период увлажнителя
DECLARE_DATA int tUvlMax;	// (сек) Максимальное время подачи воды на увлажнитель
DECLARE_DATA int kT;		// (32 33) смещение по темрературе
DECLARE_DATA int kV;		// (34 35) смещение по влажности
//DECLARE_DATA int dtiT;	// Период опроса датчиков
DECLARE_DATA int st_pov; 	//
DECLARE_DATA int en_pov;	     // Разрешение управлять поворотом
DECLARE_DATA int u_pov;		// Разрешение управления поворотом   (БИТ)
DECLARE_DATA int e_dtpov; 	// (мин) Контрольное время между поворотами в минутах
DECLARE_DATA int e_tipT;	// если == 0 ds1624, ==1 SHT1x
DECLARE_DATA int Tmax; 	// Максимально-допустимая температура в камере
DECLARE_DATA int Vmax; 	// Максимально-допустимая влажность в камере
//DECLARE_DATA int dtDerror;	// (сек) Допустимое время на опрос датчиков
DECLARE_DATA int p_Door;	// (сек) Полярность датчика двери
DECLARE_DATA int e_dtregim;  // (мин) Время восстановления режима в камере при tink>tink_min
DECLARE_DATA int e_blksig;   // (сек) Время блокировки сигнала АВАРИЯ
DECLARE_DATA int e_zadupr;   // (сек) Задержка включения управления камерой
DECLARE_DATA int tink_min;	// (час) Время для выхода камеры в режим после закладки
DECLARE_DATA int dNob;    	// Тип датчика оборотов
DECLARE_DATA int Nobmin;
DECLARE_DATA int nomer;   	// Номер камеры
DECLARE_DATA int speed_idx;   	// Скорость обмена
DECLARE_DATA int speed;   	// Скорость обмена
//
DECLARE_DATA int m_Tsr;
DECLARE_DATA int m_Vsr;
//
DECLARE_DATA int Init;
DECLARE_DATA uint32_t en_Cool;

DECLARE_DATA volatile uint8_t rx_complete;
//
//DECLARE_DATA int Tpar[8];
///////////////////////////////////////////////////////////////////////////////////////
DECLARE_DATA unsigned int blksig;
//
DECLARE_DATA int nPar;
DECLARE_DATA int kPar;
DECLARE_DATA int nPar_max;
DECLARE_DATA int kl_korr;
//
DECLARE_DATA int32_t Ttek;
DECLARE_DATA int32_t Vtek;
DECLARE_DATA int32_t Tist;
DECLARE_DATA int32_t Vist;
DECLARE_DATA int32_t Nobv;

DECLARE_DATA uint8_t merc;
//
DECLARE_DATA uint8_t kl_nastr;
DECLARE_DATA uint8_t nastr;
DECLARE_DATA uint8_t ind;
DECLARE_DATA uint8_t cpar;
DECLARE_DATA uint8_t dkl;
//
//=======================
//DECLARE_DATA uint8_t u_pov;
DECLARE_DATA uint8_t goriz;
DECLARE_DATA uint8_t nazad;
DECLARE_DATA uint8_t vpered;
DECLARE_DATA uint8_t gorsost;
DECLARE_DATA uint8_t otkat;
DECLARE_DATA uint8_t naklon;
//======================
DECLARE_DATA uint8_t regimT;
DECLARE_DATA uint8_t regimV;
DECLARE_DATA uint32_t centr;
//
DECLARE_DATA uint32_t Pnag;
DECLARE_DATA uint32_t Pohl;
DECLARE_DATA uint32_t Puvl;
//
DECLARE_DATA uint8_t errds;
//
DECLARE_DATA uint8_t s_Door;
DECLARE_DATA uint8_t b_Door;
//
DECLARE_DATA uint32_t alarm_door;
DECLARE_DATA uint32_t m_sat;
DECLARE_DATA uint32_t m_led;
//DECLARE_DATA uint32_t su_pov;
//=========================================
// Упакованные биты состояния и ошибок
DECLARE_DATA struct {
volatile uint16_t eUpr	: 1; //= Sost^0;
volatile uint16_t eVent	: 1; //= Sost^1;
volatile uint16_t ePov	: 1; //= Sost^2;
volatile uint16_t eTlov	: 1; //= Sost^3;
volatile uint16_t eThig	: 1; //= Sost^4;
volatile uint16_t eVlov	: 1; //= Sost^5;
volatile uint16_t eVhig	: 1; //= Sost^6;
volatile uint16_t setT8	: 1; //= Sost^7;
volatile uint16_t Door	: 1; //= Sost^8;
volatile uint16_t Vflg	: 1; //= Sost^9;
volatile uint16_t TK	: 1; //= Sost^10;
volatile uint16_t pL	: 1; //= Sost^11;
volatile uint16_t pR	: 1; //= Sost^12;
volatile uint16_t eReg	: 1; //= Sost^13;
volatile uint16_t edT	: 1; //= Sost^14;
volatile uint16_t edV	: 1; //= Sost^15;
} Sost;
// Sost
#define Door    Sost.Door
#define pL      Sost.pL
#define pR      Sost.pR
#define edT     Sost.edT
#define edV     Sost.edV
#define eReg    Sost.eReg
#define eTlov   Sost.eTlov
#define eThig   Sost.eThig
#define eVlov   Sost.eVlov
#define eVhig   Sost.eVhig
#define eVent   Sost.eVent
#define setT8   Sost.setT8
#define eUpr    Sost.eUpr
#define TK      Sost.TK
#define ePov    Sost.ePov
#define Vflg    Sost.Vflg

int restore_data(int n);
int save_data(int n);
void viev_data(int n);
void edit_data(int n, int data);

//void init_ff(void);
void init_data(void);

uint32_t get_speed(uint8_t idx);

uint16_t modbus_rtu_calc_crc(const void* data, size_t size);

#endif // _DFDATA_H_DEFINED
