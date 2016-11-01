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

    {"pin",             native_pin},
    {"device",          native_device},
    {"enable",          native_enable},
    {"config",          native_config},
    {"write",           native_write},
    {"read",            native_read},
    {"listen",          native_listen},
    {"ignore",          native_ignore},

    /* user native */

};

// system initial scripts
static char *initial = "\
var led = (function() { \
  var h = device('GPIO');\
  var v = 0;\
  config(h, 0, pin('a', 8));\
  config(h, 1, 'out-pushpull');\
  enable(h, 1);\
  \
  return def() {write(h, v++)}\
 })();\
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

