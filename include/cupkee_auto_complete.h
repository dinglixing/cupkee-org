#ifndef __CUPKEE_AUTO_COMPLETE__
#define __CUPKEE_AUTO_COMPLETE__

int cupkee_auto_complete(int symbal_num, const char **symbals);

void *cupkee_auto_complete_init(void *buf, unsigned size);
void cupkee_auto_complete_update(void *buf, const char *symbal);
int  cupkee_auto_complete_finish(void *buf);

#endif /* __CUPKEE_AUTO_COMPLETE__ */

