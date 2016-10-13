
#include "hal.h"
#include "sal.h"

#include "rbuff.h"


static int console_ready = 0;
static rbuff_t console_buff[CON_BUFF_MAX];
static char console_buff_mem[CON_BUFF_MAX][CON_BUFF_SIZE];
static unsigned console_recv_bytes = 0;
//static unsigned console_send_bytes = 0;

static int console_write_byte(int x, char c)
{
    rbuff_t *rb = &console_buff[x];
    int pos = rbuff_push(rb);
    if (pos < 0) {
        return 0;
    }
    *RBUFF_ELEM_PTR(rb, char, pos) = c;
    return 1;
}

static int console_read_byte(int x, char *c)
{
    rbuff_t *rb = &console_buff[x];
    int pos = rbuff_shift(rb);
    if (pos < 0) {
        return 0;
    }
    *c = *RBUFF_ELEM_PTR(rb, char, pos);
    return 1;
}

static int console_unread_byte(int x, char c)
{
    rbuff_t *rb = &console_buff[x];
    int pos = rbuff_unshift(rb);
    if (pos < 0) {
        return 0;
    }
    *RBUFF_ELEM_PTR(rb, char, pos) = c;
    return 1;
}

static void console_input_handle(void *data, int len)
{
    char *s = (char *)data;
    int i = 0;

    if (!console_ready) {
        console_ready = 1;
        event_put(EVENT_CONSOLE_READY);
    }

    while (i < len) {
        char c = s[i++];

        console_write_byte(CON_RECV, c);
        console_put(c);
        if (c == '\r') {
            console_put('\n');
            event_put(EVENT_CONSOLE_INPUT);
            console_write_byte(CON_RECV, '\n');
        }
    }

    console_recv_bytes += i;
}

static void console_drain_handle(void)
{
    char c;
    while (console_read_byte(CON_SEND, &c)) {
        if (!hal_console_write_byte(c)) {
            console_unread_byte(CON_SEND, c);
            break;
        }
    }
}

int console_init(void)
{
    rbuff_init(&console_buff[CON_RECV], CON_BUFF_SIZE, console_buff_mem[CON_RECV]);
    rbuff_init(&console_buff[CON_SEND], CON_BUFF_SIZE, console_buff_mem[CON_SEND]);

    return hal_console_set_cb(console_input_handle, console_drain_handle);
}

int console_put(char c)
{
    if (rbuff_is_empty(&console_buff[CON_SEND])) {
        if (hal_console_write_byte(c))
            return 1;
    }

    return console_write_byte(CON_SEND, c);
}

int console_puts(const char *s)
{
    const char *p = s;

    if (rbuff_is_empty(&console_buff[CON_SEND])) {
        while(*p && hal_console_write_byte(*p))
            p++;
    }
    while(*p && console_write_byte(CON_SEND, *p))
        p++;

    return p - s;
}

int console_gets(char *buf, int max)
{
    char c;
    int i = 0;

    while (console_read_byte(CON_RECV, &c) && i < max - 1) {
        buf[i++] = c;
    }

    buf[i] = 0;
    return i;
}

