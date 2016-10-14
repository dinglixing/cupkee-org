/*
 * This file is part of the cupkee project.
 *
 * Copyright (C) 2016 Lixing ding <ding.lixing@gmail.conz>
 *
 */

#include "sal.h"
#include "shell.h"
#include "native.h"

#define HEAP_SIZE     (1024 * 8)
#define STACK_SIZE    (256)
#define EXE_MEM_SPACE (1024 * 8)
#define SYM_MEM_SPACE (1024 * 2)
#define MEM_SIZE      (STACK_SIZE * sizeof(val_t) + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)
#define TIMEOUT_MAX   (16)

typedef struct timeout_t {
    int repeat;
    uint32_t from;
    uint32_t wait;
    val_t   *handle;
    struct timeout_t *next;
} timeout_t;

static uint8_t memory[MEM_SIZE];
static env_t shell_env;

static timeout_t timeout_queue[TIMEOUT_MAX];
static timeout_t *timeout_free;
static timeout_t *timeout_wait;

static val_t timeout_handles[TIMEOUT_MAX];

static void timeout_init(env_t *env)
{
    int i;

    (void) env;

    timeout_wait = NULL;
    timeout_free = NULL;
    for (i = 0; i < TIMEOUT_MAX; i++) {
        timeout_t *to = &timeout_queue[i];

        to->handle = NULL;
        to->from= 0;
        to->wait = 0;
        to->repeat = 0;
        to->next = timeout_free;
        timeout_free = to;

        val_set_undefined(&timeout_handles[i]);
    }

    env_reference_set(env, timeout_handles, TIMEOUT_MAX);
}

static void timeout_release(timeout_t *to)
{
    if (to) {
        if (to->handle) {
            val_set_undefined(to->handle);
            to->handle = NULL;
        }
        to->next = timeout_free;
        timeout_free = to;
    }
}

static timeout_t *timeout_alloc(void)
{
    timeout_t *to = timeout_free;

    if (to) {
        int i;

        timeout_free = to->next;
        for (i = 0; i < TIMEOUT_MAX; i++) {
            if (val_is_undefined(timeout_handles + i)) {
                to->handle = timeout_handles + i;
                return to;
            }
        }

        // should not go here!!
        timeout_release(to);
    }

    return to;
}

int shell_init(void)
{
    if(0 != interp_env_init_interactive(&shell_env, memory, MEM_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE)) {
        //sal_console_output("env_init fail\n");
        return -1;
    }

    native_init(&shell_env);
    timeout_init(&shell_env);

    return 0;
}

void shell_execute(void)
{
    char buf[80];
    val_t *res;
    int    err;

    console_gets(buf, 80);

    err = interp_execute_interactive(&shell_env, buf, NULL, &res);
    if (err < 0) {
        print_error(-err);
    } else
    if (err > 0) {
        print_value(res);
    }
    console_puts("> ");

}

void shell_timeout_execute(void)
{
    uint32_t cur_ticks = system_ticks_count;
    timeout_t *to = timeout_wait;
    timeout_t **ref = &timeout_wait;

    while(to) {
        int should_release = 0;

        if (cur_ticks - to->from >= to->wait) {

            env_push_call_function(&shell_env, to->handle);
            interp_execute_call(&shell_env, 0);

            if (to->repeat) {
                to->from = cur_ticks;
            } else {
                should_release = 1;
            }
        }

        if (should_release) {
            timeout_t *next = to->next;

            timeout_release(to);

            to   = next;
            *ref = next;
        } else {
            ref = &to->next;
            to = to->next;
        }
    }
}

int shell_timeout_regiseter(uint32_t wait, val_t *handle, int repeat)
{
    timeout_t *to;

    if (!val_is_function(handle)) {
        return -1;
    }

    to = timeout_alloc();
    if (to) {
        to->from = system_ticks_count;
        to->wait = wait;
        to->repeat = repeat;
        *to->handle = *handle;
        to->next = timeout_wait;
        timeout_wait = to;

        return to - timeout_queue;
    } else {
        return -1;
    }
}

int shell_timeout_unregiseter(int tid, int repeat)
{
    timeout_t *to = timeout_wait;
    timeout_t **ref = &timeout_wait;
    int drop = 0;

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

            to   = next;
            *ref = next;

            timeout_release(to);
            drop ++;
        } else {
            ref = &to->next;
            to = to->next;
        }
    }

    return drop;
}

