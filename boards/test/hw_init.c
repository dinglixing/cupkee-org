
/*******************************************************************************
 * hw field
*******************************************************************************/
#include <bsp.h>


/*******************************************************************************
 * dbg field
*******************************************************************************/
#include "hardware.h"

#define MEMORY_SIZE     (32 * 1024)

static char memory_buf[MEMORY_SIZE];
static int  memory_alloced = 0;

static void hw_dbg_memory_reset(void)
{
    memset(memory_buf, 0, MEMORY_SIZE);
    memory_alloced = 0;
}

void hw_dbg_set_systicks(uint32_t x)
{
    system_ticks_count = x;
}

void hw_dbg_reset(void)
{
    hw_dbg_set_systicks(0);

    hw_dbg_memory_reset();

    hw_dbg_console_reset();

    hw_scripts_erase();
}

/*******************************************************************************
 * bsp interface
*******************************************************************************/
uint32_t system_ticks_count = 0;

void hw_setup(void)
{
    hw_gpio_setup();
    hw_adc_setup();
    hw_usart_setup();
}

void hw_poll(void)
{
    static uint32_t system_ticks_count_pre = 0;

    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;
        systick_event_post();
    }

    hw_gpio_poll();
    hw_adc_poll();
    hw_usart_poll();
}

void hw_halt(void)
{
    printf("\nSystem into halt!\n");
    while(1)
        ;
}

void hw_info_get(hw_info_t * info)
{
    (void) info;

    return;
}

int hw_memory_alloc(void **p, int size, int align)
{
    int start = (intptr_t) memory_buf + memory_alloced;
    int shift = 0;

    if (size == 0) {
        return 0;
    }

    if (start % align) {
        shift = align - (start % align);
    } else {
        shift = 0;
    }
    memory_alloced += shift;


    if (size < 0) {
        size = MEMORY_SIZE - memory_alloced;
    } else
    if (memory_alloced + size > MEMORY_SIZE) {
        return -1;
    }

    if (p) {
        *p = memory_buf + memory_alloced;
        memory_alloced += size;
    }

    return size;
}

void hw_led_on(void)
{}

void hw_led_off(void)
{}

void hw_led_toggle(void)
{}


