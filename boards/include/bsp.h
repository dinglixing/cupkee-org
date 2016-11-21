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

#ifndef __BSP_INC__
#define __BSP_INC__

#include <stdint.h>

#define SYSTEM_TICKS_PRE_SEC    1000
#define SYSTEM_STACK_SIZE       (8 * 1024)

enum hw_device_type_t {
    HW_DEVICE_MAP = 0,
    HW_DEVICE_STREAM,
    HW_DEVICE_BLOCK
};

enum hw_config_type_t {
    HW_CONFIG_BOOL = 0,
    HW_CONFIG_NUM,
    HW_CONFIG_OPT,
};

#define DEVICE_EVENT_ERR        0
#define DEVICE_EVENT_DATA       1
#define DEVICE_EVENT_DRAIN      2
#define DEVICE_EVENT_READY      3
#define DEVICE_EVENT_MAX        4

typedef struct hw_info_t {
    int ram_sz;
    int rom_sz;
    void *ram_base;
    void *rom_base;
    unsigned sys_freq;
    unsigned sys_ticks_pre_sec;
} hw_info_t;

typedef struct hw_config_desc_t {
    uint8_t  type;
    uint8_t  opt_num;
    uint16_t opt_start;
} hw_config_desc_t;

typedef struct hw_device_t {
    const char *name;
    uint8_t id;
    uint8_t type;
    uint8_t inst_num;
    uint8_t conf_num;
    uint8_t event_num;
    uint8_t reserved[3];
    const hw_config_desc_t *conf_descs;
    const char            **conf_names;
    const char            **opt_names;
} hw_device_t;

/*
int hw_device_request(int id, int inst);
int hw_device_release(int id, int inst);
int hw_device_error(int id, int inst);
int hw_device_enable(int id, int inst);
int hw_device_disable(int id, int inst);
int hw_device_config_set(int id, int inst, int which, int value);
int hw_device_config_get(int id, int inst, int which, int *value);
int hw_device_listen(int id, int inst, int which);
int hw_device_ignore(int id, int inst, int which);
*/

typedef struct hw_driver_t {
    int (*request) (int, int);
    int (*release) (int, int);
    int (*get_err) (int, int);
    int (*enable)  (int, int);
    int (*disable) (int, int);
    int (*config_set) (int, int, int, int);
    int (*config_get) (int, int, int, int*);
    void (*listen) (int, int, int);
    void (*ignore) (int, int, int);
    union {
        struct {
            int (*set) (int, int, int, uint32_t);
            int (*get) (int, int, int, uint32_t*);
            int (*size)(int, int);
        } map;
        struct {
            int (*recv) (int, int, int, void *);
            int (*send) (int, int, int, void *);
            int (*received) (int, int);
        } stream;
        struct {
            int (*read)  (int, int, int, int, void *);
            int (*write) (int, int, int, int, void *);
        } block;
    } io;
} hw_driver_t;


extern uint32_t system_ticks_count;

void hw_setup(void);
void _hw_reset(void);

void hw_poll(void);
void hw_halt(void);

void hw_info_get(hw_info_t *);

int hw_memory_alloc(void **p, int size, int align);

/* device */
const hw_device_t *hw_device_take(const char *name, int inst, const hw_driver_t **driver);
const hw_device_t *hw_device_descript(int i);

/* console */
int hw_console_set_callback(void (*input)(void *, int), void (*drain)(void));
int hw_console_putc(int ch);
int hw_console_puts(const char *s);
int hw_console_sync_putc(int ch);
int hw_console_sync_puts(const char *s);

/* misc */
int hw_scripts_erase(void);
int hw_scripts_remove(int id);
int hw_scripts_save(const char *s);
const char *hw_scripts_load(const char *prev);

void hw_led_set(void);
void hw_led_clear(void);
void hw_led_toggle(void);

#endif /* __BSP_INC__ */

