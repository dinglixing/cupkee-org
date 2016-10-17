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
#define INPUT_BUF_SIZE    (1024)
#define HISTORY_SIZE  (128)
#define HISTORY_MAX   (4)
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

static char input_buf[INPUT_BUF_SIZE];
static char history_buf[HISTORY_MAX][HISTORY_SIZE];
static char history_start, history_count, history_pos;

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

static char auto_buf[TOKEN_MAX_SIZE]; // panda/config.h
static int auto_complete(void)
{
    int has;

    has = console_input_curr_tok(auto_buf, TOKEN_MAX_SIZE - 1);
    if (has) {
        int supplement = env_symbal_complete(&shell_env, auto_buf, has, TOKEN_MAX_SIZE, NULL);
        if (supplement > 0) {
            console_input_string(auto_buf + has);
            return CON_PREVENT_DEFAULT;
        }
    } else {
        console_input_string("    ");
        return CON_PREVENT_DEFAULT;
    }

    return 0;
}

static void history_init(void)
{
    history_start = 0;
    history_count = 0;
    history_pos = 0;
}

static int history_load(int dir)
{
    if (history_count <= 0) {
        return 0;
    }

    int pos = history_pos + dir;
    if (pos < 0 || pos > history_count) {
        return 0;
    }
    history_pos = pos;

    console_input_clear();
    if (pos < history_count) {
        pos += history_start;
        if (pos >= HISTORY_MAX) {
            pos -= HISTORY_MAX;
        }
        console_input_string(history_buf[pos]);
    }

    return CON_PREVENT_DEFAULT;
}

static void history_append(char *line)
{
    int pos;
    char *buf;

    if (history_count < HISTORY_MAX) {
        pos = history_count++;
    } else {
        pos = history_start++;
        if (history_start >= HISTORY_MAX) {
            history_start = 0;
        }
    }
    buf = history_buf[pos];

    pos = 0;
    for (pos = 0; pos < HISTORY_SIZE - 1; pos++) {
        char c = line[pos];
        if (c == 0 || c == '\n') {
            break;
        }
        buf[pos] = c;
    }
    buf[pos] = 0;

    // reset position to tail
    history_pos = history_count;
}

static int enter(void)
{
    event_put(EVENT_CONSOLE_INPUT);
    return 0;
}

static int console_ctrl_handle(int ctrl)
{
    switch (ctrl) {
    case CON_CTRL_TABLE: return auto_complete();
    case CON_CTRL_UP:    return history_load(-1);
    case CON_CTRL_DOWN:  return history_load(1);
    case CON_CTRL_ENTER: return enter();
    default: return 0;
    }
}

int shell_init(void)
{
    /* Initialise shell evn */
    if(0 != interp_env_init_interactive(&shell_env, memory, MEM_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE)) {
        //sal_console_output("env_init fail\n");
        return -1;
    }

    /* Initialise all native functions */
    native_init(&shell_env);

    /* Initialise timer */
    timeout_init(&shell_env);

    /* */
    history_init();

    /* Register console ctrl handle, to support: auto complete, history .etc */
    console_handle_register(console_ctrl_handle);


    return 0;
}
static int execute_mode = 0;
static int input_cached = 0;
static char *more_handle(void)
{
    execute_mode = 1;
    return NULL;
}

static void shell_input_execute_line(void)
{
    val_t *res;
    int    err;

    console_gets(input_buf, INPUT_BUF_SIZE);

    err = interp_execute_interactive(&shell_env, input_buf, more_handle, &res);
    if (err < 0) {
        if (execute_mode == 0 || err != -ERR_InvalidToken) {
            execute_mode = 0;
            print_error(-err);
        }
    } else
    if (err > 0) {
        execute_mode = 0;
        print_value(res);
        history_append(input_buf);
    }

    if (execute_mode == 1) {
        input_cached = strlen(input_buf);
        console_puts(". ");
    } else {
        console_puts("> ");
    }
}

static void shell_input_execute_multi(void)
{
    val_t *res;
    int    err;
    int    len;

    console_gets(input_buf + input_cached, INPUT_BUF_SIZE - input_cached);
    len = strlen(input_buf + input_cached);
    if (len > 1) {
        // Not empty line, continue read to buffer
        console_puts(". ");
        input_cached += len;
        return;
    } else {
        input_cached = 0;
        execute_mode = 0;
    }

    err = interp_execute_string(&shell_env, input_buf, &res);
    if (err < 0) {
        print_error(-err);
    } else
    if (err > 0) {
        print_value(res);
        //history_append(input_buf);
    }
    console_puts("> ");

}

void shell_input_execute(void)
{
    if (execute_mode) {
        shell_input_execute_multi();
    } else {
        shell_input_execute_line();
    }
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

