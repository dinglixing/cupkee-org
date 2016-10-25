#include <bsp.h>
#include "hardware.h"

uint32_t system_ticks_count = 0;

void hw_systicks_set(uint32_t x)
{
    system_ticks_count = x;
}

void hw_reset(void)
{
    hw_systicks_set(0);

    hw_scripts_erase();
    hw_console_reset();
    //hw_scripts_reset();
}

