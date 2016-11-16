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

#ifndef __CUPKEE_INC__
#define __CUPKEE_INC__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <bsp.h>
#include <panda.h>

/************************************************************************
 * Cupkee error code
 ***********************************************************************/
#define CUPKEE_OK               0       // not implement
#define CUPKEE_ERROR            10000   // error not in list:
#define CUPKEE_EIMPLEMENT       20000   // not implement
#define CUPKEE_ENAME            20001   // invalid device name
#define CUPKEE_EINVAL           20002   // invalid argument
#define CUPKEE_ERESOURCE        20003   // not enought resource
#define CUPKEE_EHARDWARE        20004   // hardware error

/************************************************************************
 * Cupkee API
 ***********************************************************************/
int cupkee_init(void);
int cupkee_start(const char *scripts);
int cupkee_poll(void);
int cupkee_set_native(const native_t *, int n);

/************************************************************************
 * Cupkee native functions
 ***********************************************************************/
val_t native_sysinfos(env_t *env, int ac, val_t *av);
val_t native_systicks(env_t *env, int ac, val_t *av);
val_t native_print(env_t *env, int ac, val_t *av);
val_t native_scripts(env_t *env, int ac, val_t *av);

// Timer
val_t native_set_timeout(env_t *env, int ac, val_t *av);
val_t native_set_interval(env_t *env, int ac, val_t *av);
val_t native_clear_timeout(env_t *env, int ac, val_t *av);
val_t native_clear_interval(env_t *env, int ac, val_t *av);

// Device
val_t native_device(env_t *env, int ac, val_t *av);
val_t native_unref(env_t *env, int ac, val_t *av);

val_t device_native_enable(env_t *env, int ac, val_t *av);
val_t device_native_config(env_t *env, int ac, val_t *av);

val_t device_native_read(env_t *env, int ac, val_t *av);
val_t device_native_write(env_t *env, int ac, val_t *av);

val_t device_native_listen(env_t *env, int ac, val_t *av);
val_t device_native_ignore(env_t *env, int ac, val_t *av);

// Device special api
val_t gpio_native_pin(env_t *env, int ac, val_t *av);

#endif /* __CUPKEE_INC__ */

