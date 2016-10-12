/*
 * Copyright (c) 2016, ding.lixing@gmail.com. All rights reserved.
 */

#include <stdbool.h>

#include "hal.h"
#include "atomport.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>


//#include "atomport.h"

/**
 * Set up the core clock to something other than the internal 16MHz PIOSC.
 * Make sure that you use the same clock frequency in  systick_setup().
 */
static void clock_setup(void)
{
    /* set core clock to 72MHz, generated from external 8MHz crystal */
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
}

/**
 * initialise and start SysTick counter. This will trigger the
 * sys_tick_handler() periodically once interrupts have been enabled
 * by archFirstThreadRestore()
 */
static void systick_setup(void)
{
    systick_set_frequency(SYSTEM_TICKS_PER_SEC, 72000000);

    systick_interrupt_enable();

    systick_counter_enable();
}

/**
 * Set up user LED and provide function for toggling it. This is for
 * use by the test suite programs
 */
static void led_setup(void)
{
    /* LED is connected to GPIO5 on port B */
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);

    gpio_set(GPIOB, GPIO5);
}

void hal_led_toggle(void)
{
    gpio_toggle(GPIOB, GPIO5);
}

void hal_led_on(void)
{
    gpio_set(GPIOB, GPIO5);
}

void hal_led_off(void)
{
    gpio_clear(GPIOB, GPIO5);
}


/**
 * Callback from your main program to set up the board's hardware before
 * the kernel is started.
 */
int hal_setup(void)
{
    /* Disable interrupts. This makes sure that the sys_tick_handler will
     * not be called before the first thread has been started.
     * Interrupts will be enabled by archFirstThreadRestore().
     */
    cm_mask_interrupts(true);

    /* configure system clock */
    clock_setup();

    /* initialise SysTick counter */
    systick_setup();

    /* initalise user LED */
    led_setup();

    /* Set exception priority levels. Make PendSv the lowest priority and
     * SysTick the second to lowest
     */
    nvic_set_priority(NVIC_PENDSV_IRQ, 0xFF);
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0xFE);

    return 0;
}

