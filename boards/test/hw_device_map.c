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

#include "hardware.h"

#define DEVICE_NAME     "map"
#define DEVICE_ID       0
#define EVENT_NUM       2
#define CONFIG_NUM      3
#define INSTANCE_NUM    2

/*******************************************************************************
 * dbg field
*******************************************************************************/
static uint32_t dbg_val[INSTANCE_NUM];
static int      dbg_off[INSTANCE_NUM];
static int      dbg_event_flags[INSTANCE_NUM];
static int      dbg_size[INSTANCE_NUM];

static void device_set_error(int id, int inst, int error);

void hw_dbg_map_set(int inst, int off, uint32_t v)
{
    dbg_off[inst] = off;
    dbg_val[inst] = v;
}

int hw_dbg_map_off_get(int inst)
{
    return dbg_off[inst];
}

uint32_t hw_dbg_map_val_get(int inst)
{
    return dbg_val[inst];
}

void hw_dbg_map_set_size(int inst, int size)
{
    dbg_size[inst] = size;
}

void hw_dbg_map_event_triger(int inst, int event)
{
    if (event == DEVICE_EVENT_ERR) {
        device_set_error(0, inst, 17);
    } else {
        dbg_event_flags[inst] |= 1 << event;
    }
}

/*******************************************************************************
 * dbg field end
*******************************************************************************/
static const char *device_config_names[] = {
    "bool", "number", "option"
};
static const char *device_opt_names[] = {
    "x", "yy", "zzz"
};
static const hw_config_desc_t device_config_descs[] = {
    {
        .type = HW_CONFIG_BOOL,
    },
    {
        .type = HW_CONFIG_NUM,
    },
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 3,
        .opt_start = 0,
    }
};

static uint8_t device_used = 0;
static uint8_t device_work = 0;
static int device_error[INSTANCE_NUM];
static int device_config_settings[INSTANCE_NUM][CONFIG_NUM];
static int device_event_settings[INSTANCE_NUM];

static inline int device_is_inused(int inst) {
    if (inst < INSTANCE_NUM) {
        return device_used & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_is_work(int inst) {
    if (inst < INSTANCE_NUM) {
        return (device_used & device_work) & (1 << inst);
    } else {
        return 0;
    }
}

static void device_set_error(int id, int inst, int error)
{
    device_error[inst] = error;

    (void) id;

    if (device_event_settings[inst] & 1) {
        devices_event_post(DEVICE_ID, inst, 0);
    }
}

static int device_get_error(int id, int inst)
{
    if (inst >= INSTANCE_NUM) {
        return 0;
    }

    (void) id;

    return device_error[inst];
}

static int device_setup(int inst)
{
    (void) inst;
    return 1;
}

static int device_reset(int inst)
{
    (void) inst;
    return 1;
}

static int device_enable(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        uint8_t b = 1 << inst;

        if (!(device_work & b)) {
            device_work |= b;
            return device_setup(inst);
        }
        return 1;
    }
    return 0;
}

static int device_disable(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        uint8_t b = 1 << inst;

        if (device_work & b) {
            device_work &= ~b;
            return device_reset(inst);
        } else {
            return 1;
        }
    }
    return 0;
}

// 0: fail
// 1: ok
static int device_request(int id, int inst)
{
    (void) id;
    if (inst < INSTANCE_NUM) {
        int used = device_used & (1 << inst);

        if (!used) {
            int c;

            device_error[inst] = 0;
            device_event_settings[inst] = 0;
            device_used |= 1 << inst;
            device_work &= ~(1 << inst);
            for (c = 0; c < CONFIG_NUM; c++) {
                device_config_settings[inst][c] = 0;
            }

            return 1;
        }
    }

    return 0;
}

// 0: fail
// other: ok
static int device_release(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        device_disable(id, inst);
        device_used &= ~(1 << inst);
        return 1;
    } else {
        return 0;
    }
}

static int device_config_set(int id, int inst, int which, int setting)
{
    (void) id;
    if (device_is_inused(inst) && which < CONFIG_NUM) {
        device_config_settings[inst][which] = setting;
        return 1;
    }
    return 0;
}

static int device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    if (device_is_inused(inst) && which < CONFIG_NUM && setting) {
        *setting = device_config_settings[inst][which];
        return 1;
    }
    return 0;
}

static void device_listen(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < EVENT_NUM) {
        device_event_settings[inst] |= 1 << event;
    }
}

static void device_ignore(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < EVENT_NUM) {
        device_event_settings[inst] &= ~(1 << event);
    }
}

static int device_get(int id, int inst, int off, uint32_t *v)
{
    (void) id;
    if (dbg_off[inst] == off) {
        *v = dbg_val[inst];
        return 1;
    }
    return 0;
}

static int device_set(int id, int inst, int off, uint32_t val)
{
    (void) id;

    dbg_off[inst] = off;
    dbg_val[inst] = val;

    return 1;
}

static int device_size(int id, int inst)
{
    (void) id;
    return dbg_size[inst];
}

void hw_device_map_setup(void)
{
    device_used = 0;
    device_work = 0;
}

void hw_device_map_poll(void)
{
    int i, e;

    for (i = 0; i < INSTANCE_NUM; i++) {
        if (!device_is_work(i)) {
            continue;
        }

        for (e = 1; e < EVENT_NUM; e++) {
            int test = 1 << e;
            if ((device_event_settings[i] & test) && (dbg_event_flags[i] & test)) {
                devices_event_post(DEVICE_ID, i, e);
                dbg_event_flags[i] &= ~test;
            }
        }
    }
}

const hw_driver_t hw_driver_map = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,
    .io.map.get = device_get,
    .io.map.set = device_set,
    .io.map.size = device_size,
};

const hw_device_t hw_device_map = {
    .name = DEVICE_NAME,
    .id   = DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = INSTANCE_NUM,
    .conf_num   = CONFIG_NUM,
    .event_num  = EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = device_config_descs,
    .opt_names  = device_opt_names,
};

