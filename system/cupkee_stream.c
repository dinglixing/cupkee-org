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
    if (stream_is_writable(s) && !(s->flags & CUPKEE_STREAM_FL_TX_SHUTDOWN)) {
        if (s->tx_buf) {
            return cupkee_buffer_space(s->tx_buf);
        } else {
            return s->tx_size_max;
        }
    }

    return 0;
}

void cupkee_stream_shutdown(cupkee_stream_t *s, uint8_t flags)
{
    if (stream_is_readable(s) && flags & CUPKEE_STREAM_FL_READABLE) {
        s->flags |= CUPKEE_STREAM_FL_RX_SHUTDOWN;
        if (!s->rx_buf) {
            stream_event_emit(s, CUPKEE_EVENT_STREAM_END);
        } else
        if (0 == cupkee_buffer_length(s->rx_buf)) {
            stream_event_emit(s, CUPKEE_EVENT_STREAM_END);
            cupkee_buffer_release(s->rx_buf);
            s->rx_buf = NULL;
        }
    }
    if (stream_is_writable(s) && flags & CUPKEE_STREAM_FL_WRITABLE) {
        s->flags |= CUPKEE_STREAM_FL_TX_SHUTDOWN;
        if (!s->tx_buf) {
            stream_event_emit(s, CUPKEE_EVENT_STREAM_FINISH);
        } else
        if (0 == cupkee_buffer_length(s->tx_buf)) {
            stream_event_emit(s, CUPKEE_EVENT_STREAM_FINISH);
            cupkee_buffer_release(s->tx_buf);
            s->tx_buf = NULL;
        }
    }
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

    if (!stream_is_readable(s) || !n || !data) {
        return 0;
    }

    if (s->flags & (CUPKEE_STREAM_FL_RX_SHUTDOWN | CUPKEE_STREAM_FL_RX_BLOCKED)) {
        return 0;
    }

    if (!s->rx_buf && (NULL == (s->rx_buf = cupkee_buffer_alloc(s->rx_size_max)))) {
        return -CUPKEE_ENOMEM;
    }

    if (s->rx_state == CUPKEE_STREAM_STATE_FLOWING && cupkee_buffer_is_empty(s->rx_buf)) {
        stream_event_emit(s, CUPKEE_EVENT_STREAM_DATA);
    }

    cnt = cupkee_buffer_give(s->rx_buf, n, data);
    if (cupkee_buffer_is_full(s->rx_buf)) {
        s->flags |= CUPKEE_STREAM_FL_RX_BLOCKED;
    }

    return cnt;
}

int cupkee_stream_pull(cupkee_stream_t *s, size_t n, void *data)
{
    if (stream_is_writable(s) && s->tx_buf && n && data) {
        int cnt = cupkee_buffer_take(s->tx_buf, n, data);

        if (s->flags & CUPKEE_STREAM_FL_TX_SHUTDOWN) {
            if (cupkee_buffer_is_empty(s->tx_buf)) {
                stream_event_emit(s, CUPKEE_EVENT_STREAM_FINISH);
                cupkee_buffer_release(s->tx_buf);
                s->tx_buf = NULL;
            }
        } else
        if (cnt > 0 && (s->flags & CUPKEE_STREAM_FL_TX_BLOCKED)) {
            s->flags &= ~CUPKEE_STREAM_FL_TX_BLOCKED;
            stream_event_emit(s, CUPKEE_EVENT_STREAM_DRAIN);
        }
        return cnt;
    }
    return 0;
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

void cupkee_stream_pause(cupkee_stream_t *s)
{
    if (stream_is_readable(s)) {
        s->rx_state = CUPKEE_STREAM_STATE_PAUSED;
    }
}

void cupkee_stream_resume(cupkee_stream_t *s)
{
    if (stream_is_readable(s) && s->rx_state != CUPKEE_STREAM_STATE_FLOWING) {
        s->rx_state = CUPKEE_STREAM_STATE_FLOWING;
        if (s->rx_buf) {
            stream_rx_request(s, cupkee_buffer_space(s->rx_buf));
        } else {
            stream_rx_request(s, s->rx_size_max);
        }
    }
}

int cupkee_stream_read(cupkee_stream_t *s, size_t n, void *buf)
{
    int cnt;

    if (!stream_is_readable(s) || !buf) {
        return -CUPKEE_EINVAL;
    }

    if (s->rx_state == CUPKEE_STREAM_STATE_IDLE) {
        s->rx_state = CUPKEE_STREAM_STATE_PAUSED;
        stream_rx_request(s, n);
    }

    if (!s->rx_buf) {
        return 0;
    }

    if (cupkee_buffer_is_empty(s->rx_buf)) {
        stream_rx_request(s, n);
        return 0;
    }

    cnt = cupkee_buffer_take(s->rx_buf, n, buf);
    if (s->flags & CUPKEE_STREAM_FL_RX_SHUTDOWN) {
        if (cupkee_buffer_is_empty(s->rx_buf)) {
            stream_event_emit(s, CUPKEE_EVENT_STREAM_END);
            cupkee_buffer_release(s->rx_buf);
            s->rx_buf = NULL;
        }
    } else
    if (cnt >0 && s->flags & CUPKEE_STREAM_FL_RX_BLOCKED) {
        s->flags &= ~CUPKEE_STREAM_FL_RX_BLOCKED;
        stream_rx_request(s, cupkee_buffer_space(s->rx_buf));
    }
    return cnt;
}

int cupkee_stream_write(cupkee_stream_t *s, size_t n, const void *data)
{
    int should_trigger = 0;
    int cached;

    if (!stream_is_writable(s) || !data) {
        return -CUPKEE_EINVAL;
    }

    if (s->flags & CUPKEE_STREAM_FL_TX_SHUTDOWN) {
        return 0;
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

