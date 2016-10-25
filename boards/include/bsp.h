

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
    unsigned sys_freq;
    unsigned sys_ticks_pre_sec;
} hal_info_t;

extern uint32_t system_ticks_count;

void board_setup(void);

int hal_memory_alloc(void **p, int size, int align);

void hal_poll(void);
void hal_halt(void);
void hal_info_get(hal_info_t *);

int hal_console_set_cb(void (*input)(void *, int), void (*drain)(void));
int hal_console_write_byte(char c);
int hal_console_puts(const char *s);
int hal_console_write_sync_byte(char c);
int hal_console_sync_puts(const char *s);

int hal_scripts_erase(void);
int hal_scripts_remove(int id);
int hal_scripts_save(const char *s);
const char *hal_scripts_load(const char *prev);

void hal_led_on(void);
void hal_led_off(void);
void hal_led_toggle(void);

#endif /* __BSP_INC__ */

