/*
MIT License

This file is part of cupkee project.

Copyright (c) 2017 Lixing Ding <ding.lixing@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __CUPKEE_STREAM_INC__
#define __CUPKEE_STREAM_INC__

enum {
    CUPKEE_EVENT_STREAM_ERROR,   // rw
    CUPKEE_EVENT_STREAM_DATA,    // r
    CUPKEE_EVENT_STREAM_DRAIN,   //  w
    CUPKEE_EVENT_STREAM_READY,   // r
    CUPKEE_EVENT_STREAM_CLOSE,   // rw
    CUPKEE_EVENT_STREAM_END,     // r
    CUPKEE_EVENT_STREAM_FINISH,  //  w
    CUPKEE_EVENT_STREAM_MAX
};

typedef struct cupkee_stream_t cupkee_stream_t;

typedef void (*cupkee_stream_do_read) (cupkee_stream_t *s, size_t n);
typedef void (*cupkee_stream_do_write)(cupkee_stream_t *s, size_t n, void *data);
typedef void (*cupkee_stream_event_handler)(cupkee_stream_t *s, int event, void *param);

struct cupkee_stream_t {
    cupkee_event_emitter_t emitter;
    uint8_t flags;
    uint8_t rx_state;
    uint8_t tx_state;
    uint8_t event_once;

    void *rx_buf;
    void *tx_buf;
    void (*do_read) (cupkee_stream_t *s, size_t n);
    void (*do_write)(cupkee_stream_t *s, size_t n, void *data);
    void (*event_handler)(cupkee_stream_t *, int event, void *param);
    void *event_params[CUPKEE_EVENT_STREAM_MAX];
};

/* Call in do_read */
int _cupkee_stream_push(cupkee_stream_t *s, size_t n, uint8_t *data);

/* Call in do_write */
int _cupkee_stream_write_done(cupkee_stream_t *s, int err);

int cupkee_stream_init(
    cupkee_stream_t *stream,
    void (*do_read) (cupkee_stream_t *, size_t),
    void (*do_write)(cupkee_stream_t *, size_t, void *),
    void (*event_handler)(cupkee_stream_t *, int, void *)
);

int cupkee_stream_error(cupkee_stream_t *s, int err);

int cupkee_stream_event_on  (cupkee_stream_t *s, int event, void *param);
int cupkee_stream_event_once(cupkee_stream_t *s, int event, void *param);

void *cupkee_stream_read(cupkee_stream_t *s, size_t n);
int cupkee_stream_write(cupkee_stream_t *s, size_t n, const uint8_t *data);

int cupkee_stream_pipe(cupkee_stream_t *src, cupkee_stream_t *dst);
int cupkee_stream_unpipe(cupkee_stream_t *src);

int cupkee_stream_linkup(cupkee_stream_t *src, cupkee_stream_t *dst);
int cupkee_stream_unlink(cupkee_stream_t *src);

#endif /* __CUPKEE_STREAM_INC__ */

