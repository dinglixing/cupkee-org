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

#include <cupkee.h>

#include "cupkee_shell_misc.h"
#include "cupkee_shell_systick.h"

#define TIMEOUT_MAX   (16)

typedef struct timeout_t {
    int repeat;
    uint32_t from;
    uint32_t wait;
    val_t   *handle_ref;
    struct timeout_t *next;
} timeout_t;

static timeout_t timeout_queue[TIMEOUT_MAX];
static timeout_t *timeout_free;
static timeout_t *timeout_wait;

static timeout_t *timeout_alloc(val_t *handle)
{
    timeout_t *to = timeout_free;

    if (to) {
        val_t *ref = shell_reference_create(handle);

        if (ref) {
            to->handle_ref = ref;
            timeout_free = to->next;
            return to;
        }
    }

    return to;
}

static void timeout_release(timeout_t *to)
{
    if (to) {
        if (to->handle_ref) {
            shell_reference_release(to->handle_ref);
            to->handle_ref = NULL;
        }
        to->next = timeout_free;
        timeout_free = to;
    }
}

static int timeout_register(int ac, val_t *av, int repeat)
{
    val_t   *handle;
    uint32_t wait;
    timeout_t *to;

    if (ac < 1 || !val_is_function(av)) {
        return -1;
    }
    handle = av++;

    if (ac > 1 && val_is_number(av)) {
        wait = val_2_double(av);
    } else {
        wait = 0;
    }

    to = timeout_alloc(handle);
    if (to) {
        to->from = cupkee_systicks();
        to->wait = wait;
        to->repeat = repeat;
        to->next = timeout_wait;
        timeout_wait = to;

        return to - timeout_queue;
    } else {
        return -1;
    }
}

static int timeout_unregister(int ac, val_t *av, int repeat)
{
    timeout_t *to = timeout_wait;
    timeout_t **prev = &timeout_wait;
    int drop = 0;
    int tid = -1; // all

    if (ac > 0) {
        if (val_is_number(av)) {
            tid = val_2_double(av);
        } else {
            return -1;
        }
    }

    while(to) {
        int should_release = 0;

        if (tid < 0) {
            if (!repeat == !to->repeat) {
                should_release = 1;
            }
        } else {
            if (to - timeout_queue == tid) {
                should_release = 1;
            }
        }

        if (should_release) {
            timeout_t *next = to->next;

            to    = next;
            *prev = next;

            timeout_release(to);
            drop ++;
        } else {
            prev = &to->next;
            to = to->next;
        }
    }

    return drop;
}

val_t native_set_timeout(env_t *env, int ac, val_t *av)
{
    int tid = timeout_register(ac, av, 0);

    (void) env;

    return tid < 0 ? val_mk_boolean(0) : val_mk_number(tid);
}

val_t native_set_interval(env_t *env, int ac, val_t *av)
{
    int tid = timeout_register(ac, av, 1);

    (void) env;

    return tid < 0 ? val_mk_boolean(0) : val_mk_number(tid);
}

val_t native_clear_timeout(env_t *env, int ac, val_t *av)
{
    int n = timeout_unregister(ac, av, 0);

    (void) env;

    return n < 0 ? val_mk_boolean(0) : val_mk_number(n);
}

val_t native_clear_interval(env_t *env, int ac, val_t *av)
{
    int n = timeout_unregister(ac, av, 1);

    (void) env;

    return n < 0 ? val_mk_boolean(0) : val_mk_number(n);
}

void shell_systick_init(void)
{
    int i;

    timeout_wait = NULL;
    timeout_free = NULL;
    for (i = 0; i < TIMEOUT_MAX; i++) {
        timeout_t *to = &timeout_queue[i];

        to->handle_ref = NULL;
        to->from= 0;
        to->wait = 0;
        to->repeat = 0;
        to->next = timeout_free;
        timeout_free = to;
    }
}

void shell_systick_handle(env_t *env, uint32_t cur_ticks)
{
    timeout_t *to = timeout_wait;
    timeout_t *tr = NULL;

    timeout_wait = NULL;
    while(to) {
        timeout_t *next = to->next;
        int repeat = 1;

        if (cur_ticks - to->from >= to->wait) {
            shell_do_callback(env, to->handle_ref, 0, NULL);

            if (to->repeat) {
                to->from = cur_ticks;
            } else {
                repeat = 0;
            }
        }

        if (repeat) {
            to->next = tr;
            tr = to;
        } else {
            timeout_release(to);
        }

        to = next;
    }

    if (timeout_wait) {
        to = timeout_wait;
        while(to->next) {
            to = to->next;
        }
        to->next = tr;
    } else {
        timeout_wait = tr;
    }
}

