
#include "hardware.h"
#include <bsp.h>

#define MAIN_STACK_SIZE 8192

extern vector_table_t vector_table;
extern char end;
static int memory_alloced = 0;
static int memory_size = 0;
static const hw_device_t *hw_devices[] = {
    &hw_device_pin,
    &hw_device_key,
    &hw_device_adc,
    &hw_device_usart,
};
static const hw_driver_t *hw_drivers[] = {
    &hw_driver_pin,
    &hw_driver_key,
    &hw_driver_adc,
    &hw_driver_usart,
};

static void hw_memory_init(void)
{
    memory_alloced = 0;
    memory_size = (char *)(vector_table.initial_sp_value) - (&end) - MAIN_STACK_SIZE;
}

static void hw_systick_setup(void)
{
    systick_set_frequency(SYSTEM_TICKS_PRE_SEC, 72000000);

    systick_interrupt_enable();
    systick_counter_enable();
}

/* systick interrupt handle routing  */
uint32_t system_ticks_count = 0;
void sys_tick_handler(void)
{
    system_ticks_count++;
}

int hw_memory_alloc(void **p, int size, int align)
{
    int start = (intptr_t) (&end) + memory_alloced;
    int shift = 0;

    if (size == 0) {
        return 0;
    }

    if (start % align) {
        shift = align - (start % align);
    } else {
        shift = 0;
    }
    memory_alloced += shift;

    if (size < 0) {
        size = memory_size - memory_alloced;
    } else
    if (memory_alloced + size > memory_size) {
        return -1;
    }

    if (p) {
        *p = &end + memory_alloced;
        memory_alloced += size;
    }

    return size;
}

void hw_info_get(hw_info_t *info)
{
    if (info) {
        info->sys_freq = 72000000;
        info->sys_ticks_pre_sec = SYSTEM_TICKS_PRE_SEC;
        info->ram_sz = 64 * 1024;
        info->rom_sz = desig_get_flash_size() * 1024;
        info->ram_base = (void *)0x20000000;
        info->rom_base = (void *)0x08000000;
    }
}

void hw_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

    hw_memory_init();

    hw_gpio_setup();
    hw_adc_setup();
    hw_usart_setup();
    hw_usb_setup();         // usb used as console
    hw_systick_setup();
    hw_storage_setup();
}

void hw_poll(void)
{
    static uint32_t system_ticks_count_pre = 0;

    hw_usb_poll();
    hw_usart_poll();

    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;

        systick_event_post();

        hw_gpio_poll();
        hw_adc_poll();
    }
}

void hw_halt(void)
{
    while (1) {
    }
}

const hw_device_t *hw_device_descript(int i)
{
    int max = sizeof(hw_devices) / sizeof(hw_device_t *);

    if (i < 0 || i >= max) {
        return NULL;
    }
    return hw_devices[i];
}

const hw_device_t *hw_device_take(const char *name, int inst, const hw_driver_t **driver)
{
    int i, max = sizeof(hw_devices) / sizeof(hw_device_t *);

    _TRACE("request %s[%d]\n", name, inst);
    for (i = 0; i < max; i++) {
        const hw_device_t *desc = hw_devices[i];
        if (!strcmp(name, desc->name)) {
            if (hw_drivers[i]->request(desc->id, inst)) {
                if (driver) {
                    *driver = hw_drivers[i];
                }
                return desc;
            }
            break;
        }
    }
    return NULL;
}

