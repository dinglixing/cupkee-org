
#ifndef __CUPKEE_EVENT_INC__
#define __CUPKEE_EVENT_INC__

enum {
    EVENT_SYSTICK = 0,
    EVENT_DEVICE  = 1,
    EVENT_USER    = 32,
};

enum {
    EVENT_DEVICE_ERR = 0,
    EVENT_DEVICE_DATA,
    EVENT_DEVICE_DRAIN,
    EVENT_DEVICE_READY,
    EVENT_DEVICE_MAX
};

typedef struct event_info_t {
    uint8_t type;
    uint8_t which;
    uint16_t code;
} event_info_t;

typedef int (*cupkee_event_handle_t)(event_info_t *);

void cupkee_event_init(void);
int  cupkee_event_take(event_info_t *event);
int  cupkee_event_post(uint8_t type, uint8_t which, uint16_t code);

static inline
int cupkee_event_post_systick(void) {
    return cupkee_event_post(EVENT_SYSTICK, 0, 0);
}

static inline
int cupkee_event_post_device(uint8_t which, uint16_t code) {
    return cupkee_event_post(EVENT_DEVICE, which, code);
}

static inline
int cupkee_event_post_device_error(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_ERR);
}

static inline
int cupkee_event_post_device_data(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_DATA);
}

static inline
int cupkee_event_post_device_drain(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_DRAIN);
}

static inline
int cupkee_event_post_device_ready(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_READY);
}

#endif /* __CUPKEE_EVENT_INC__ */

