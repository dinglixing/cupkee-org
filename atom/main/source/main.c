/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hal.h"
#include "sal.h"
#include "shell.h"

//static const char *logo = "CUPKEE\r\n> ";

static const char *logo = "\
 _________               __                  \r\n\
 \\_   ___ \\ __ ________ |  | __ ____   ____  \r\n\
 /    \\  \\/|  |  \\____ \\|  |/ // __ \\_/ __ \\ \r\n\
 \\     \\___|  |  /  |_> >    <\\  ___/\\  ___/ \r\n\
  \\________/____/|   __/|__|_ \\\\____> \\____>\r\n\
                 |__|        \\/ ATOM v0.0.1\r\n";

int main(void)
{
    hal_init();
    sal_init();

    if (0 != shell_init()) {
        hal_halt();
    }

    while (1) {
        int e;
        while (EVENT_IDLE != (e = event_get())) {
            switch (e) {
            case EVENT_CONSOLE_READY:
                hal_console_sync_puts(logo);
                break;
            case EVENT_CONSOLE_INPUT:
                shell_execute();
                break;
            case EVENT_SYSTICK_OCCUR:
                shell_timeout_execute();
                break;
            default:
                break;
            }
        }
        sal_loop();
    }
}

