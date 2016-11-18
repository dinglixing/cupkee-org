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

#ifndef __DEVICE_INC__
#define __DEVICE_INC__

#include <cupkee.h>

#define DEV_FL_ENBALE 1
#define DEV_FL_ERROR  2

struct cupkee_driver_t;
typedef struct cupkee_device_t {
    uint16_t magic;
    uint16_t flags;
    void     *data;
    const struct cupkee_driver_t *driver;
    struct cupkee_device_t *next;
} cupkee_device_t;

typedef struct device_config_handle_t {
    int (*setter)(cupkee_device_t *, env_t *, val_t *);
    val_t (*getter)(cupkee_device_t *, env_t *);
} device_config_handle_t;

typedef struct cupkee_driver_t {
    int (*init)     (cupkee_device_t *dev);
    int (*deinit)   (cupkee_device_t *dev);

    int (*enable)   (cupkee_device_t *dev);
    int (*disable)  (cupkee_device_t *dev);

    int (*listen)   (cupkee_device_t *dev, val_t *event, val_t *callback);
    int (*ignore)   (cupkee_device_t *dev, val_t *event);

    val_t (*read)   (cupkee_device_t *dev, env_t *env, int ac, val_t *av);
    val_t (*write)  (cupkee_device_t *dev, env_t *env, int ac, val_t *av);

    void (*event_handle) (env_t *env, uint8_t which, uint8_t event);
    device_config_handle_t *(*config) (cupkee_device_t *dev, val_t *name);
} cupkee_driver_t;

void devices_setup(void);
void devices_event_proc(env_t *env, int event);

static inline int device_param_stream(int ac, val_t *av, void **addr, int *size) {
    if (ac) {
        if (val_is_buffer(av)) {
            *addr = buffer_addr(av);
            *size = buffer_size(av);
        } else
        if ((*size = string_len(av)) < 0) {
            return 0;
        } else {
            *addr = (void *) val_2_cstring(av);
        }
        return 1;
    }
    return 0;
}

static inline int device_param_int(int ac, val_t *av, int *n) {
    if (ac > 0 && val_is_number(av)) {
        *n = val_2_integer(av);
        return 1;
    } else {
        return 0;
    }
}

// new here
void device_setup(void);
void xxx_event_proc(env_t *env, int event);



#endif /* __DEVICE_INC__ */

