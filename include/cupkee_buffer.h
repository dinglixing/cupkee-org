#ifndef __CUPKEE_BUFFER_INC__
#define __CUPKEE_BUFFER_INC__

void cupkee_buffer_init(void);

void *cupkee_buf_alloc(size_t size);
void cupkee_buf_release(void *b);
void cupkee_buf_reset(void *b);
int cupkee_buf_capacity(void *b);
int cupkee_buf_space(void *b);
int cupkee_buf_length(void *b);
int cupkee_buf_is_empty(void *b);
int cupkee_buf_is_full(void *b);
int cupkee_buf_push(void *b, uint8_t d);
int cupkee_buf_shift(void *b, uint8_t *d);
int cupkee_buf_take(void *b, int n, void *buf);
int cupkee_buf_give(void *b, int n, void *buf);

#endif /* __CUPKEE_BUFFER_INC__ */

