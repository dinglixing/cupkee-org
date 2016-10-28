
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
#define CUPKEE_EIMPLEMENT       20000   // not implement
#define CUPKEE_ENAME            20001   // invalid device name
#define CUPKEE_EINVAL           20002   // invalid argument
#define CUPKEE_ERESOURCE        20003   // not enought resource

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
val_t native_led(env_t *env, int ac, val_t *av);
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

val_t native_enable(env_t *env, int ac, val_t *av);
val_t native_config(env_t *env, int ac, val_t *av);
val_t native_read(env_t *env, int ac, val_t *av);
val_t native_write(env_t *env, int ac, val_t *av);

// GPIO api
val_t native_pin(env_t *env, int ac, val_t *av);

#endif /* __CUPKEE_INC__ */

