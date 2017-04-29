/*
MIT License

This file is part of cupkee project.

Copyright (c) 2017 Lixing Ding <ding.lixing@gmail.com>

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

#include "board.h"

#include "at24cxx.h"

static int e2prom;

static void e2prom_event_handle(cupkee_device_t *dev, uint8_t code, intptr_t param)
{
    (void) param;

    switch (code) {
    case DEVICE_EVENT_ERR:   console_log("e2prom error: %u\r\n", dev->error); break;
    case DEVICE_EVENT_DATA:  e2prom_data_cb(dev); break;
    case DEVICE_EVENT_DRAIN: console_log("e2prom drain\r\n"); break;
    default: break;
    }
}

static int command_hello(int ac, char **av)
{
    (void) ac;
    (void) av;

    console_log("hello cupkee module example!");

    return 0;
}

static int command_at24cxx_read(int ac, char **av)
{
    uint8_t off = 0, n = 1;

    if (ac > 1) {
        off = atoi(av[1]);
        if (ac > 2) {
            n = atoi(av[2]);
        }
    }

    i2c_read(off, n);
    console_log("i2c read %u from %u\r\n", n, off);

    return at24cxx_read_req(e2prom, off, n);
}

static int command_at24cxx_write(int ac, char **av)
{
    uint8_t off = 0, data = 0x11;

    if (ac > 1) {
        off = atoi(av[1]);
        if (ac > 2) {
            data = atoi(av[2]);
        }
    }

    console_log("write %u: %u!\r\n", off, data);

    return at24cxx_write(e2prom, off, 1, &data);
}

static cupkee_command_entry_t command_entries[] = {
    {"hello", command_hello},
    {"read",  command_at24cxx_read},
    {"write", command_at24cxx_write},
};

int board_commands(void)
{
    return sizeof(command_entries) / sizeof(cupkee_command_entry_t);
}

cupkee_command_entry_t *board_command_entries(void)
{
    return command_entries;
}

int board_setup(void)
{
    cupkee_device_t *i2c; // at24cxx

    /**********************************************************
     * Map pin of debug LED
     *********************************************************/
    hw_led_map(0, 8);

    /**********************************************************
     * Map pin of GPIOs
     *********************************************************/
    //hw_pin_map(0, 0, 0); // PIN0(GPIO A0)
    //hw_pin_map(1, 0, 1); // PIN0(GPIO A1)
    // ...
    //hw_pin_map(15, 0, 15); // PIN0(GPIO A15)

    /**********************************************************
     * initial board modules
     *********************************************************/
    if (at24cxx_mod_init()) {
//    if (at24cxx_mod_register()) {
        console_log_sync("mod at24cxx initial fail\r\n");
    }


    /**********************************************************
     * Setup board modules
     *********************************************************/
    i2c = cupkee_device_request2(DEVICE_TYPE_I2C, 0);
    if (i2c) {
        /*
        int err;
        i2c->config.data.i2c.speed = 100000;
        i2c->config.data.i2c.addr = 0;
        i2c->handle = i2c_event_handle;

        if ((err = cupkee_device_enable(i2c)) != 0) {
            console_log_sync("enable i2c fail! %d\r\n", err);
            hw_halt();
        }
        */

        if ((e2prom = at24cxx_request()) >= 0) {
            at24cxx_config_t *conf = (at24cxx_config_t *) cupkee_device_config(e2prom);
            conf->i2c = i2c;
            conf->addr = 0xA0;
        } else {
            console_log_sync("request at24cxx instance fail! %d\r\n", e2prom);
            hw_halt();
        }

        if (cupkee_device_enable(e2prom) != CUPKEE_OK) {
            console_log_sync("enable at24cxx device fail! %d\r\n", e2prom);
            hw_halt();
        }
    } else {
        console_log_sync("request i2c fail!\r\n");
        return -1;
    }

    console_log("GO Start!\r\n");
    return 0;
}
