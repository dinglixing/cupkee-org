
#ifndef __EVENT_INC__
#define __EVENT_INC__

#define EVENTQ_SIZE 16

typedef enum event_type_t{
    EVENT_IDLE    = 0,
    EVENT_MASK    = 0xff,
    EVENT_SHELL   = 1,
    EVENT_SYSTICK = 2,
    EVENT_DEVICE  = 3
}event_type_t;

#define EVENT_MAKE(t)                       ((t) << 24)
#define EVENT_MAKE_PARAM(t, v)              (((t) << 24) + (v))
#define EVENT_MAKE_PARAM2(t, v1, v2)        (((t) << 24) + ((v1) << 16) + (v2))
#define EVENT_MAKE_PARAM3(t, v1, v2, v3)    (((t) << 24) + ((v1) << 16) + ((v2) << 8) + (v3))

#define EVENT_TYPE(e)                       (((e) >> 24) & EVENT_MASK)
#define EVENT_PARAM(e)                      ((e) & 0xffffff)
#define EVENT_PARAM1(e)                     (((e) >> 16) & 0xff)
#define EVENT_PARAM2(e)                     ((e) & 0xffff)

void event_init(void);
int event_put(int e);
int event_get(void);

#endif /* __EVENT_INC__ */

