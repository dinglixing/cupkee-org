/*
MIT License

This file is part of cupkee project.

Copyright (C) 2017 Lixing Ding <ding.lixing@gmail.com>

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

#ifndef __CUPKEE_TIMER_INC__
#define __CUPKEE_TIMER_INC__

typedef void (*cupkee_timer_handle_t)(int drop, void *param);
typedef struct cupkee_timer_t {
    struct cupkee_timer_t *next;
    cupkee_timer_handle_t handle;
    uint32_t flags;
    uint32_t wait;
    uint32_t from;
    void    *param;
} cupkee_timer_t;

void cupkee_timer_init(void);
void cupkee_timer_sync(uint32_t ticks);

cupkee_timer_t *cupkee_timer_register(uint32_t wait, int repeat, cupkee_timer_handle_t handle, void *param);
void cupkee_timer_unregister(cupkee_timer_t *t);

void cupkee_timer_clear_all(void);
void cupkee_timer_clear_with_flags(uint32_t flags);

#endif /* __CUPKEE_TIMER_INC__ */

