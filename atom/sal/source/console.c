#include <ctype.h>
#include <string.h>
#include "hal.h"
#include "sal.h"

#include "rbuff.h"


#define BELL    "\007"
#define CRLF    "\r\n"
#define RIGHT   "\033[C"
#define LEFT    "\010"

static int console_ready = 0;
static int console_in_pos = 0;
static rbuff_t console_buff[CON_BUFF_MAX];
static char console_buff_mem[CON_BUFF_MAX][CON_BUFF_SIZE];
static unsigned console_recv_bytes = 0;
//static unsigned console_send_bytes = 0;
static int (*console_handle)(int ctrl) = NULL;

static int console_buf_write_byte(int x, char c)
{
    rbuff_t *rb = &console_buff[x];
    int pos = rbuff_push(rb);
    if (pos < 0) {
        return 0;
    }
    console_buff_mem[x][pos] = c;
    return 1;
}

static int console_buf_read_byte(int x, char *c)
{
    rbuff_t *rb = &console_buff[x];
    int pos = rbuff_shift(rb);
    if (pos < 0) {
        return 0;
    }
    *c = console_buff_mem[x][pos];
    return 1;
}

static int console_buf_unread_byte(int x, char c)
{
    rbuff_t *rb = &console_buff[x];
    int pos = rbuff_unshift(rb);
    if (pos < 0) {
        return 0;
    }
    console_buff_mem[x][pos] = c;
    return 1;
}

#if 0
#include <stdio.h>
static void parse_input(void *data, int len)
{
    char buf[128];
    int pos, i;
    char *d = (char *) data;

    pos = snprintf(buf, 10, "[%d] ", len);
    for (i = 0; i < len; i++) {
        pos += snprintf(buf + pos, 128 - pos, "%d-%x ", d[i], d[i]);
    }
    pos += snprintf(buf + pos, 128 - pos, "\r\n");
    buf[pos] = 0;

    hal_console_sync_puts(buf);
}
#endif

int console_input_curr_tok(char *buf, int size)
{
    if (console_in_pos == 0) {
        return 0;
    }

    rbuff_t *rb = &console_buff[CON_IN];
    char *ptr = console_buff_mem[CON_IN];
    int start = console_in_pos;
    int pos = 0;

    while ((pos = rbuff_get(rb, start - 1)) >= 0) {
        int c = ptr[pos];

        if (!isalnum(c) && c != '_') {
            break;
        }
        start--;
    }

    for (pos = 0; pos < size && start < console_in_pos; pos++, start++) {
        buf[pos] = ptr[rbuff_get(rb, start)];
    }
    buf[pos] = 0;

    return pos;
}

int  console_input_peek(int pos)
{
    rbuff_t *rb = &console_buff[CON_IN];
    char *ptr = console_buff_mem[CON_IN];

    if (pos < rb->cnt) {
        return ptr[rbuff_get(rb, pos)];
    } else {
        return 0;
    }
}

static int console_input_parse(char *input, int end, int *ppos, char *pkey)
{
    int  type = CON_CTRL_IDLE;
    int  pos = *ppos;
    char key = input[pos++];

    if (key >= 32 && key < 127) {
        *pkey = key;
        type = CON_CTRL_CHAR;
    } else {
        if (key == 8) {
            type = CON_CTRL_BACKSPACE;
        } else
        if (key == 9) {
            type = CON_CTRL_TABLE;
        } else
        if (key == 13) {
            type = CON_CTRL_ENTER;
        } else
        if (key == 127) {
            type = CON_CTRL_DELETE;
        } else
        if (key == 27) {
            if (pos == end) {
                type = CON_CTRL_ESCAPE;
            } else
            if (input[pos] == 91) {
                key = input[pos + 1];
                if (key == 65) {
                    type = CON_CTRL_UP;
                } else
                if (key == 66) {
                    type = CON_CTRL_DOWN;
                } else
                if (key == 67) {
                    type = CON_CTRL_RIGHT;
                } else
                if (key == 68) {
                    type = CON_CTRL_LEFT;
                }
            }
            pos = end; // Skip all left input
        }
    }
    *ppos = pos;

    return type;
}

void console_input_clear(void)
{
    rbuff_t *rb = &console_buff[CON_IN];
    int n = rbuff_end(rb);
    int i;

    if (n <= 0) {
        return;
    }

    rbuff_reset(rb);
    while (console_in_pos) {
        console_put(8);
        console_in_pos--;
    }

    for (i = 0; i < n; i++) {
        console_put(' ');
    }
    for (i = 0; i < n; i++) {
        console_put(8);
    }
}

void console_input_string(char *s)
{
    int len = strlen(s);
    rbuff_t *rb = &console_buff[CON_IN];

    if (len == 0) {
        return;
    }

    if (console_in_pos == rbuff_end(rb)) {
        while (*s) {
            char c = *s++;
            console_put(c);
            console_buf_write_byte(CON_IN, c);
        }
        console_in_pos += len;
    } else {
        char *ptr = console_buff_mem[CON_IN];
        int pos = console_in_pos;
        int i, n = rbuff_end(rb) - pos;

        console_puts(s);
        rbuff_append(rb, len);
        for (i = n - 1; i >= 0; i--) {
            ptr[rbuff_get(rb, pos + i + len)] = ptr[rbuff_get(rb, pos + i)];
        }
        for (i = 0; i < n; i++) {
            console_put(ptr[rbuff_get(rb, pos + i + len)]);
        }
        while (n--) {
            console_puts(LEFT);
        }
        for (i = 0; i < len; i++)
            ptr[rbuff_get(rb, console_in_pos++)] = s[i];
    }
}

