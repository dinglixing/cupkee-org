
#ifndef __CONSOLE_INC__
#define __CONSOLE_INC__

#define CON_IN          0
#define CON_OUT         1
#define CON_BUFF_MAX    2
#define CON_BUFF_SIZE 128

int console_init(void);
int console_handle_register(int (*handle)(int));


int console_put(char c);
int console_get(void);

int console_puts(const char *s);
int console_gets(char *buf, int max);

int  console_input_curr_tok(char *buf, int size);
void console_input_clear(void);
void console_input_string(char *s);

#endif /* __CONSOLE_INC__ */

