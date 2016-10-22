#include "rbuff.h"
#include "event.h"

static rbuff_t eventq;
static int eventq_mem[EVENTQ_SIZE];

void event_init(void)
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
