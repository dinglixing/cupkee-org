/*
 * This file is part of the cupkee project.
 *
 * Copyright (C) 2016 ding.lixing@gmail.com
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
#include "atom.h"
#include "atomport-private.h"
#include "atomtimer.h"

#define IDLE_STACK_SIZE_BYTES       512
#define MAIN_STACK_SIZE_BYTES       4096

/* Main thread's stack area */
static uint8_t main_thread_stack[MAIN_STACK_SIZE_BYTES];

/* Idle thread's stack area */
static uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];

static void main_thread_func (uint32_t data __maybe_unused)
{
    int sleep_ticks = SYSTEM_TICKS_PER_SEC;

    while (1)
    {
        hal_led_toggle();
        atomTimerDelay(sleep_ticks);
    }
}

static ATOM_TCB main_tcb;
static void sal_setup(void (*main_task)(uint32_t))
{
    int status;

    /**
     * Initialise the OS before creating our threads.
     *
     * Note that we cannot enable stack-checking on the idle thread on
     * this platform because we are already using part of the idle
     * thread's stack now as our startup stack. Prefilling for stack
     * checking would overwrite our current stack.
     *
     * If you are not reusing the idle thread's stack during startup then
     * you are free to enable stack-checking here.
     */
    status = atomOSInit(&idle_thread_stack[0], IDLE_STACK_SIZE_BYTES, FALSE);
    if (status == ATOM_OK)
    {

        /* Create an application thread */
        status = atomThreadCreate(&main_tcb,
                     16, main_task, 0,
                     &main_thread_stack[0],
                     MAIN_STACK_SIZE_BYTES,
                     TRUE);
        if (status == ATOM_OK)
        {
            /**
             * First application thread successfully created. It is
             * now possible to start the OS. Execution will not return
             * from atomOSStart(), which will restore the context of
             * our application thread and start executing it.
             *
             * Note that interrupts are still disabled at this point.
             * They will be enabled as we restore and execute our first
             * thread in archFirstThreadRestore().
             */
            atomOSStart();
        }
    }

    while (1)
        ;
}

int main(void)
{
    hal_setup();
    sal_setup(main_thread_func);

    /* There was an error starting the OS if we reach here */
    return (0);
}

