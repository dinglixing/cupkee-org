/* Linker script for an STM32F107VC board (256KB flash, 64KB RAM). */

/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 256K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Include the common ld script. */
INCLUDE libopencm3_stm32f1.ld

