#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal.h"
#include "sal.h"

#include "rbuff.h"

static rbuff_t eventq;
static int eventq_mem[EVENTQ_SIZE];

static void event_queue_init(void)
{
    rbuff_init(&eventq, EVENTQ_SIZE, eventq_mem);
}

int event_put(int e)
{
    int pos = rbuff_push(&eventq);
    if (pos < 0) {
        return 0;
    }

    eventq_mem[pos] = e;
    return 1;
}

int event_get(void)
{
    int pos = rbuff_shift(&eventq);
    if (pos < 0) {
        return EVENT_IDLE;
    }

    return eventq_mem[pos];
}

static uint32_t system_ticks_count_pre = 0;
int sal_init(void)
{
    board_setup();

    event_queue_init();

    if (storage_init()) {
        return -1;
    }

    if (console_init()) {
        return -1;
    }

    system_ticks_count_pre = system_ticks_count;

    return 0;
}

void sal_loop(void)
{
    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;
        event_put(EVENT_SYSTICK_OCCUR);
    }

    hal_loop();
}

