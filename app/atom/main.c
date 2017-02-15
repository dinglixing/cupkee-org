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

#include <cupkee.h>

static const native_t native_entry[] = {
    /* panda natives */
    {"Buffer",          buffer_native_create},

    /* cupkee natives */
#if 0
    {"sysinfos",        native_sysinfos},
    {"systicks",        native_systicks},
    {"scripts",         native_scripts},
    {"show",            native_show},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"Device",          device_native_create},
    {"pinMap",          cupkee_native_pin_map},
    {"ledMap",          cupkee_native_led_map},
    {"led",             cupkee_native_led},
#endif

    /* user native */

};

// system initial scripts
static char *initial = NULL;

int main(void)
{

    /**********************************************************
     * Cupkee system initial
     *********************************************************/
    cupkee_init();

    /**********************************************************
     * user setup code
     *********************************************************/


    /**********************************************************
     * Cupkee shell initial
     *********************************************************/
    cupkee_shell_init(sizeof(native_entry)/sizeof(native_t), native_entry);

    /**********************************************************
     * Let's Go!
     *********************************************************/
    cupkee_shell_loop(initial);

    /**********************************************************
     * Let's Go!
     *********************************************************/
    return 0;
}

