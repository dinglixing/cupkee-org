/*
 * This file is part of the cupkee project.
 *
 * Copyright (C) 2016 Lixing Ding <ding.lixing@gmail.com>
 *
 */

#include "cupkee.h"

static const native_t native_entry[] = {
    /* cupkee native */
    {"sysinfos",        native_sysinfos},
    {"systicks",        native_systicks},

    {"print",           native_print},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"scripts",         native_scripts},

    {"pin",             gpio_native_pin},
    {"device",          native_device},
    {"enable",          device_native_enable},
    {"config",          device_native_config},
    {"write",           device_native_write},
    {"read",            device_native_read},
    {"listen",          device_native_listen},
    {"ignore",          device_native_ignore},

    /* user native */

};

// system initial scripts
static char *initial = "\
var h = device('GPIO');\
var v = 0;\
enable(h, {\
  pin: pin('a', 8),\
  mode: 'out-pushpull'\
});\
def led() {write(h, v++)}\
setInterval(led, 1000);";

int main(void)
{
    cupkee_init();

    /* user code here */

    cupkee_set_native(native_entry, sizeof(native_entry)/sizeof(native_t));
    cupkee_start(initial);

    while (1)
        cupkee_poll();

    // Should not go here
    return 0;
}

