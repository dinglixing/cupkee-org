/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

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

#include "cupkee.h"

static cupkee_device_t devices[APP_DEV_MAX];
static cupkee_device_t *device_free = NULL;
static cupkee_device_t *device_work = NULL;
static uint8_t device_magic;

static cupkee_device_t *device_block_alloc(void)
{
    cupkee_device_t *dev = device_free;

    if (dev) {
        device_free = dev->next;
        dev->magic = device_magic++;
    }
    return dev;
}

static void device_block_release(cupkee_device_t *dev)
{
    memset(dev, 0, sizeof(cupkee_device_t));

    dev->next = device_free;
    device_free = dev;
}

static void device_join_work_list(cupkee_device_t *device)
{
    device->next = device_work;
    device_work = device;
}

static void device_drop_work_list(cupkee_device_t *device)
{
    cupkee_device_t *cur = device_work;

    if (cur == device) {
        device_work = cur->next;
        return;
    }

    while(cur) {
        cupkee_device_t *next = cur->next;

        if (next == device) {
            cur->next = device->next;
            return;
        }

        cur = next;
    }

    device->next = NULL;
}

static const cupkee_device_desc_t *device_query(const char *name)
{
    unsigned i;

    for (i = 0; device_entrys[i]; i++) {
        if (strcmp(name, device_entrys[i]->name) == 0) {
            return device_entrys[i];
        }
    }
    return NULL;
}

static const cupkee_device_desc_t *device_query_by_type(uint16_t type)
{
    unsigned i;

    for (i = 0; device_entrys[i]; i++) {
        if (device_entrys[i]->type == type) {
            return device_entrys[i];
        }
    }
    return NULL;
}

static cupkee_device_t *device_request(const cupkee_device_desc_t *desc, int instance)
{
    cupkee_device_t *dev;
    const hw_driver_t *driver;

    dev = device_block_alloc();
    if (!dev) {
        return NULL;
    }

    driver = hw_device_request(desc->type, instance);
    if (!driver) {
        device_block_release(dev);
        return NULL;
    }

    dev->flags = 0;
    dev->error = 0;
    dev->instance = instance;

    dev->driver = driver;
    dev->desc   = desc;

    dev->next   = NULL;

    return dev;
}

int cupkee_device_id(cupkee_device_t *device)
{
    int id = device - devices;

    return id < APP_DEV_MAX ? id : -1;
}

cupkee_device_t *cupkee_device_block(int id)
{
    if (id >= 0 && id < APP_DEV_MAX) {
        cupkee_device_t *dev = &devices[id];

        if (dev->desc) {
            return dev;
        }
    }

    return NULL;
}

hw_config_t *cupkee_device_config(int id)
{
    if (id >= 0 && id < APP_DEV_MAX) {
        cupkee_device_t *dev = &devices[id];

        if (dev->desc) {
            return &dev->config;
        }
    }

    return NULL;
}

void cupkee_device_set_error(int id, uint8_t code)
{
    if (id >= 0 && id < APP_DEV_MAX) {
        cupkee_device_t *dev = &devices[id];

        if (dev->desc) {
            dev->error = code;
            cupkee_event_post_device_error(id);
        }
    }
}

void cupkee_device_event_handle(uint8_t which, uint16_t code)
{
    cupkee_device_t *dev = &devices[which];

    if ((dev->flags & DEVICE_FL_ENABLE) && dev->handle) {
        dev->handle(dev, code, dev->handle_param);
    }
}

void cupkee_device_sync(uint32_t systicks)
{
    cupkee_device_t *dev = device_work;

    while (dev) {
        if (dev->driver->sync) {
            dev->driver->sync(dev->instance, systicks);
        }
        dev = dev->next;
    }
}

void cupkee_device_poll(void)
{
    cupkee_device_t *dev = device_work;

    hw_poll();
    while (dev) {
        if (dev->driver->poll) {
            dev->driver->poll(dev->instance);
        }
        dev = dev->next;
    }
}

int cupkee_device_init(void)
{
    int i;

    /* Hardware device startup */
    hw_setup();

    /* Device blocks initial */
    memset(devices, 0, sizeof(devices));
    device_free = NULL;
    device_work = NULL;
    device_magic = cupkee_systicks();

    for (i = 0; i < APP_DEV_MAX; i++) {
        cupkee_device_t *d = &devices[i];

        d->next = device_free;
        device_free = d;
    }

    return 0;
}

