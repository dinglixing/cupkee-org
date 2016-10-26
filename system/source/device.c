#include <cupkee.h>
#include "device.h"

static int device_is_valid(val_t *d)
{
    (void) d;

    return 1;
}

static val_t device_config_get(val_t *hnd, env_t *env, val_t *which)
{
    (void) hnd;

    if (which) {
        const char *item = val_2_cstring(which);

        if (!strcmp(item, "select")) {
            intptr_t sel = array_create(env, 0, NULL);
            if (sel) {
                return val_mk_array((void *)sel);
            } else {
                // return error_create(env, code, msg);
                return val_mk_undefined();
            }
        } else
        if (!strcmp(item, "dir")) {
            return val_mk_static_string((intptr_t)"out");
        } else
        if (!strcmp(item, "mode")) {
            return val_mk_static_string((intptr_t)"push-pull");
        } else
        if (!strcmp(item, "enable")) {
            return val_mk_boolean(0);
        } else {
            return val_mk_undefined();
        }
    } else {
        // all items compose a dictionary
        return val_mk_undefined();
    }
}

static val_t device_config_set(val_t *hnd, env_t *env, val_t *which, val_t *setting)
{
    (void) hnd;
    (void) env;
    (void) which;
    (void) setting;

    return val_mk_boolean(1);
}

void device_init(void)
{
    return;
}

val_t native_device(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;

    return val_mk_undefined();
}

val_t native_config(env_t *env, int ac, val_t *av)
{
    val_t *hnd;

    (void) env;

    if (ac == 0 || !device_is_valid(av)) {
        return val_mk_undefined();
    } else {
        hnd = av++; ac--;
    }

    if (ac == 0) {
        return device_config_get(hnd, env, NULL);
    } else {
        if (ac == 1) {
            if (val_is_string(av)) {
                return device_config_get(hnd, env, av);
            } else {
                return device_config_set(hnd, env, NULL, av);
            }
        } else {
            return device_config_set(hnd, env, av, av + 1);
        }
    }
}

