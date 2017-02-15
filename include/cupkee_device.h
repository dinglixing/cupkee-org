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

#ifndef __CUPKEE_DEVICE_INC__
#define __CUPKEE_DEVICE_INC__

#define DEVICE_FL_ENABLE    1

typedef struct cupkee_device_t cupkee_device_t;
typedef void (*cupkee_handle_t)(cupkee_device_t *, uint8_t event, void *);

typedef struct cupkee_device_desc_t {
    const char *name;
    uint16_t type;
    uint8_t  category;
    uint8_t  conf_num;
    const char **conf_names;
    void (*conf_init)(hw_config_t *);
} cupkee_device_desc_t;

struct cupkee_device_t {
    uint8_t id;
    uint8_t flags;
    uint8_t error;
    uint8_t instance;

    hw_config_t config;

    cupkee_handle_t handle;
    void     *handle_param;

    const cupkee_device_desc_t *desc;
    const hw_driver_t *driver;

    struct cupkee_device_t *next;
};

extern const cupkee_device_desc_t *device_entrys[];

int  cupkee_device_init(void);
void cupkee_device_poll(void);
void cupkee_device_sync(uint32_t systicks);
void cupkee_device_event_handle(uint8_t which, uint16_t code);

cupkee_device_t *cupkee_device_request(const char *name, int instance);
cupkee_device_t *cupkee_device_request2(int type, int instance);

int cupkee_device_id(cupkee_device_t *device);
int cupkee_device_release(cupkee_device_t *dev);

static inline int cupkee_device_is_enabled(cupkee_device_t *dev) {
    return dev->flags & DEVICE_FL_ENABLE;
}

int cupkee_device_enable(cupkee_device_t *dev);
int cupkee_device_disable(cupkee_device_t *dev);

int cupkee_device_set(cupkee_device_t *dev, int n, uint32_t data);
int cupkee_device_get(cupkee_device_t *dev, int n, uint32_t *data);

int cupkee_device_send(cupkee_device_t *dev, int n, const void *data);
int cupkee_device_recv(cupkee_device_t *dev, int n, void *buf);
int cupkee_device_send_sync(cupkee_device_t *dev, int n, const void *data);
int cupkee_device_recv_sync(cupkee_device_t *dev, int n, void *buf);

#endif /* __CUPKEE_DEVICE_INC__ */