static void console_input_char(char c)
{
    rbuff_t *rb = &console_buff[CON_IN];

    if (console_in_pos == rbuff_end(rb)) {
        console_put(c);
        console_buf_write_byte(CON_IN, c);
        console_in_pos++;
    } else {
        int pos = console_in_pos++;
        int i, n = rbuff_end(rb) - pos;
        char *ptr = console_buff_mem[CON_IN];

        rbuff_append(rb, 1);
        for (i = 0; i <= n; i++) {
            int  dst = rbuff_get(rb, pos + i);
            char tmp;

            console_put(c);
            tmp = ptr[dst];
            ptr[dst] = c;
            c = tmp;
        }
        while (n--) {
            console_puts(LEFT);
        }
    }
}

static void console_input_enter(void)
{
    console_puts(CRLF);
    console_buf_write_byte(CON_IN, '\n');
    console_in_pos = 0;
}

static void console_input_delete(void)
{
    rbuff_t *rb = &console_buff[CON_IN];
    char *ptr = console_buff_mem[CON_IN];

    if (console_in_pos <= 0) {
        console_puts(BELL);
        return;
    }

    console_puts(LEFT);

    int pos = console_in_pos--;
    int i, n = rbuff_end(rb) - pos;
    int dst = rbuff_get(rb, pos - 1);

    for (i = 0; i < n; i++) {
        int src = rbuff_get(rb, pos + i);

        console_put(ptr[src]);
        ptr[dst] = ptr[src];
        dst = src;
    }

    rbuff_remove(rb, 1);
    console_put(' ');

    while (n-- >= 0) {
        console_puts(LEFT);
    }
}

static void console_input_seek(int n)
{
    int pos = console_in_pos + n;

    if (pos < 0 || pos > rbuff_end(&console_buff[CON_IN])) {
        console_puts(BELL);
    } else {
        console_in_pos = pos;
        if (n > 0) {
            while(n--)
                console_puts(RIGHT);
        } else {
            while(n++)
                console_puts(LEFT);
        }
    }
}

static void console_input_proc(int type, char c)
{
    if (console_handle && console_handle(type)) {
        return;
    }

    switch (type) {
    case CON_CTRL_IDLE:         break;
    case CON_CTRL_CHAR:         console_input_char(c); break;
    case CON_CTRL_BACKSPACE:    console_input_delete(); break;
    case CON_CTRL_DELETE:       console_input_delete(); break;
    case CON_CTRL_TABLE:        console_puts(BELL); break;
    case CON_CTRL_ENTER:        console_input_enter(); break;
    case CON_CTRL_ESCAPE:       console_puts(BELL); break;
    case CON_CTRL_UP:           console_puts(BELL); break;
    case CON_CTRL_DOWN:         console_puts(BELL); break;
    case CON_CTRL_RIGHT:        console_input_seek(1);  break;
    case CON_CTRL_LEFT:         console_input_seek(-1); break;
    default: break;
    }
}

static void console_input_handle(void *data, int n)
{
    if (!console_ready) {
        console_ready = 1;
        event_put(EVENT_CONSOLE_READY);
    }

    int pos = 0;
    char c = '.';
    while (pos < n) {
        int type = console_input_parse(data, n, &pos, &c);
        console_input_proc(type, c);
    }
    console_recv_bytes += n;
}

static void console_drain_handle(void)
{
    char c;
    while (console_buf_read_byte(CON_OUT, &c)) {
        if (!hal_console_write_byte(c)) {
            console_buf_unread_byte(CON_OUT, c);
            break;
        }
    }
}

int console_init(void)
{
    rbuff_init(&console_buff[CON_IN], CON_BUFF_SIZE, console_buff_mem[CON_IN]);
    rbuff_init(&console_buff[CON_OUT], CON_BUFF_SIZE, console_buff_mem[CON_OUT]);

    return hal_console_set_cb(console_input_handle, console_drain_handle);
}

int console_handle_register(int (*handle)(int))
{
    console_handle = handle;
    return 0;
}

int console_put(char c)
{
    if (rbuff_is_empty(&console_buff[CON_OUT])) {
        if (hal_console_write_byte(c))
            return 1;
    }

    return console_buf_write_byte(CON_OUT, c);
}

int console_puts(const char *s)
{
    const char *p = s;

    if (rbuff_is_empty(&console_buff[CON_OUT])) {
        while(*p && hal_console_write_byte(*p))
            p++;
    }
    while(*p && console_buf_write_byte(CON_OUT, *p))
        p++;

    return p - s;
}

int console_gets(char *buf, int max)
{
    char c;
    int i = 0;

    while (console_buf_read_byte(CON_IN, &c) && i < max - 1) {
        buf[i++] = c;
    }

    buf[i] = 0;
    return i;
}

