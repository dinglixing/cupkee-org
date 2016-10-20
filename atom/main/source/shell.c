/*
 * This file is part of the cupkee project.
 *
 * Copyright (C) 2016 Lixing ding <ding.lixing@gmail.conz>
 *
 */

#include "sal.h"
#include "shell.h"
#include "misc.h"
#include "timeout.h"

#define HEAP_SIZE     (1024 * 8)
#define STACK_SIZE    (256)
#define EXE_MEM_SPACE (1024 * 8)
#define SYM_MEM_SPACE (1024 * 2)
#define MEM_SIZE      (STACK_SIZE * sizeof(val_t) + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)
#define INPUT_BUF_SIZE    (1024)
#define HISTORY_SIZE  (128)
#define HISTORY_MAX   (4)

#define VARIABLE_REF_MAX    (16)

static uint8_t memory[MEM_SIZE];
static env_t shell_env;

static int execute_mode = 0;

static int input_cached = 0;
static char input_buf[INPUT_BUF_SIZE];
static char history_buf[HISTORY_MAX][HISTORY_SIZE];
static char history_start, history_count, history_pos;

static val_t variable_refs[VARIABLE_REF_MAX];

static void reference_init(env_t *env)
{
    int i;

    for (i = 0; i < VARIABLE_REF_MAX; i++) {
        val_set_undefined(&variable_refs[i]);
    }

    env_reference_set(env, variable_refs, VARIABLE_REF_MAX);
}

val_t *shell_reference_create(val_t *v)
{
    int i;

    for (i = 0; i < VARIABLE_REF_MAX; i++) {
        val_t *r = variable_refs + i;

        if (val_is_undefined(r)) {
            *r = *v;
            return r;
        }
    }
    return NULL;
}

void shell_reference_release(val_t *ref)
{
    if (ref) {
        int pos = ref - variable_refs;

        if (pos >= 0 && pos < VARIABLE_REF_MAX) {
            val_set_undefined(ref);
        }
    }
}

typedef struct auto_complete_t {
    uint8_t prefix;
    uint8_t supply;
    uint8_t same;
    uint8_t pos;
    char buf[TOKEN_MAX_SIZE];
} auto_complete_t;

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

static int auto_complete(void)
{
    int has;
    auto_complete_t ac;

    has = console_input_curr_tok(ac.buf, TOKEN_MAX_SIZE - 1);
    if (has) {
        ac.pos = 0;
        ac.prefix = has;
        ac.supply = 0;
        ac.same = 0;

        env_symbal_foreach(&shell_env, do_complete, &ac);
        if (ac.same) {
            if (ac.same > 1) {
                int i = 0, ch;
                console_puts(execute_mode == 0 ? "\r\n> " : "\r\n. ");
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
        if (c == 0 || c == '\n' || c == '\r') {
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

static int save_input(void)
{
    if (usr_scripts_append(input_buf)) {
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
    case CON_CTRL_TABLE: return auto_complete();
    case CON_CTRL_UP:    return history_load(-1);
    case CON_CTRL_DOWN:  return history_load(1);
    case CON_CTRL_ENTER: return enter();
    case CON_CTRL_F2:    return save_input();
    default: return 0;
    }
}

static int shell_execute_start_script(void)
{
    const char *script = NULL;
    val_t *res;
    int err = 0;

    while (NULL != (script = usr_scripts_next(script))) {
        err = interp_execute_string(&shell_env, script, &res);
        if (err < 0) {
            break;
        }
    }

    return err;
}

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
    if (len <= 2) {
        //empty line: "\r\n" only
        // exit multi input mode, and execute
        input_cached = 0;
        execute_mode = 0;
    } else {
        console_puts(". ");
        input_cached += len;
        return;
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

static const native_t native_entry[] = {
    {"sysinfos",        native_sysinfos},
    {"systicks",        native_systicks},

    {"print",           native_print},
    {"led",             native_led},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"scripts",         native_scripts},
};

int shell_init(void)
{
    /* Initialise shell evn */
    if(0 != interp_env_init_interactive(&shell_env, memory, MEM_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE)) {
        return -1;
    }

    /* Initialise all native functions */
    env_native_set(&shell_env, native_entry, sizeof(native_entry)/sizeof(native_t));

    /* Initialise reference */
    reference_init(&shell_env);

    /* Initialise timer */
    timeout_init(&shell_env);

    /* */
    history_init();

    /* Register console ctrl handle, to support: auto complete, history .etc */
    console_handle_register(console_ctrl_handle);

    /* Execute user restored scripts */
    shell_execute_start_script();

    return 0;
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
    timeout_execute(&shell_env);
}

