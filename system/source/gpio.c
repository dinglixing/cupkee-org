#include "device.h"

static int gpio_init(cupkee_device_t *dev)
{
    (void) dev;

    return -CUPKEE_EIMPLEMENT;
}

static int gpio_deinit(cupkee_device_t *dev)
{
    (void) dev;

    return -CUPKEE_EIMPLEMENT;
}

static int gpio_config(cupkee_device_t *dev, env_t *env, val_t *k, val_t *v)
{
    (void) dev;
    (void) env;
    (void) k;
    (void) v;

    return -CUPKEE_EIMPLEMENT;
}

static int gpio_read(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static int gpio_write(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static int gpio_read_int(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static int gpio_write_int(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static int gpio_read_uint(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static int gpio_write_uint(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

// export
cupkee_driver_t cupkee_gpio_driver = {
    .init   = gpio_init,
    .deinit = gpio_deinit,
    .config = gpio_config,
    .read   = gpio_read,
    .write  = gpio_write,
    .read_int   = gpio_read_int,
    .read_uint  = gpio_read_uint,
    .write_int  = gpio_write_int,
    .write_uint = gpio_write_uint,
};
