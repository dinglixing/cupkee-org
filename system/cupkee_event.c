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

#define EVENTQ_SIZE     16

static rbuff_t eventq;
static event_info_t eventq_mem[EVENTQ_SIZE];

void cupkee_event_init(void)
{
    rbuff_init(&eventq, EVENTQ_SIZE);
}

int cupkee_event_post(uint8_t type, uint8_t which, uint16_t code)
{
    int pos = rbuff_push(&eventq);
    if (pos < 0) {
        return 0;
    }

    eventq_mem[pos].type  = type;
    eventq_mem[pos].which = which;
    eventq_mem[pos].code  = code;

    return 1;
}

int cupkee_event_take(event_info_t *e)
{
    int pos = rbuff_shift(&eventq);
    if (pos < 0) {
        return 0;
    }

    *e = eventq_mem[pos];

    return 1;
}

