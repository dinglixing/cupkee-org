/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

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

#include "app.h"

#define COMMAND_SIZ_MAX      512
#define COMMAND_ARG_MAX      8

static uint32_t systick_pass = 0;
static char     command_str[COMMAND_SIZ_MAX];
static char    *args[COMMAND_ARG_MAX];

static int seconds = 0;
static const gprs_host_t gprs_hosts[] = {
    {"TCP", "www.cupkee.cn", "8124"},
    {"TCP", "www.cupkee.cn", "8124"}  // second host
};

static void app_systick_proc(void)
{
    gprs_systick_proc();

    if (++systick_pass > 1000) {
        char buf[16];
        int  n;

        systick_pass = 0;
        seconds++;

        n = snprintf(buf, 16, "PASS %d\r\n", seconds);
        gprs_send(n, buf);
        hw_led_toggle();
    }
}

static void app_gprs_cb(int len, void *data)
{
    char *msg = (char *) data;

    (void) len;

    console_log(msg);
}

static int app_command_parse(char *input)
{
    int i;
    char *word, *p;
    const char *sep = " \t\r\n";

    for (word = strtok_r(input, sep, &p), i = 0;
         word;
         word = strtok_r(NULL, sep, &p)) {
        args[i++] = word;
    }

    return i;
}

static void app_command_do(int ac, char **av)
{
    if (!strcmp(av[0], "send")) {
        if (ac < 2) {
            gprs_send(5, "hello");
        } else {
            gprs_send(strlen(av[1]), av[1]);
        }
    } else
    if (!strcmp(av[0], "show")) {
        if (ac < 2) {
            gprs_show_msg(1);
        } else {
            gprs_show_msg(0);
        }
    } else
    if (!strcmp(av[0], "at")) {
        if (ac < 2) {
            gprs_send_command(NULL);
        } else {
            gprs_send_command(av[1]);
        }
    } else
    if (!strcmp(av[0], "ss")) {
        if (ac < 2) {
            gprs_send_data(NULL);
        } else {
            gprs_send_data(av[1]);
        }
    } else
    if (!strcmp(av[0], "hello")) {
        console_log("hi\r\n");
    }
}

static const char *commands[] = {
    "hello", "world", "nihao", "wohao", "welcome"
};

static int app_console_handle(int type, int ch)
{
    (void) ch;

    if (type == CON_CTRL_ENTER) {
        int len = console_input_load(COMMAND_SIZ_MAX, command_str);
        int argc;

        if (len < 1) {
            return CON_EXECUTE_DEF;
        }
        command_str[len] = 0;
        cupkee_history_push(len, command_str);

        argc = app_command_parse(command_str);
        if (argc) {
            app_command_do(argc, args);
        }
    } else
    if (type == CON_CTRL_TABLE) {
        return cupkee_auto_complete(5, commands);
    } else
    if (type == CON_CTRL_UP) {
        return cupkee_history_load(-1);
    } else
    if (type == CON_CTRL_DOWN) {
        return cupkee_history_load(1);
    }

    return CON_EXECUTE_DEF;
}

static int app_event_handle(event_info_t *e)
{
    switch(e->type) {
    case EVENT_SYSTICK: app_systick_proc(); break;
    case EVENT_GPRS:    gprs_event_proc(e->which); break;
    default: break;
    }
    return 0;
}

static cupkee_device_t *usb_cdc;
static cupkee_device_t *uart1;
static cupkee_device_t *uart2;
static cupkee_device_t *io_gprs;

static void reset_gprs_device(void)
{
    cupkee_device_set(io_gprs, 0, 0);
    cupkee_device_set(io_gprs, 1, 0);
}

static void enable_gprs_device(void)
{
    cupkee_device_set(io_gprs, 0, 1);
}

static void start_gprs_device(void)
{
    cupkee_device_set(io_gprs, 1, 1);
}

int main(void)
{
    /**********************************************************
     * Cupkee nosys initial
     *********************************************************/
    cupkee_init();

    /**********************************************************
     * Map pin of debug LED
     *********************************************************/
    hw_led_map(0, 8);

    /**********************************************************
     * Map pin of GPIOs
     *********************************************************/
    hw_pin_map(0, 2, 6); // PIN0(GPIO C6) GPRS_EN
    hw_pin_map(1, 2, 7); // PIN1(GPIO C7) GPRS_PWR

    hw_pin_map(2, 2, 8); // PIN1(GPIO C8) LOCK_CTL
    hw_pin_map(3, 2, 9); // PIN1(GPIO C9) DOOR_ST

    /**********************************************************
     * User event handle register
     *********************************************************/
    cupkee_event_handle_register(app_event_handle);

    /**********************************************************
     * Console initial
     *********************************************************/
#if 1
    uart1 = cupkee_device_request("uart", 0);
    uart1->config.data.uart.baudrate = 115200;
    uart1->config.data.uart.stop_bits = DEVICE_OPT_STOPBITS_1;
    uart1->config.data.uart.data_bits = 8;
    cupkee_device_enable(uart1);

    cupkee_history_init();
    cupkee_console_init(uart1, app_console_handle);

#else
    usb_cdc = cupkee_device_request("usb-cdc", 0);
    if (usb_cdc) {
        cupkee_device_enable(usb_cdc);

        cupkee_history_init();
        cupkee_console_init(usb_cdc, app_console_handle);
    }
#endif

    /**********************************************************
     * Create & Setup
     *********************************************************/
    io_gprs = cupkee_device_request("pin", 0);
    io_gprs->config.data.pin.num = 2;
    io_gprs->config.data.pin.start = 0;
    io_gprs->config.data.pin.dir = DEVICE_OPT_DIR_OUT;
    cupkee_device_enable(io_gprs);

    uart2 = cupkee_device_request("uart", 1);
    uart2->config.data.uart.baudrate = 115200;
    uart2->config.data.uart.stop_bits = DEVICE_OPT_STOPBITS_1;
    uart2->config.data.uart.data_bits = 8;
    cupkee_device_enable(uart2);

    gprs_init(uart2, sizeof(gprs_hosts) / sizeof(gprs_host_t), gprs_hosts);
    gprs_recv_cb_register(app_gprs_cb);

    /**********************************************************
     * Let's Go!
     *********************************************************/
    console_log("APP start!\r\n");

    gprs_start(reset_gprs_device, enable_gprs_device, start_gprs_device);

    cupkee_loop();

    /**********************************************************
     * Should not go here, make gcc happy!
     *********************************************************/
    return 0;
}

