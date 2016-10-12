/*
 * This file is part of the cupkee project.
 *
 * Copyright (C) 2016 Lixing ding <ding.lixing@gmail.conz>
 *
 */

#include "sal.h"
#include "shell.h"
#include "native.h"

#define HEAP_SIZE     (1024 * 8)
#define STACK_SIZE    (256)
#define EXE_MEM_SPACE (1024 * 8)
#define SYM_MEM_SPACE (1024 * 2)
#define MEM_SIZE      (STACK_SIZE * sizeof(val_t) + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)

static uint8_t memory[MEM_SIZE];
static env_t shell_env;

int shell_init(void)
{
    if(0 != interp_env_init_interactive(&shell_env, memory, MEM_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE)) {
        //sal_console_output("env_init fail\n");
        return -1;
    }

    native_init(&shell_env);
    //sal_console_output("> ");

    return 0;
}

void shell_loop(void)
{
    const char  *line = sal_console_getline();
    val_t *res;
    int    err;

    if (!line) {
        return;
    }

    err = interp_execute_interactive(&shell_env, line, NULL, &res);
    if (err < 0) {
        print_error(-err);
        hal_led_on(HAL_LED_0);
    } else
    if (err > 0) {
        print_value(res);
    }
    sal_console_output("> ");
}

