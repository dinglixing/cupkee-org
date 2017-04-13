#include "app.h"

static char app_cmd_buf[APP_CMD_BUF_SIZE];

static cupkee_device_t *tty;
static cupkee_device_t *i2c;

static void i2c_read_sync(uint8_t offset, uint8_t n, uint8_t *buf)
{
    cupkee_device_write_sync(i2c, 1, &offset);
    cupkee_device_read_sync(i2c, n, buf);
}

static void i2c_write_sync(uint8_t offset, uint8_t n, uint8_t *data)
{
    cupkee_device_write_sync(i2c, 1, &offset);
    cupkee_device_write_sync(i2c, n, data);
}

static void app_systick_proc(void)
{
    /* add user code here */
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

static int app_cmd_i2c_read(int ac, char **av)
{
    uint8_t off, data;

    if (ac > 1) {
        off = atoi(av[1]);
    } else {
        off = 0;
    }

    i2c_read_sync(off, 1, &data);
    console_log("read i2c %u: %x\r\n", off, data);

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

    i2c_write_sync(off, 1, &data);

    console_log("write %u: %u!\r\n", off, data);

    return 0;
}

static cupkee_command_entry_t app_cmd_entrys[] = {
    {"hello", app_cmd_hello},
    {"read",  app_cmd_i2c_read},
    {"write", app_cmd_i2c_write},
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
        i2c->config.data.i2c.speed = 100000;
        i2c->config.data.i2c.addr = 0;
        cupkee_device_enable(i2c);

        // set slave address
        cupkee_device_set(i2c, 0, 0xa0);
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

    cupkee_command_init(3, app_cmd_entrys, APP_CMD_BUF_SIZE, app_cmd_buf);
    cupkee_history_init();
    cupkee_console_init(tty, cupkee_command_handle);
}

int main(void)
{
    /**********************************************************
     * system init
     *********************************************************/
    cupkee_init();

    /**********************************************************
     * app device create & setup
     *********************************************************/
    app_setup();

    console_init();

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

