#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#include "misc.h"
#include "shell.h"
#include "console.h"
#include "device.h"
#include "timeout.h"

static env_t core_env;

#define MEM_BLOCK_SIZE  256
static void memory_distribution(
        void **core_mem_ptr,
        int *core_mem_size,
        int *heap_mem_size,
        int *stack_mem_size,

        void **shell_mem_ptr,
        int *shell_mem_size
) {
    void *memory;
    int size, blocks;

    size = hw_memory_alloc(&memory, -1, 16);
    if (size < 1024 * 8) {
        // memory not enought !
        hw_halt();
    }

    blocks = size / MEM_BLOCK_SIZE;
    *core_mem_size  = (blocks * 7 / 8) * MEM_BLOCK_SIZE;
    *heap_mem_size  = (blocks * 3 / 8) * MEM_BLOCK_SIZE;
    *stack_mem_size = (blocks / 8) * MEM_BLOCK_SIZE;
    *shell_mem_size = size - *core_mem_size;

    *core_mem_ptr  = memory;
    *shell_mem_ptr = memory + *core_mem_size;
}

static void core_init(void *memory, int size, int stack_mem_size, int heap_mem_size)
{
    if(0 != interp_env_init_interactive(&core_env, memory, size,
                                        NULL, heap_mem_size,
                                        NULL, stack_mem_size / sizeof(val_t))) {
        hw_halt();
    }

    event_init();
}

static void event_process(void)
{
    int e;

    while (EVENT_IDLE != (e = event_get())) {
        switch (EVENT_TYPE(e)) {
        case EVENT_SHELL:   shell_event_proc(&core_env, e);     break;
        case EVENT_SYSTICK: systick_event_proc(&core_env, e);   break;
        case EVENT_DEVICE:  device_event_proc(&core_env, e);       break;
        default:
            break;
        }
    }
}

int cupkee_init(void)
{
    void *core_mem, *shell_mem;
    int   core_mem_sz, shell_mem_sz, heap_mem_sz, stack_mem_sz;

    /* initial hardware */
    hw_setup();

    memory_distribution(
            &core_mem, &core_mem_sz,
            &heap_mem_sz, &stack_mem_sz,
            &shell_mem, &shell_mem_sz);

    /* Initial console reources */
    console_init();

    /* Initial memory env, etc. */
    core_init(core_mem, core_mem_sz, stack_mem_sz, heap_mem_sz);

    /* Initial reference */
    reference_init(&core_env);

    /* Initial timeout resource */
    timeout_init();

    /* Initial devices resource */
    device_init();

    /* Initial shell resource */
    shell_init(&core_env, shell_mem, shell_mem_sz);

    return 0;
}

int cupkee_set_native(const native_t *entry, int n)
{
    /* Initialise all native functions */
    return env_native_set(&core_env, entry, n);
}

int cupkee_start(const char *scripts)
{
    // Start shell & execute initial scripts
    return shell_start(scripts);
}

int cupkee_poll(void)
{
    hw_poll();

    device_poll();

    event_process();

    return 0;
}

