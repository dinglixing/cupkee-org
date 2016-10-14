
#ifndef __SAL_INC__
#define __SAL_INC__

#include <stdint.h>
#include <hal.h>

enum {
    EVENT_IDLE = 0,
    EVENT_CONSOLE_READY,
    EVENT_CONSOLE_INPUT,
    EVENT_SYSTICK_OCCUR,
};

#define EVENTQ_SIZE 16

int sal_init(void);
void sal_loop(void);

int event_put(int e);
int event_get(void);

#include "console.h"

#endif /* __SAL_INC__ */

