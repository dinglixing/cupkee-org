#include "device.h"

int device_pin_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pin_t *pin = (hw_config_pin_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_PIN_CONF_NUM:   val_set_number(val, pin->num);   break;
    case DEVICE_PIN_CONF_START: val_set_number(val, pin->start); break;
    case DEVICE_PIN_CONF_DIR:   device_get_option(val, pin->dir, DEVICE_OPT_DIR_MAX, device_opt_dir); break;
    default:                    return -CUPKEE_EINVAL;
    }

    return CUPKEE_OK;
}

int device_pin_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pin_t *pin = (hw_config_pin_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_PIN_CONF_NUM:   return device_set_uint8(val, &pin->num);
    case DEVICE_PIN_CONF_START: return device_set_uint8(val, &pin->start);
    case DEVICE_PIN_CONF_DIR:   return device_set_option(val, &pin->dir, DEVICE_OPT_DIR_MAX, device_opt_dir);
    default:                    return -CUPKEE_EINVAL;
    }
}

