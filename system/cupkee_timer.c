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

#include <cupkee.h>
#define TIMER_FL_REPEAT 1

static cupkee_timer_t *timer_head = NULL;
static int timer_next = 0;

static int timer_clear_by(int (*fn)(cupkee_timer_t *, int), int x)
{
    cupkee_timer_t *prev = NULL, *curr = timer_head;
    int n = 0;

    while (curr) {
        cupkee_timer_t *next = curr->next;

        if (fn(curr, x)) {
            if (prev) {
                prev->next = next;
            } else {
                timer_head = next;
            }

            curr->handle(1, curr->param); // drop timer
            cupkee_free(curr);
            n ++;
        } else {
            prev = curr;
        }

        curr = next;
    }
    return n;
}

static int timer_with_flag(cupkee_timer_t *t, int flags)
{
    return t->flags == flags;
}

static int timer_with_id(cupkee_timer_t *t, int id)
{
    return t->id == id;
}

void cupkee_timer_init(void)
{
    timer_head = NULL;
    timer_next = 0;
}

void cupkee_timer_sync(uint32_t curr_ticks)
{
    cupkee_timer_t *prev = NULL, *curr = timer_head;

    while (curr) {
        cupkee_timer_t *next = curr->next;

        if (curr_ticks - curr->from >= curr->wait) {
            curr->handle(0, curr->param);       // wake up

            if (curr->flags & TIMER_FL_REPEAT) {
                curr->from = curr_ticks;
                prev = curr;
            } else {
                curr->handle(1, curr->param);   // drop
                if (prev) {
                    prev->next = next;
                } else {
                    // Current timer is header
                    timer_head = next;
                }
                cupkee_free(curr);
            }
        } else {
            prev = curr;
        }
        curr = next;
    }
}

cupkee_timer_t *cupkee_timer_register(uint32_t wait, int repeat, cupkee_timer_handle_t handle, void *param)
{
    cupkee_timer_t *t;

    if (!handle) {
        return NULL;
    }

    t = cupkee_malloc(sizeof(cupkee_timer_t));
    if (t) {
        t->handle = handle;
        t->param  = param;
        t->id     = timer_next++;
        t->wait   = wait;
        t->from   = _cupkee_systicks;
        t->flags  = repeat ? TIMER_FL_REPEAT : 0;

        t->next = timer_head;
        timer_head = t;
    }

    return t;
}

void cupkee_timer_unregister(cupkee_timer_t *t)
{
    cupkee_timer_t *prev = NULL, *curr = timer_head;

    while (curr) {
        if (curr == t) {
            if (prev) {
                prev->next = curr->next;
            } else {
                timer_head = curr->next;
            }

            curr->handle(1, curr->param); // drop timer
            cupkee_free(curr);

            return;
        }

        prev = curr;
        curr = curr->next;
    }
}

int cupkee_timer_clear_all(void)
{
    cupkee_timer_t *curr = timer_head;
    int n = 0;

    timer_head = NULL;
    while (curr) {
        cupkee_timer_t *next = curr->next;

        curr->handle(1, curr->param); // drop timer
        cupkee_free(curr);

        curr = next;
        n ++;
    }

    return n;
}

int cupkee_timer_clear_with_flags(uint32_t flags)
{
    return timer_clear_by(timer_with_flag, flags);
}

int cupkee_timer_clear_with_id(uint32_t id)
{
    return timer_clear_by(timer_with_id, id);
}

volatile uint32_t _cupkee_systicks;

