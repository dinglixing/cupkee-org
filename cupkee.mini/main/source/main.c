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
#include "lang.h"

//static const char *logo = "CUPKEE\r\n> ";

static const char *logo = "\
 _________               __                  \r\n\
 \\_   ___ \\ __ ________ |  | __ ____   ____  \r\n\
 /    \\  \\/|  |  \\____ \\|  |/ // __ \\_/ __ \\ \r\n\
 \\     \\___|  |  /  |_> >    <\\  ___/\\  ___/ \r\n\
  \\______  /____/|   __/|__|_ \\___  >\\___  >\r\n\
         \\/      |__|        \\/    \\/     \\/  V0.0.1\r\n";

int main(void)
{
    hal_init();
    sal_init();

    if (0 != lang_init()) {
        hal_halt();
    }

	while (!sal_console_ready()) {
        hal_loop();
        sal_loop();
    }

    sal_console_output(logo);

    while (1) {
        hal_loop();
        lang_loop();
    }
}

