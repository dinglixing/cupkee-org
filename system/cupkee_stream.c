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

static inline int stream_is_writable(cupkee_stream_t *s) {
    return s && (s->flags & CUPKEE_STREAM_FL_WRITABLE);
}

static inline void stream_rx_request(cupkee_stream_t *s, size_t n) {
    s->_read(s, n);
}

static inline void stream_tx_request(cupkee_stream_t *s) {
    s->_write(s);
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
    s->flags = flags;
    s->rx_state = CUPKEE_STREAM_STATE_IDLE;

    s->event_offset = 0;

    s->emitter = emitter;

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
   size_t buf_max_size,
   void (*_read)(cupkee_stream_t *s, size_t n)
) {
    if (!s || !_read) {
        return -CUPKEE_EINVAL;
    }

    stream_init(s, emitter, CUPKEE_STREAM_FL_READABLE);

    s->rx_size_max = buf_max_size;
    s->_read = _read;

    return CUPKEE_OK;
}

int cupkee_stream_init_writable(
   cupkee_stream_t *s,
   cupkee_event_emitter_t *emitter,
   size_t buf_max_size,
   void (*_write)(cupkee_stream_t *s)
) {
    if (!s || !_write) {
        return -CUPKEE_EINVAL;
    }

    stream_init(s, emitter, CUPKEE_STREAM_FL_WRITABLE);

    s->tx_size_max = buf_max_size;
    s->_write = _write;

    return CUPKEE_OK;
}

int cupkee_stream_init_duplex(
   cupkee_stream_t *s,
   cupkee_event_emitter_t *emitter,
   size_t rx_buf_max_size,
   size_t tx_buf_max_size,
   void (*_read)(cupkee_stream_t *s, size_t n),
   void (*_write)(cupkee_stream_t *s)
) {
    if (!s || !_read || !_write) {
        return -CUPKEE_EINVAL;
    }

    stream_init(s, emitter, CUPKEE_STREAM_FL_WRITABLE | CUPKEE_STREAM_FL_READABLE);

    s->rx_size_max = rx_buf_max_size;
    s->tx_size_max = tx_buf_max_size;
    s->_read = _read;
    s->_write = _write;

    return CUPKEE_OK;
}

int cupkee_stream_readable(cupkee_stream_t *s)
{
    if (stream_is_readable(s) && s->rx_buf) {
        return cupkee_buffer_length(s->rx_buf);
    }

    return 0;
}

int cupkee_stream_writable(cupkee_stream_t *s)
{
    if (stream_is_writable(s)) {
        if (s->tx_buf) {
            return cupkee_buffer_space(s->tx_buf);
        } else {
            return s->tx_size_max;
        }
    }

    return 0;
}

int cupkee_stream_rx_cache_space(cupkee_stream_t *s)
{
    if (stream_is_readable(s)) {
        if (s->rx_buf) {
            return cupkee_buffer_space(s->rx_buf);
        } else {
            return s->rx_size_max;
        }
    }
    return -CUPKEE_EINVAL;
}

int cupkee_stream_tx_cache_space(cupkee_stream_t *s)
{
    if (stream_is_writable(s)) {
        if (s->tx_buf) {
            return cupkee_buffer_space(s->tx_buf);
        } else {
            return s->rx_size_max;
        }
    }
    return -CUPKEE_EINVAL;
}

void cupkee_stream_set_error(cupkee_stream_t *s, uint8_t code)
{
    if (s) {
        s->error_code = code;
        if (code) {
            stream_event_emit(s, CUPKEE_EVENT_STREAM_ERROR);
        }
    }
}

int cupkee_stream_get_error(cupkee_stream_t *s)
{
    if (s) {
        return -s->error_code;
    } else {
        return -CUPKEE_EINVAL;
    }
}

int cupkee_stream_push(cupkee_stream_t *s, size_t n, const void *data)
{
    int cnt = 0;

    if (s && s->rx_buf && n && data) {
        cnt = cupkee_buffer_give(s->rx_buf, n, data);
        if (s->rx_state == CUPKEE_STREAM_STATE_FLOWING && !(s->flags & CUPKEE_STREAM_FL_RX_UPDATE)) {
            s->flags |= CUPKEE_STREAM_FL_RX_UPDATE;
            stream_event_emit(s, CUPKEE_EVENT_STREAM_DATA);
        }

        if (cupkee_buffer_is_full(s->rx_buf)) {
            s->flags |= CUPKEE_STREAM_FL_RX_FULL;
        }
    }

    return cnt;
}

int cupkee_stream_pull(cupkee_stream_t *s, size_t n, void *data)
{
    int cnt = 0;

    if (s && s->tx_buf && n && data) {
        cnt = cupkee_buffer_take(s->tx_buf, n, data);

        if (cnt > 0 && (s->flags & CUPKEE_STREAM_FL_TX_BLOCKED)) {
            s->flags &= ~CUPKEE_STREAM_FL_TX_BLOCKED;
            stream_event_emit(s, CUPKEE_EVENT_STREAM_DRAIN);
        }
    }
    return cnt;
}

int cupkee_stream_unshift(cupkee_stream_t *s, uint8_t data)
{
    if (!stream_is_readable(s)) {
        return -CUPKEE_EINVAL;
    }

    if (s->rx_buf) {
        return cupkee_buffer_unshift(s->rx_buf, data);
    } else {
        return 0;
    }
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

int cupkee_stream_write(cupkee_stream_t *s, size_t n, const void *data)
{
    int should_trigger = 0;
    int cached;

    if (!stream_is_writable(s) || !data) {
        return -CUPKEE_EINVAL;
    }

    if (!s->tx_buf) {
        if (NULL == (s->tx_buf = cupkee_buffer_alloc(s->tx_size_max))) {
            return -CUPKEE_ENOMEM;
        }
        should_trigger = 1;
    } else
    if (cupkee_buffer_is_empty(s->tx_buf)) {
        should_trigger = 1;
    }

    cached = cupkee_buffer_give(s->tx_buf, n, data);
    if (cached != (int) n) {
        s->flags |= CUPKEE_STREAM_FL_TX_BLOCKED;
    }

    if (should_trigger) {
        stream_tx_request(s);
    }

    return cached;
}

