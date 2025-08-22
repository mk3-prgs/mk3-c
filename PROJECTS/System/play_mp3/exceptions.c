/*************************************************************************************************
 * @file		exceptions.c
 *
 * @brief		Модуль обработки исключений
 *
 * @version		v1.0
 * @date		03.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/


//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"


//*-----------------------------------------------------------------------------------------------
/**			Инициализация																		*/
//*-----------------------------------------------------------------------------------------------
void hard_fault_ini(void)
{
	 SCB->CCR |= 0x18;                // enable div-by-0 and unaligned fault
	 SCB->SHCSR |= 0x00007000;        // enable Usage Fault, Bus Fault, and MMU Fault
}


//*-----------------------------------------------------------------------------------------------
/**			Первичный обработчик исключения HardFault											*/
//*-----------------------------------------------------------------------------------------------
__attribute__((naked)) void HardFault_Handler(void)
{
	__asm volatile
	(
		"TST LR, #4					\n"
		"ITE EQ						\n"
		"MRSEQ R0, MSP				\n"
		"MRSNE R0, PSP				\n"
		"B hard_fault_handler_c		\n"
	);
}


//*-----------------------------------------------------------------------------------------------
/**			Обработчик исключения
 *
 * @param hardfault_args - указатель на стек с сохранённым контекстом							*/
//*-----------------------------------------------------------------------------------------------
void hard_fault_handler_c (uint32_t * hardfault_args)
{
	volatile uint32_t stacked_r0 __attribute__((unused));
	volatile uint32_t stacked_r1 __attribute__((unused));
	volatile uint32_t stacked_r2 __attribute__((unused));
	volatile uint32_t stacked_r3 __attribute__((unused));
	volatile uint32_t stacked_r12 __attribute__((unused));
	volatile uint32_t stacked_lr __attribute__((unused));
	volatile uint32_t stacked_pc __attribute__((unused));
	volatile uint32_t stacked_psr __attribute__((unused));

	volatile uint32_t BFAR __attribute__((unused));	// Bus Fault Address Register
	volatile uint32_t CFSR __attribute__((unused)); // Configurable Fault Status Register Consists of MMSR, BFSR and UFSR
	volatile uint32_t HFSR __attribute__((unused));	// Hard Fault Status Register
	volatile uint32_t DFSR __attribute__((unused));	// Debug Fault Status Register
	volatile uint32_t AFSR __attribute__((unused));	// Auxiliary Fault Status Register

    // Read the Fault Address Registers. These may not contain valid values.
    // Check BFARVALID/MMARVALID to see if they are valid values
    // MemManage Fault Address Register
	volatile uint32_t MMAR __attribute__((unused));

	volatile uint32_t SCB_SHCSR __attribute__((unused));

	stacked_r0 = hardfault_args[0];
	stacked_r1 = hardfault_args[1];
	stacked_r2 = hardfault_args[2];
	stacked_r3 = hardfault_args[3];
	stacked_r12 = hardfault_args[4];
	stacked_lr = hardfault_args[5];
	stacked_pc = hardfault_args[6];
	stacked_psr = hardfault_args[7];

	BFAR = (*((volatile unsigned long *)(0xE000ED38)));
	CFSR = (*((volatile unsigned long *)(0xE000ED28)));
	HFSR = (*((volatile unsigned long *)(0xE000ED2C)));
	DFSR = (*((volatile unsigned long *)(0xE000ED30)));
	AFSR = (*((volatile unsigned long *)(0xE000ED3C)));
	MMAR = (*((volatile unsigned long *)(0xE000ED34)));
	SCB_SHCSR = SCB->SHCSR;

//	DBG_HALT(1);

	 xprintf ("\n\n[Hard fault handler - all numbers in hex]\n");
	 xprintf ("R0 = %x\n", stacked_r0);
	 xprintf ("R1 = %x\n", stacked_r1);
	 xprintf ("R2 = %x\n", stacked_r2);
	 xprintf ("R3 = %x\n", stacked_r3);
	 xprintf ("R12 = %x\n", stacked_r12);
	 xprintf ("LR [R14] = %x  subroutine call return address\n", stacked_lr);
	 xprintf ("PC [R15] = %x  program counter\n", stacked_pc);
	 xprintf ("PSR = %x\n", stacked_psr);
	 xprintf ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));
	 xprintf ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));
	 xprintf ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));
	 xprintf ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));
	 xprintf ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));
	 xprintf ("SCB_SHCSR = %x\n", SCB->SHCSR);

	while (1);
}
