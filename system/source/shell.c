/*
 * This file is part of the cupkee project.
 *
 * Copyright (C) 2016 Lixing ding <ding.lixing@gmail.com>
 *
 */


#include "shell.h"
#include "misc.h"
#include "timeout.h"
#include "console.h"


#define HISTORY_SIZE        (64)

typedef struct auto_complete_t {
    uint8_t prefix;
    uint8_t supply;
    uint8_t same;
    uint8_t pos;
    char buf[TOKEN_MAX_SIZE];
} auto_complete_t;

static env_t *env_ptr;
static int shell_mode = 0;
static int input_cached = 0;
static int input_buf_size = 0;
static int history_max = 0;
static char *input_buf = NULL;
static char *history_buf = NULL;
static char history_start, history_count, history_pos;

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
        char *buf = history_buf + pos * HISTORY_SIZE;
        pos += history_start;
        if (pos >= history_max) {
            pos -= history_max;
        }
        console_input_string(buf);
    }

    return CON_PREVENT_DEFAULT;
}

static void history_append(char *line)
{
    int pos;
    char *buf;

    if (history_count < history_max) {
        pos = history_count++;
    } else {
        pos = history_start++;
        if (history_start >= history_max) {
            history_start = 0;
        }
    }
    buf = history_buf + pos * HISTORY_SIZE;

    pos = 0;
    for (pos = 0; pos < HISTORY_SIZE - 1; pos++) {
        char c = line[pos];
        if (c == 0 || c == '\n' || c == '\r') {
            break;
        }
        buf[pos] = c;
    }
    buf[pos] = 0;

    // reset position to tail
    history_pos = history_count;
}

static int do_complete(const char *sym, void *param)
{
    auto_complete_t *ac = (auto_complete_t *)param;
    char *prefix = ac->buf;
    char *supply = ac->buf + ac->prefix;

    if (strncmp(sym, prefix, ac->prefix)) {
        return 0;
    }

    ac->same++;
    if (ac->same == 1) {
        char *p = strncpy(supply, sym + ac->prefix, TOKEN_MAX_SIZE - ac->prefix - 1);
        if (p == supply) {
            ac->supply = strlen(supply);
        } else {
            *p = 0;
            ac->supply = p - supply;
        }
        return 0;
    }

    /* show suggist */
    if (ac->same == 2) {
        console_puts("\r\n");
        console_puts(ac->buf);
        ac->pos += strlen(ac->buf);
    }

    // padding spaces, align 4
    do {
        console_put(' ');
        ac->pos++;
    } while (ac->pos % 4);
    console_puts(sym);
    ac->pos += strlen(sym);

    if (ac->pos > 80) {
        console_puts("\r\n");
    }

    /* update supply */
    int n = 0;
    while (n < ac->supply && sym[ac->prefix + n] == supply[n]) {
        n++;
    }
    supply[n] = 0;
    ac->supply = n;

    return 0;
}

static int input_complete(void)
{
    int has;
    auto_complete_t ac;

    has = console_input_curr_tok(ac.buf, TOKEN_MAX_SIZE - 1);
    if (has) {
        ac.pos = 0;
        ac.prefix = has;
        ac.supply = 0;
        ac.same = 0;

        env_symbal_foreach(env_ptr, do_complete, &ac);
        if (ac.same) {
            if (ac.same > 1) {
                int i = 0, ch;
                console_puts(shell_mode == 0 ? "\r\n> " : "\r\n. ");
                while ((ch = console_input_peek(i++)) > 0) {
                    console_put(ch);
                }
            }
        }
        if (ac.supply) {
            console_input_string(ac.buf + has);
            return CON_PREVENT_DEFAULT;
        }
    } else {
        console_input_string("    ");
        return CON_PREVENT_DEFAULT;
    }

    return 0;
}

static int input_enter(void)
{
    event_put(EVENT_CONSOLE_INPUT);
    return 0;
}

static int input_save(void)
{
    if (hal_scripts_save(input_buf)) {
        console_puts("save fail ...");
        return 0;
    } else {
        console_puts("save ok ...");
        return CON_PREVENT_DEFAULT;
    }
}

static int console_ctrl_handle(int ctrl)
{
    switch (ctrl) {
    case CON_CTRL_TABLE: return input_complete();
    case CON_CTRL_UP:    return history_load(-1);
    case CON_CTRL_DOWN:  return history_load(1);
    case CON_CTRL_ENTER: return input_enter();
    case CON_CTRL_F2:    return input_save();
    default: return 0;
    }
}

static int run_initial_scripts(env_t *env, const char *initial)
{
    const char *script = NULL;
    val_t *res;
    int err = 0;

    if (initial) {
        err = interp_execute_string(env, initial, &res);
    }

    while (err >= 0 && NULL != (script = hal_scripts_load(script))) {
        err = interp_execute_string(env, script, &res);
    }

    return err;
}

static char *more_handle(void)
{
    shell_mode = 1;
    return NULL;
}

static void shell_execute_line(env_t *env)
{
    val_t *res;
    int    err;

    console_gets(input_buf, input_buf_size);

    err = interp_execute_interactive(env, input_buf, more_handle, &res);
    if (err < 0) {
        if (shell_mode == 0 || err != -ERR_InvalidToken) {
            shell_mode = 0;
            print_error(-err);
        }
    } else
    if (err > 0) {
        shell_mode = 0;
        print_value(res);
        history_append(input_buf);
    }

    if (shell_mode == 1) {
        input_cached = strlen(input_buf);
        console_puts(". ");
    } else {
        console_puts("> ");
    }
}

static void shell_execute_multi(env_t *env)
{
    val_t *res;
    int    err;
    int    len;

    console_gets(input_buf + input_cached, input_buf_size - input_cached);
    len = strlen(input_buf + input_cached);
    if (len <= 2) {
        //empty line: "\r\n" only
        // exit multi input mode, and execute
        input_cached = 0;
        shell_mode = 0;
    } else {
        console_puts(". ");
        input_cached += len;
        return;
    }

    err = interp_execute_string(env, input_buf, &res);
    if (err < 0) {
        print_error(-err);
    } else
    if (err > 0) {
        print_value(res);
        //history_append(input_buf);
    }
    console_puts("> ");
}

int shell_init(env_t *env, void *mem, int size)
{
    int blocks = size / HISTORY_SIZE;

    if (blocks < 4) {
        history_max = 0;
    } else {
        history_max = blocks / 2;
    }
    input_buf = mem + (HISTORY_SIZE * history_max);
    input_buf_size = size - HISTORY_SIZE * history_max;
    history_buf = mem;
    memset(mem, 0, size);

    history_init();

    /* Register console ctrl handle, to support: auto complete, history .etc */
    console_handle_register(console_ctrl_handle);

    env_ptr = env;

    return 0;
}

int shell_start(const char *initial)
{
    /* Execute scripts initial and restored by user, */
    return run_initial_scripts(env_ptr, initial);
}

void shell_execute(env_t *env)
{
    if (shell_mode) {
        shell_execute_multi(env);
    } else {
        shell_execute_line(env);
    }
}

