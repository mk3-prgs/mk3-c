#include "gd32f10x.h"
#include "usart.h"
#include "xprintf.h"
/*
.weak NMI_Handler
.thumb_set NMI_Handler,Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler,Default_Handler

.weak MemManage_Handler
.thumb_set MemManage_Handler,Default_Handler

.weak BusFault_Handler
.thumb_set BusFault_Handler,Default_Handler

.weak UsageFault_Handler
.thumb_set UsageFault_Handler,Default_Handler
*/

void NMI_Handler(void)
{
    usart_print("NMI_Handler!\n");
    while(1) {}
}

uint32_t psp;
uint32_t msp;

void HardFault_Handler(void)
{
    psp = __get_PSP();
    msp = __get_MSP();
    xprintf("HardFault_Handler!\npsp=0x%x msp=0x%x\n");
    while(1) {}
}

void MemManage_Handler(void)
{
    usart_print("MemManage_Handler!\n");
    while(1) {}
}

void BusFault_Handler(void)
{
    usart_print("BusFault_Handler!\n");
    while(1) {}
}

void UsageFault_Handler(void)
{
    usart_print("UsageFault_Handler!\n");
    while(1) {}
}

typedef struct __attribute__((packed))
{
    unsigned short    b15_0;
    unsigned short    b16_31;
    unsigned int    b32_63;
    unsigned int    b64_95;
} UNIQUE_ID;

const UNIQUE_ID    *id = (UNIQUE_ID*) 0x1FFFF7E8;

void uid(void)
{
unsigned int id_l, id_m, id_h;

    id_l = id->b15_0 | (id->b16_31 << 16);
    id_m = id->b32_63;
    id_h = id->b64_95;
    //
    xprintf("UID: 0x%08x%08x%08x\n", id_h, id_m, id_l);
}
