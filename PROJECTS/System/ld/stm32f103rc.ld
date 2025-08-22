/*
Linker script for STM32F10x
Copyright RAISONANCE 2007 (modified by Lanchon 1-Feb-2008)
You can use, copy and distribute this file freely, but without any waranty.
Configure memory sizes, end of stack and boot mode for your project here.
*/

/* include the common STM32F10x sub-script */
INCLUDE "stm32_common.ld"

/* Memory Spaces Definitions */
MEMORY
{
  XRAM    (xrw) : ORIGIN = 0x2000c000, LENGTH = 16K
  RAM     (xrw) : ORIGIN = 0x20000000, LENGTH = 48K
  FLASH    (rx) : ORIGIN = 0x08000000, LENGTH = 256K
}

/* highest address of the user mode stack */
_estack = 0x20000000 + 48K;

/* include the section management sub-script */
/* (either "stm32_sec_flash.ld" or "stm32_sec_ram.ld") */
INCLUDE "stm32_sec_flash.ld"
