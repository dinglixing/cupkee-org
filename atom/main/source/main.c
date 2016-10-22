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

#include "cupkee.h"

static const native_t native_entry[] = {
    /* cupkee native */
    {"sysinfos",        native_sysinfos},
    {"systicks",        native_systicks},

    {"print",           native_print},
    {"led",             native_led},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"scripts",         native_scripts},

    /* user native */
};

int main(void)
{
    cupkee_init();

    /* user code here */

    cupkee_set_native(native_entry, sizeof(native_entry)/sizeof(native_t));
    return cupkee_loop();
}

