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

#include "cupkee.h"

#include "cupkee_sysdisk.h"

static void cupkee_event_process(void)
{
    cupkee_event_t e;

    while (cupkee_event_take(&e)) {
        /* Cupkee process */
        if (e.type == EVENT_SYSTICK) {
            cupkee_device_sync(_cupkee_systicks);
            cupkee_timer_sync(_cupkee_systicks);
        } else
        if (e.type == EVENT_DEVICE) {
            cupkee_device_event_handle(e.which, e.code);
            continue;
        } else
        if (e.type == EVENT_EMITTER) {
            cupkee_event_emitter_dispatch(e.which, e.code);
        }
    }
}

void cupkee_init(void)
{
    /* Hardware startup */
    hw_setup();

    /* Memory pool initial */
    cupkee_memory_init(0, NULL);

    /* System timer initial */
    cupkee_timer_init();

    /* Devices initial */
    cupkee_device_init();

    /* Sysdisk initial */
    cupkee_sysdisk_init();

    /* Buffer initial */
    cupkee_buffer_init();

    /* Module initial */
    cupkee_module_init();

    /* Event initial */
    cupkee_event_setup();
}

void cupkee_loop(void)
{
    // Reset systick at first
    _cupkee_systicks = 0;

    while (1) {
        cupkee_device_poll();

        cupkee_event_process();
    }
}

