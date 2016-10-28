#include "test.h"

static int reply_show = 0;

static char *test_cut_prompt(char *buf, int end)
{
    if (end > 1 && buf[end - 1] == ' ' && buf[end - 2] == '>') {
        buf[end - 2] = 0;
    }

    return buf;
}

static char *test_parse_cupkee_reply()
{
    int   len;
    char *buf;

    len = hw_dbg_console_get_reply(&buf);
    if (len > 0) {
        if (reply_show) {
            printf("reply(%d): '%s'\n", len, buf);
        }
        return test_cut_prompt(buf, len);
    } else {
        return NULL;
    }
}

static char *test_cupkee_wait_reply(int try)
{
    int n = 0;

    hw_dbg_console_clr_buf();
    while (n++ < try) {
        char *reply;

        cupkee_poll();

        reply = test_parse_cupkee_reply();
        if (reply) {
            return reply;
        }
    }
    return NULL; // no reply
}

void test_reply_show(int on)
{
    reply_show = on;
}

int test_cupkee_run_with_reply(const char *input, const char *expected, int try_max)
{
    char *reply;

    if (input) {
        hw_dbg_console_set_input(input);
    }

    reply = test_cupkee_wait_reply(try_max);
    if (reply) {
        if (expected == NULL) {
            return 0;
        }

        if (strcmp(reply, expected) == 0) {
            return 0;
        } else {
            return -2;
        }
    } else {
        return -1;
    }
}

int test_cupkee_run_without_reply(const char *input, int try_max)
{
    char *reply;

    if (input) {
        hw_dbg_console_set_input(input);
    }
    reply = test_cupkee_wait_reply(try_max);
    if (reply) {
        return -1;
    } else {
        return 0;
    }
}
