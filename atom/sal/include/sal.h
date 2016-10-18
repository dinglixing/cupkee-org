
#ifndef __SAL_INC__
#define __SAL_INC__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <hal.h>

#define EVENTQ_SIZE 16
typedef enum event_t{
    EVENT_IDLE = 0,
    EVENT_CONSOLE_READY,
    EVENT_CONSOLE_INPUT,
    EVENT_SYSTICK_OCCUR,
}event_t;


int sal_init(void);
void sal_loop(void);

int event_put(int e);
int event_get(void);

#include "console.h"
#include "storage.h"

#endif /* __SAL_INC__ */

