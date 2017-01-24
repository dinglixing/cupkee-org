#include "cupkee.h"

static int      device_alloced = 0;
static device_t devices[APP_DEV_MAX];

void cupkee_device_event_handle(uint8_t which, uint16_t code)
{
    device_t *dev = &devices[which];

    if (code < EVENT_DEVICE_MAX && dev->handles[code]) {
        dev->handles[code]();
    }
}

void cupkee_device_sync(uint32_t systicks)
{
    int i;

    for (i = 0; i < APP_DEV_MAX; i++) {
        device_t *dev = &devices[i];
        if (dev->driver == NULL) {
            break;
        }

        if (dev->driver->sync) {
            dev->driver->sync(dev->instance, systicks);
        }
    }
}

void cupkee_device_poll(void)
{
    int i;

    hw_poll();

    for (i = 0; i < APP_DEV_MAX; i++) {
        device_t *dev = &devices[i];
        if (dev->driver == NULL) {
            break;
        }

        if (dev->driver->poll) {
            dev->driver->poll(dev->instance);
        }
    }
}

int cupkee_device_init(void)
{
    /* Hardware device startup */
    hw_setup();

    /* Resouce initial */
    device_alloced = 0;
    memset(devices, 0, sizeof(devices));

    return 0;
}

device_t *cupkee_device_alloc(int type, int instance)
{
    device_t *dev;
    const hw_driver_t *driver;

    if (device_alloced >= APP_DEV_MAX) {
        return NULL;
    }
    dev = &devices[device_alloced];

    driver = hw_device_request(type, instance);
    if (!driver) {
        return NULL;
    }

    dev->id       = device_alloced++;
    dev->instance = instance;
    dev->state    = 0;
    dev->driver   = driver;

    return dev;
}

int cupkee_device_enable(device_t *dev)
{
    if (0 == dev->driver->setup(dev->instance, dev->id, &dev->config)) {
        dev->state = 1;
        return 0;
    }
    return -1;
}

int cupkee_device_send(device_t *dev, int n, const void *data)
{
    if (dev->state) {
        return dev->driver->io.stream.send(dev->instance, n, (void *)data);
    } else {
        return -1;
    }
}

int cupkee_device_recv(device_t *dev, int n, void *buf)
{
    if (dev->state) {
        return dev->driver->io.stream.recv(dev->instance, n, buf);
    } else {
        return -1;
    }
}

int cupkee_device_set(device_t *dev, int n, uint32_t data)
{
    if (dev->state) {
        return dev->driver->io.map.set(dev->instance, n, data);
    } else {
        return -1;
    }
}

int cupkee_device_get(device_t *dev, int n, uint32_t *data)
{
    if (dev->state) {
        return dev->driver->io.map.get(dev->instance, n, data);
    } else {
        return -1;
    }
}

