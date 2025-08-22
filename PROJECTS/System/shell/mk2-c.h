#ifndef _MK2_C_DEFINED
#define _MK2_C_DEFINED
//
#include <stdint.h>
//
#ifndef DECLARE     // if DECLARE is not defined
	#define DECLARE  extern
#endif
//
typedef struct
{
volatile uint32_t Bit0 : 1;
volatile uint32_t Bit1 : 1;
volatile uint32_t Bit2 : 1;
volatile uint32_t Bit3 : 1;
volatile uint32_t Bit4 : 1;
volatile uint32_t Bit5 : 1;
volatile uint32_t Bit6 : 1;
volatile uint32_t Bit7 : 1;
volatile uint32_t Bit8 : 1;
} spi_io;

DECLARE union
{
volatile uint8_t d;
spi_io   b;
} d_out;

DECLARE union
{
volatile uint8_t d;
spi_io   b;
} d_inp;

DECLARE int version;
DECLARE int revision;
/*
#define Alarm (d_out.b.Bit0)
#define Vent  (d_out.b.Bit1)
#define Zasl  (d_out.b.Bit2)
#define PovL  (d_out.b.Bit3)
#define PovR  (d_out.b.Bit4)
#define Uvl   (d_out.b.Bit5)
#define Ohl   (d_out.b.Bit6)
#define Nagr  (d_out.b.Bit7)
*/
// plata gd32
#define Alarm (d_out.b.Bit1)
#define Vent  (d_out.b.Bit0)
#define Zasl  (d_out.b.Bit7)
#define PovL  (d_out.b.Bit6)
#define PovR  (d_out.b.Bit5)
#define Uvl   (d_out.b.Bit4)
#define Ohl   (d_out.b.Bit3)
//
#define Nagr  (d_out.b.Bit2)
//#define Nagr1  (d_out.b.Bit7)
//
#define dPovR  (d_inp.b.Bit0)
#define dPovL  (d_inp.b.Bit1)
#define dDoor  (d_inp.b.Bit2)
#define dContr (d_inp.b.Bit3)
#define dPower (d_inp.b.Bit4)

#define dVent  (d_inp.b.Bit5)
//
#endif
