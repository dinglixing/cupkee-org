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

#include <cupkee.h>

static inline int stream_is_readable(cupkee_stream_t *s) {
    return s && (s->flags & CUPKEE_STREAM_FL_READABLE);
}

static inline void stream_rx_request(cupkee_stream_t *s, size_t n) {
    s->_read(s, n);
}

static inline int cupkee_stream_rx_buf_create(cupkee_stream_t *s) {
    if (s->rx_buf) {
        cupkee_buffer_release(s->rx_buf);
        // Todo: set overflow error event?
    }
    s->rx_buf = cupkee_buffer_alloc(s->rx_size_max);

    return s->rx_buf ? CUPKEE_OK : -CUPKEE_ENOMEM;
}

static void stream_init(cupkee_stream_t *s, cupkee_event_emitter_t *emitter, uint8_t flags)
{
    s->emitter = emitter;
    s->event_offset = 0;
    s->flags = flags;

    s->rx_state = CUPKEE_STREAM_STATE_IDLE;

    s->rx_size_max = 0;
    s->tx_size_max = 0;

    s->rx_buf = NULL;
    s->tx_buf = NULL;

    s->_read = NULL;
    s->_write = NULL;
}

static void stream_event_emit(cupkee_stream_t *s, uint8_t code)
{
    if (s->emitter) {
        cupkee_event_emitter_emit(s->emitter, s->event_offset + code);
    }
}

int cupkee_stream_init_readable(
   cupkee_stream_t *s,
   cupkee_event_emitter_t *emitter,
   size_t buf_size,
   void (*_read)(cupkee_stream_t *s, size_t n)
) {
    if (!s || !_read) {
        return -CUPKEE_EINVAL;
    }

    stream_init(s, emitter, CUPKEE_STREAM_FL_READABLE);

    s->rx_size_max = buf_size;
    s->_read = _read;

    return CUPKEE_OK;
}

int cupkee_stream_readable(cupkee_stream_t *s)
{
    if (s && (s->flags & CUPKEE_STREAM_FL_READABLE) && s->rx_buf) {
        return cupkee_buffer_length(s->rx_buf);
    }

    return 0;
}

int cupkee_stream_push(cupkee_stream_t *s, size_t n, const void *data)
{
    int cnt = 0;

    if (s && s->rx_buf) {
        if (cupkee_buffer_is_full(s->rx_buf)) {
            s->flags |= CUPKEE_STREAM_FL_RX_FULL;
        } else {
            cnt = cupkee_buffer_give(s->rx_buf, n, data);
            if (s->rx_state == CUPKEE_STREAM_STATE_FLOWING && !(s->flags & CUPKEE_STREAM_FL_RX_UPDATE)) {
                s->flags |= CUPKEE_STREAM_FL_RX_UPDATE;
                stream_event_emit(s, CUPKEE_EVENT_STREAM_DATA);
            }
        }
    }

    return cnt;
}

int cupkee_stream_read(cupkee_stream_t *s, size_t n, void *buf)
{
    size_t cached;

    if (!stream_is_readable(s) || !buf) {
        return -CUPKEE_EINVAL;
    }

    if (s->rx_state == CUPKEE_STREAM_STATE_IDLE) {
        if (0 != cupkee_stream_rx_buf_create(s)) {
            return -CUPKEE_ENOMEM;
        }

        s->rx_state = CUPKEE_STREAM_STATE_PAUSED;
        stream_rx_request(s, s->rx_size_max);
    } else
    if (!s->rx_buf) {
        return -CUPKEE_ENOMEM;
    }

    if (s->rx_size_max < n) {
        return 0;
    }

    cached = cupkee_buffer_length(s->rx_buf);
    if (n > cached) {
        return 0;
    }

    cupkee_buffer_take(s->rx_buf, n, buf);

    if (s->flags & CUPKEE_STREAM_FL_RX_FULL) {
        s->flags &= ~CUPKEE_STREAM_FL_RX_FULL;
        stream_rx_request(s, s->rx_size_max - cached + n);
    } else {
        s->flags &= ~(CUPKEE_STREAM_FL_RX_FULL | CUPKEE_STREAM_FL_RX_UPDATE);
    }
    return n;
}

