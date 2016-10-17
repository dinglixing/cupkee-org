
#ifndef __RBUFF_INC__
#define __RBUFF_INC__

typedef struct rbuff_t {
    int size;
    int head;
    int  cnt;
    void *ptr;
} rbuff_t;

void rbuff_init(rbuff_t *rb, int size, void *buf);

int rbuff_shift(rbuff_t *rb);
int rbuff_unshift(rbuff_t *rb);
int rbuff_push(rbuff_t *rb);
int rbuff_pop(rbuff_t *rb);
int rbuff_insert(rbuff_t *rb, int pos);

static inline int rbuff_get(rbuff_t *rb, int pos)
{
    if (pos < 0 || pos >= rb->cnt) {
        return -1;
    }
    pos = rb->head + pos;
    if (pos >= rb->size) {
        pos = pos % rb->size;
    }
    return pos;
}


static inline void rbuff_reset(rbuff_t *rb) {
    rb->head = 0;
    rb->cnt = 0;
}

static inline int rbuff_remove(rbuff_t *rb, int n) {
    if (rb->cnt >= n) {
        rb->cnt -= n;
        return n;
    } else {
        return 0;
    }
}

static inline int rbuff_append(rbuff_t *rb, int n) {
    if (rb->cnt + n <= rb->size) {
        rb->cnt += n;
        return n;
    } else {
        return 0;
    }
}

static inline int rbuff_end(rbuff_t *rb) {
    return rb->cnt;
}

static inline int rbuff_is_empty(rbuff_t *rb) {
    return !rb->cnt;
}
static inline int rbuff_is_full(rbuff_t *rb) {
    return rb->cnt == rb->size;
}

#endif /* __RBUFF_INC__ */

