#ifndef __CUPKEE_HISTORY_INC__
#define __CUPKEE_HISTORY_INC__

int cupkee_history_init(void);
int cupkee_history_push(int len, const char *data);
int cupkee_history_load(int dir);

#endif /* __CUPKEE_HISTORY_INC__ */
