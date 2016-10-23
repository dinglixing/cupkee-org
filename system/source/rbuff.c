#include <string.h>
#include "rbuff.h"

void rbuff_init(rbuff_t *rb, int size, void *buf)
{
    rb->size = size;
    rb->head = 0;
    rb->cnt = 0;
    rb->ptr = buf;
}

int rbuff_shift(rbuff_t *rb)
{
    if (rb->cnt <= 0) {
        return -1;
    }

    int pos = rb->head++;
    if (rb->head >= rb->size) {
        rb->head = 0;
    }
    rb->cnt--;

    return pos;
}

int rbuff_unshift(rbuff_t *rb)
{
    if (rb->cnt >= rb->size) {
        return -1;
    }

    if (rb->head == 0) {
        rb->head = rb->size - 1;
    } else {
        rb->head--;
    }
    rb->cnt++;
    return rb->head;
}


int rbuff_push(rbuff_t *rb)
{
    if (rb->cnt >= rb->size) {
        return -1;
    }

    int pos = rb->head + rb->cnt++;
    if (pos >= rb->size) {
        pos %= rb->size;
    }
    return pos;
}

int rbuff_pop(rbuff_t *rb)
{
    if (rb->cnt <= 0) {
        return -1;
    }

    int pos = rb->head + rb->cnt--;
    if (pos >= rb->size) {
        pos %= rb->size;
    }
    return pos;
}

