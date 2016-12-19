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

#include <cupkee.h>
#include "util.h"

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

void devices_event_post(uint8_t id, uint8_t type)
{
    event_put(EVENT_MAKE_PARAM2(EVENT_DEVICE, id, type));
}

void systick_event_post(void)
{
    event_put(EVENT_MAKE(EVENT_SYSTICK));
}

void shell_event_post(int type)
{
    event_put(EVENT_MAKE_PARAM(EVENT_SHELL, type));
}

