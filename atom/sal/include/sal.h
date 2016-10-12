
#ifndef __SAL_INC__
#define __SAL_INC__

#include <hal.h>
#include <stdint.h>

int sal_init(void);
void sal_loop(void);

int  sal_console_ready(void);
void sal_console_output(const char *s);
const char *sal_console_getline(void);

#endif /* __SAL_INC__ */

