/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

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

#include "cupkee.h"
#include "rbuff.h"

#define EVENTQ_SIZE         16
#define EMITTER_CODE_MAX    65535

static rbuff_t eventq;
static cupkee_event_t eventq_mem[EVENTQ_SIZE];
static cupkee_event_emitter_t *emitter_head;
static unsigned emitter_next;

void cupkee_event_setup(void)
{
    rbuff_init(&eventq, EVENTQ_SIZE);
    emitter_head = NULL;
    emitter_next = 0;
}

void cupkee_event_reset(void)
{
    rbuff_reset(&eventq);
}

int cupkee_event_emitter_init(cupkee_event_emitter_t *emitter, cupkee_event_emitter_handle_t handle)
{
    if (!emitter) {
        return -CUPKEE_EINVAL;
    }

    if (emitter_next >= EMITTER_CODE_MAX) {
        return -CUPKEE_ERESOURCE;
    }

    emitter->handle = handle;
    emitter->id   = emitter_next++;
    emitter->next = emitter_head;

    emitter_head = emitter;

    return emitter->id;
}

int cupkee_event_emitter_deinit(cupkee_event_emitter_t *emitter)
{
    cupkee_event_emitter_t *prev, *curr;

    if (!emitter || !emitter_head) {
        return -CUPKEE_EINVAL;
    }

    if (emitter_head == emitter) {
        emitter_head = emitter->next;
        return CUPKEE_OK;
    } else {
        prev = emitter_head;
        curr = emitter_head->next;
    }

    while (curr) {
        if (curr == emitter) {
            prev->next = curr->next;
            return CUPKEE_OK;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    return -CUPKEE_EINVAL;
}

void cupkee_event_emitter_dispatch(uint16_t which, uint8_t code)
{
    cupkee_event_emitter_t *emitter = emitter_head;

    while (emitter) {
        if (emitter->id == which) {
            if (emitter->handle) emitter->handle(emitter, code);
            return;
        }
        emitter = emitter->next;
    }
}

int cupkee_event_post(uint8_t type, uint8_t code, uint16_t which)
{
    uint32_t state;
    int pos;

    hw_enter_critical(&state);
    pos = rbuff_push(&eventq);
    hw_exit_critical(state);

    if (pos < 0) {
        return 0;
    }

    eventq_mem[pos].type  = type;
    eventq_mem[pos].code  = code;
    eventq_mem[pos].which = which;

    return 1;
}

int cupkee_event_take(cupkee_event_t *e)
{
    uint32_t state;
    int pos;

    hw_enter_critical(&state);
    pos = rbuff_shift(&eventq);
    hw_exit_critical(state);

    if (pos < 0) {
        return 0;
    }

    *e = eventq_mem[pos];

    return 1;
}

