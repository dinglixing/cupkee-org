
#ifndef __EVENT_INC__
#define __EVENT_INC__

#define EVENTQ_SIZE 16

typedef enum event_t{
    EVENT_IDLE = 0,
    EVENT_CONSOLE_READY,
    EVENT_CONSOLE_INPUT,
    EVENT_SYSTICK_OCCUR,
}event_t;

void event_init(void);
int event_put(int e);
int event_get(void);

#endif /* __EVENT_INC__ */

