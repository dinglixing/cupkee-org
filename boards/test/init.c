#include <bsp.h>
#include "hardware.h"

#define MEMORY_SIZE     (32 * 1024)

static char memory_buf[MEMORY_SIZE];
static int  memory_alloced = 0;

static void hw_memory_init(void)
{
    memset(memory_buf, 0, MEMORY_SIZE);
    memory_alloced = 0;
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

uint32_t system_ticks_count = 0;
void hw_systicks_set(uint32_t x)
{
    system_ticks_count = x;
}

void hw_reset(void)
{
    hw_systicks_set(0);

    hw_memory_init();
    hw_scripts_erase();
    hw_console_reset();

    //hw_scripts_reset();
}


