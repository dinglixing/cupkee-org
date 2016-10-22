#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cupkee.h>

#include "misc.h"
#include "event.h"
#include "shell.h"
#include "console.h"
#include "timeout.h"

static const char *logo = "\
 _________               __                  \r\n\
 \\_   ___ \\ __ ________ |  | __ ____   ____  \r\n\
 /    \\  \\/|  |  \\____ \\|  |/ // __ \\_/ __ \\ \r\n\
 \\     \\___|  |  /  |_> >    <\\  ___/\\  ___/ \r\n\
  \\________/____/|   __/|__|_ \\\\____> \\____>\r\n\
                 |__|        \\/ ATOM v0.0.1\r\n";


#define HEAP_SIZE     (1024 * 8)
#define STACK_SIZE    (256)
#define EXE_MEM_SPACE (1024 * 8)
#define SYM_MEM_SPACE (1024 * 2)
#define MEM_SIZE      (STACK_SIZE * sizeof(val_t) + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)


static uint8_t memory[MEM_SIZE];
static env_t shell_env;


//static uint32_t system_ticks_count_pre = 0;
static void sal_poll(void)
{
    static uint32_t system_ticks_count_pre = 0;
    int e;

    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;
        event_put(EVENT_SYSTICK_OCCUR);
    }

    while (EVENT_IDLE != (e = event_get())) {
        switch (e) {
        case EVENT_CONSOLE_READY:
            hal_console_sync_puts(logo);
            break;
        case EVENT_CONSOLE_INPUT:
            shell_execute(&shell_env);
            break;
        case EVENT_SYSTICK_OCCUR:
            timeout_execute(&shell_env);
            break;
        default:
            break;
        }
    }
}

int cupkee_init(void)
{
    board_setup();

    event_init();

    console_init();

    /* Initialise evn */
    if(0 != interp_env_init_interactive(&shell_env, memory, MEM_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE)) {
        return -1;
    }

    /* Initialise reference */
    reference_init(&shell_env);

    return 0;
}

int cupkee_set_native(const native_t *entry, int n)
{
    /* Initialise all native functions */
    return env_native_set(&shell_env, entry, n);
}

int cupkee_loop(void)
{
    /* */
    timeout_init(&shell_env);

    shell_init(&shell_env);

    /* forever */
    while (1) {
        hal_loop();
        sal_poll();
    }

    /* Never go here! */
    return 0;
}

