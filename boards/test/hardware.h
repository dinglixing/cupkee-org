#ifndef __HW_MOCK_INC__
#define __HW_MOCK_INC__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

void hw_reset(void);

void hw_systicks_set(uint32_t x);

void hw_console_reset(void);
void hw_console_give(const char *data);
int  hw_console_reply(char **ptr);
void hw_console_set_cb(void (*input)(void *, int), void (*drain)(void));
int  hw_console_putc(int ch);
void hw_console_buf_clear(void);

int hw_scripts_erase(void);
int hw_scripts_remove(int id);
int hw_scripts_save(const char *s);
const char *hw_scripts_load(const char *prev);

#endif /* __HW_MOCK_INC__ */
