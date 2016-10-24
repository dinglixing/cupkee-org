#ifndef __HW_MOCK_INC__
#define __HW_MOCK_INC__

#include <stdint.h>

void hw_mock_reset(void);

void hw_mock_systicks_set(uint32_t x);

void hw_mock_console_give(const char *data);
void hw_mock_console_buf_reset(void);
int  hw_mock_console_reply(char **ptr);

#endif /* __HW_MOCK_INC__ */
