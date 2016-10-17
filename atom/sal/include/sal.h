
#ifndef __SAL_INC__
#define __SAL_INC__

#include <stdint.h>
#include <hal.h>

#define EVENTQ_SIZE 16
typedef enum event_t{
    EVENT_IDLE = 0,
    EVENT_CONSOLE_READY,
    EVENT_CONSOLE_INPUT,
    EVENT_SYSTICK_OCCUR,
}event_t;

#define CON_PREVENT_DEFAULT 1
typedef enum console_ctrl_t{
    CON_CTRL_IDLE = 0,
    CON_CTRL_CHAR = 1,
    CON_CTRL_BACKSPACE,
    CON_CTRL_DELETE,
    CON_CTRL_TABLE,
    CON_CTRL_ENTER,
    CON_CTRL_ESCAPE,
    CON_CTRL_UP,
    CON_CTRL_DOWN,
    CON_CTRL_RIGHT,
    CON_CTRL_LEFT
} console_ctrl_t;


int sal_init(void);
void sal_loop(void);

int event_put(int e);
int event_get(void);

#include "console.h"

#endif /* __SAL_INC__ */

