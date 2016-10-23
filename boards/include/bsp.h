

#ifndef __BSP_INC__
#define __BSP_INC__

#include <stdint.h>

#define SYSTEM_TICKS_PRE_SEC     1000
#define SYSTEM_STACK_SIZE        8 * 1024
typedef struct hal_info_t {
    int ram_sz;
    int rom_sz;
    void *ram_base;
    void *rom_base;
    uint32_t sys_freq;
    uint32_t sys_ticks_pre_sec;
} hal_info_t;

extern uint32_t system_ticks_count;

void board_setup(void);

int hal_memory_alloc(void **p, int size, int align);

void hal_loop(void);
void hal_halt(void);
void hal_info_get(hal_info_t *);

int hal_console_set_cb(void (*input)(void *, int), void (*drain)(void));
int hal_console_write_byte(char c);
int hal_console_puts(const char *s);
int hal_console_write_sync_byte(char c);
int hal_console_sync_puts(const char *s);

void hal_led_on(void);
void hal_led_off(void);
void hal_led_toggle(void);

int hal_storage_size_usr(void);
int hal_storage_erase_usr(void);
int hal_storage_clear_usr(const void *addr, int size);
int hal_storage_write_usr(const void *data, int size);
int hal_storage_valid_usr(const void *addr);
void *hal_storage_base_usr(void);

#endif /* __BSP_INC__ */

