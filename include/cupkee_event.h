/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __CUPKEE_EVENT_INC__
#define __CUPKEE_EVENT_INC__

enum {
    EVENT_SYSTICK = 0,
    EVENT_DEVICE  = 1,
    EVENT_EMITTER = 2,
};

enum {
    EVENT_DEVICE_ERR = 0,
    EVENT_DEVICE_DATA,
    EVENT_DEVICE_DRAIN,
    EVENT_DEVICE_READY,
    EVENT_DEVICE_MAX
};

typedef struct cupkee_event_t {
    uint8_t type;
    uint8_t which;
    uint16_t code;
} cupkee_event_t;

typedef struct cupkee_event_emitter_t cupkee_event_emitter_t;
typedef int (*cupkee_event_handle_t)(cupkee_event_t *);
typedef void (*cupkee_event_emitter_handle_t)(cupkee_event_emitter_t *emitter, uint8_t code);

struct cupkee_event_emitter_t {
    cupkee_event_emitter_t *next;
    cupkee_event_emitter_handle_t handle;
    uint16_t code;
};

void cupkee_event_setup(void);
void cupkee_event_reset(void);

int cupkee_event_emitter_init(cupkee_event_emitter_t *emitter, cupkee_event_emitter_handle_t handle);
int cupkee_event_emitter_deinit(cupkee_event_emitter_t *emitter);

int cupkee_event_post(uint8_t type, uint8_t which, uint16_t code);
int cupkee_event_take(cupkee_event_t *event);

void cupkee_event_emitter_dispatch(uint8_t which, uint16_t code);

static inline int cupkee_event_emitter_emit(cupkee_event_emitter_t *emitter, uint8_t which) {
    return cupkee_event_post(EVENT_EMITTER, which, emitter->code);
}

static inline int cupkee_event_post_systick(void) {
    return cupkee_event_post(EVENT_SYSTICK, 0, 0);
}

static inline int cupkee_event_post_device(uint8_t which, uint16_t code) {
    return cupkee_event_post(EVENT_DEVICE, which, code);
}

static inline int cupkee_event_post_device_error(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_ERR);
}

static inline int cupkee_event_post_device_data(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_DATA);
}

static inline int cupkee_event_post_device_drain(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_DRAIN);
}

static inline int cupkee_event_post_device_ready(uint8_t which) {
    return cupkee_event_post(EVENT_DEVICE, which, EVENT_DEVICE_READY);
}

#endif /* __CUPKEE_EVENT_INC__ */