cupkee_device_t *cupkee_device_request(const char *name, int instance)
{
    const cupkee_device_desc_t *desc;

    desc = device_query(name);
    if (!desc) {
        return NULL;
    }

    return device_request(desc, instance);
}

cupkee_device_t *cupkee_device_request2(int type, int instance)
{
    const cupkee_device_desc_t *desc;

    desc = device_query_by_type(type);
    if (!desc) {
        return NULL;
    }

    return device_request(desc, instance);
}

int cupkee_device_release(cupkee_device_t *dev)
{
    cupkee_device_disable(dev);

    dev->driver->release(dev->instance);
    device_block_release(dev);

    return CUPKEE_OK;
}

int cupkee_device_enable(cupkee_device_t *dev)
{
    int id;

    if (cupkee_device_is_enabled(dev)) {
        return CUPKEE_OK;
    }

    id = cupkee_device_id(dev);
    if (id < 0) {
        return -CUPKEE_EINVAL;
    }

    if (0 == dev->driver->setup(dev->instance, id, &dev->config)) {
        dev->flags |= DEVICE_FL_ENABLE;
        device_join_work_list(dev);
        return CUPKEE_OK;
    }

    return -CUPKEE_ERROR;
}

int cupkee_device_disable(cupkee_device_t *dev)
{
    if (cupkee_device_is_enabled(dev)) {
        dev->driver->reset(dev->instance);
        device_drop_work_list(dev);
        dev->flags &= ~DEVICE_FL_ENABLE;
    }

    return CUPKEE_OK;
}

int cupkee_device_send(cupkee_device_t *dev, int n, const void *data)
{
    if (cupkee_device_is_enabled(dev)) {
        if (dev->driver->io.stream.send) {
            return dev->driver->io.stream.send(dev->instance, n, (void *)data);
        } else {
            return -CUPKEE_EIMPLEMENT;
        }
    } else {
        return -CUPKEE_EENABLED;
    }
}

int cupkee_device_recv(cupkee_device_t *dev, int n, void *buf)
{
    if (cupkee_device_is_enabled(dev)) {
        if (dev->driver->io.stream.recv) {
            return dev->driver->io.stream.recv(dev->instance, n, buf);
        } else {
            return -CUPKEE_EIMPLEMENT;
        }
    } else {
        return -CUPKEE_EENABLED;
    }
}

int cupkee_device_send_sync(cupkee_device_t *dev, int n, const void *data)
{
    if (cupkee_device_is_enabled(dev)) {
        if (dev->driver->io.stream.send_sync) {
            return dev->driver->io.stream.send_sync(dev->instance, n, (void *)data);
        } else {
            return -CUPKEE_EIMPLEMENT;
        }
    } else {
        return -CUPKEE_EENABLED;
    }
}

int cupkee_device_recv_sync(cupkee_device_t *dev, int n, void *buf)
{
    if (cupkee_device_is_enabled(dev)) {
        if (dev->driver->io.stream.recv_sync) {
            return dev->driver->io.stream.recv_sync(dev->instance, n, buf);
        } else {
            return -CUPKEE_EIMPLEMENT;
        }
    } else {
        return -CUPKEE_EENABLED;
    }
}

int cupkee_device_received(cupkee_device_t *dev)
{
    if (dev->desc->category == DEVICE_CATEGORY_STREAM) {
        if (dev->driver->io.stream.received) {
            return dev->driver->io.stream.received(dev->instance);
        }
    }
    return 0;
}

int cupkee_device_set(cupkee_device_t *dev, int n, uint32_t data)
{
    if (cupkee_device_is_enabled(dev)) {
        if (dev->driver->set) {
            return dev->driver->set(dev->instance, n, data);
        } else {
            return -CUPKEE_EIMPLEMENT;
        }
    } else {
        return -CUPKEE_EENABLED;
    }
}

int cupkee_device_get(cupkee_device_t *dev, int n, uint32_t *data)
{
    if (cupkee_device_is_enabled(dev)) {
        if (dev->driver->get) {
            return dev->driver->get(dev->instance, n, data);
        } else {
            return -CUPKEE_EIMPLEMENT;
        }
    } else {
        return -CUPKEE_EENABLED;
    }
}

