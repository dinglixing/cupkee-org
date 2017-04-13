#include "app.h"

static char app_cmd_buf[APP_CMD_BUF_SIZE];

static cupkee_device_t *tty;
static cupkee_device_t *i2c;

static void i2c_read_sync(uint8_t offset, uint8_t n, uint8_t *data)
{
    cupkee_device_write_sync(i2c, 1, &offset);
    cupkee_device_read_sync(i2c, n, data);
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
    uint8_t buf[10];
    int n;

    n = cupkee_device_read(dev, 10, buf);
    if (n > 0) {
        int i;
        console_log("i2c data:\r\n");

        for (i = 0; i < n; i++) {
            console_log("%.2x ", buf[i]);
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

static void app_systick_proc(void)
{
    static int c = 0;

    /* add user code here */
    if (++c > 50) {
        i2c_read(10, 10);
        c = 0;
    }
}

static int app_event_handle(event_info_t *e)
{
    switch(e->type) {
    case EVENT_SYSTICK: app_systick_proc(); break;
    default: break;
    }
    return 0;
}

static int app_cmd_hello(int ac, char **av)
{
    (void) ac;
    (void) av;

    console_log("hello cupkee!\r\n");
    console_log("i2c is %s\r\n", i2c ? "OK" : "FAIL");

    return 0;
}

static int app_cmd_i2c_read_sync(int ac, char **av)
{
    uint8_t off = 0, data;

    if (ac > 1) {
        off = atoi(av[1]);
    }

    i2c_read_sync(off, 1, &data);
    console_log("i2c %u: %u\r\n", off, data);

    return 0;
}

static int app_cmd_i2c_read(int ac, char **av)
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

static int app_cmd_i2c_write(int ac, char **av)
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

static cupkee_command_entry_t app_cmd_entrys[] = {
    {"hello", app_cmd_hello},
    {"read",  app_cmd_i2c_read},
    {"write", app_cmd_i2c_write},
    {"read_sync", app_cmd_i2c_read_sync},
};


static void app_setup(void)
{
    /**********************************************************
     * Map pin of debug LED
     *********************************************************/
    hw_led_map(1, 5);

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
        if (1 != cupkee_device_set(i2c, 0, 0xa0)) {
            console_log_sync("set i2c slave fail!\r\n");
        }
    } else {
        console_log_sync("request i2c fail!\r\n");
    }
}

static void console_init(void)
{
#ifdef USE_USB_CONSOLE
    tty = cupkee_device_request("usb-cdc", 0);
#else
    tty = cupkee_device_request("uart", 2);
    tty->config.data.uart.baudrate = 115200;
    tty->config.data.uart.stop_bits = DEVICE_OPT_STOPBITS_1;
    tty->config.data.uart.data_bits = 8;
#endif
    cupkee_device_enable(tty);

    cupkee_command_init(4, app_cmd_entrys, APP_CMD_BUF_SIZE, app_cmd_buf);
    cupkee_history_init();
    cupkee_console_init(tty, cupkee_command_handle);
}

int main(void)
{
    cupkee_init();
    console_init();

    /**********************************************************
     * app device create & setup
     *********************************************************/
    app_setup();

    /**********************************************************
     * User event handle register
     *********************************************************/
    cupkee_event_handle_register(app_event_handle);

    console_log("APP start!\r\n");
    /**********************************************************
     * Let's Go!
     *********************************************************/
    cupkee_loop();

    /**********************************************************
     * Should not go here, make gcc happy!
     *********************************************************/
    return 0;
}

