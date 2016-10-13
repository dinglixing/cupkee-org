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

    *RBUFF_ELEM_PTR(&eventq, int, pos) = e;
    return 1;
}

int event_get(void)
{
    int pos = rbuff_shift(&eventq);
    if (pos < 0) {
        return EVENT_IDLE;
    }

    return *RBUFF_ELEM_PTR(&eventq, int, pos);
}

int sal_init(void)
{
    event_queue_init();

    if (console_init()) {
        return -1;
    }

    return 0;
}

void sal_loop(void)
{
    hal_loop();
}

