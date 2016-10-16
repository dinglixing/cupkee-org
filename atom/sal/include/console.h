
#ifndef __CONSOLE_INC__
#define __CONSOLE_INC__

#define CON_IN          0
#define CON_OUT         1
#define CON_BUFF_MAX    2
#define CON_BUFF_SIZE 128

int console_init(void);

int console_put(char c);
int console_get(void);

int console_puts(const char *s);
int console_gets(char *buf, int max);

#endif /* __CONSOLE_INC__ */

