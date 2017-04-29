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

static cupkee_device_t *i2c;

/*
static void i2c_read_sync(uint8_t offset, uint8_t *data)
{
    cupkee_device_write_sync(i2c, 1, &offset);
    cupkee_device_read_sync(i2c, 1, data);
}
*/

static void i2c_write_sync(uint8_t offset, uint8_t data)
{
    uint8_t buf[2];

    buf[0] = offset;
    buf[1] = data;

    cupkee_device_write_sync(i2c, 2, buf);
}

static void i2c_read(uint8_t offset, uint8_t n)
{
    cupkee_device_write(i2c, 1, &offset);
    cupkee_device_read_req(i2c, n);
}

static void i2c_write(uint8_t offset, uint8_t n, uint8_t *data)
{
    uint8_t buf[10];

    buf[0] = offset;
    memcpy(buf + 1, data, n);
    cupkee_device_write(i2c, n + 1, buf);
}

static void i2c_data_cb(cupkee_device_t *dev)
{
    uint8_t buf[20];
    int n;

    n = cupkee_device_read(dev, 20, buf);
    if (n > 0) {
        int i;

        console_log("i2c data:\r\n");
        for (i = 0; i < 7; i++) {
            int16_t v = buf[i * 2];

            v = (v << 8) | buf[i * 2 + 1];
            if (i == 3) {
                console_log("%f ", v / 340.0 + 36.53);
            } else {
                console_log("%d ", v);
            }
        }
        console_log("\r\n");
    }
}

static void i2c_event_handle(cupkee_device_t *dev, uint8_t code, intptr_t param)
{
    (void) param;

    switch (code) {
    case DEVICE_EVENT_ERR:   console_log("i2c error: %u\r\n", dev->error); break;
    case DEVICE_EVENT_DATA:  i2c_data_cb(dev); break;
    case DEVICE_EVENT_DRAIN: console_log("i2c drain\r\n"); break;
    default: break;
    }
}

static int command_hello(int ac, char **av)
{
    (void) ac;
    (void) av;

    console_log("hello cupkee!\r\n");
    console_log("i2c is %s\r\n", i2c ? "OK" : "FAIL");

    return 0;
}

static int command_i2c_read(int ac, char **av)
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

    return 0;
}

static int command_i2c_write(int ac, char **av)
{
    uint8_t off = 0, data = 0x11;

    if (ac > 1) {
        off = atoi(av[1]);
        if (ac > 2) {
            data = atoi(av[2]);
        }
    }

    i2c_write(off, 1, &data);

    console_log("write %u: %u!\r\n", off, data);

    return 0;
}

static cupkee_command_entry_t command_entries[] = {
    {"hello", command_hello},
    {"read",  command_i2c_read},
    {"write", command_i2c_write},
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
     * Map pin of GPIOs
     *********************************************************/
    i2c = cupkee_device_request2(DEVICE_TYPE_I2C, 0);
    if (i2c) {
        int err;
        i2c->config.data.i2c.speed = 100000;
        i2c->config.data.i2c.addr = 0;
        i2c->handle = i2c_event_handle;

        if (0 != (err = cupkee_device_enable(i2c))) {
            console_log_sync("enable i2c fail! %d\r\n", err);
        }

        // set slave address
        if (1 != cupkee_device_set(i2c, 0, 0xD0)) {
            console_log_sync("set i2c slave fail!\r\n");
        }

        i2c_write_sync(0x6b, 0);
        i2c_write_sync(0x19, 0x07);
        i2c_write_sync(0x1A, 0x06);
        i2c_write_sync(0x1B, 0x18);
        i2c_write_sync(0x1C, 0x01);

        console_log("GO Start!\r\n");
        return 0;
    } else {
        console_log_sync("request i2c fail!\r\n");
        return -1;
    }
}
