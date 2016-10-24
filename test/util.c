#include "test.h"

static int   reply_show = 0;

static char *test_cut_prompt(char *buf, int end)
{
    if (end < 4) {
        return NULL;
    }

    if (buf[end - 1] == ' ' && buf[end - 2] == '>' &&
        buf[end - 3] == '\n' && buf[end - 4] == '\r') {
        buf[end - 4] = 0;
        return buf;
    }

    return NULL;
}

static char *test_parse_cupkee_reply()
{
    int   len;
    char *buf;

    len = hw_mock_console_reply(&buf);
    if (len > 0) {
        if (reply_show) {
            printf("reply: %s\n", buf);
        }
        return test_cut_prompt(buf, len);
    } else {
        return NULL;
    }
}

static char *test_cupkee_wait_reply(int try)
{
    int n = 0;

    hw_mock_console_buf_reset();
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

int test_cupkee_run_with_reply(const char *input, const char *expected, int try_max)
{
    char *reply;

    hw_mock_console_give(input);
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
