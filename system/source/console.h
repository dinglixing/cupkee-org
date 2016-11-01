
#ifndef __CONSOLE_INC__
#define __CONSOLE_INC__

#define CON_IN              0
#define CON_OUT             1
#define CON_BUFF_MAX        2
#define CON_BUFF_SIZE       256
#define CON_PREVENT_DEFAULT 1

typedef enum console_ctrl_t{
    CON_CTRL_IDLE = 0,
    CON_CTRL_CHAR = 1,
    CON_CTRL_BACKSPACE,
    CON_CTRL_DELETE,
    CON_CTRL_TABLE,
    CON_CTRL_ENTER,
    CON_CTRL_ESCAPE,
    CON_CTRL_UP,
    CON_CTRL_DOWN,
    CON_CTRL_RIGHT,
    CON_CTRL_LEFT,
    CON_CTRL_F1,
    CON_CTRL_F2,
    CON_CTRL_F3,
    CON_CTRL_F4
} console_ctrl_t;


int console_init(void);
int console_handle_register(int (*handle)(int));

int console_put(char c);
int console_get(void);

int console_puts(const char *s);
int console_gets(char *buf, int max);

int  console_input_curr_tok(char *buf, int size);
int  console_input_peek(int pos);
void console_input_clear(void);
void console_input_string(char *s);

#endif /* __CONSOLE_INC__ */

