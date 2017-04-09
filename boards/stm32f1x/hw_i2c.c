/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

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

#include "hardware.h"

static uint8_t use_map;

typedef struct hw_i2c_t {
    uint8_t dev_id;
    uint8_t state;
    uint8_t tout;
    uint8_t addr;
    void   *rx_buff;
    void   *tx_buff;
} hw_i2c_t;

static hw_i2c_t i2c_controls[HW_INSTANCES_I2C];

static inline int hw_i2c_match_event(uint32_t i2c, uint32_t event)
{
    uint32_t status = I2C_SR1(i2c);

    status = status | (I2C_SR2(i2c) << 16);

    return (status & event) == event;
}

static inline hw_i2c_t *hw_i2c_control(int instance)
{
    if (instance < HW_INSTANCES_I2C && (use_map & (1 << instance))) {
        return &i2c_controls[instance];
    } else {
        return NULL;
    }
}

static inline int hw_i2c_setup_pin(int instance)
{
    uint16_t pins;

    if (instance == 0) {
        pins = GPIO6 | GPIO7;
    } else {
        pins = GPIO6 | GPIO7;
        pins = GPIO10 | GPIO11;
    }

    if (hw_gpio_use(1, pins)) {
        return -1;
    }

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, pins);
    return 0;
}

static int request_append_read(struct inqueue_t *inq, int size, uint32_t off)
{
    void *buf;

    if (inq->buf) {
        return -CUPKEE_EFULL;
    }

    buf = cupkee_buf_alloc(size);
    if (buf) {
        inq->off  = off;
        inq->buf  = buf;
        return 0;
    }
    return -CUPKEE_ERESOURCE;
}

static int request_append_write(struct outqueue_t *outq, int size, uint32_t off, void *data)
{
    void *buf;

    if (outq->buf) {
        return -CUPKEE_EFULL;
    }

    buf = cupkee_buf_alloc(size);
    if (buf) {
        cupkee_buf_give(buf, size, data);
        outq->off  = off;
        outq->buf  = buf;
        return 0;
    }
    return -CUPKEE_ERESOURCE;
}

static int request_peek(hw_i2c_t *control)
{
}

static int request_shift(struct inqueue_t *inq, int size, uint32_t *off, void *buf)
{
    int n;

    if (!inq->buf) {
        return 0;
    }

    n = cupkee_buf_take(inq->buf, size, buf);
    if (n <= 0) {
        return 0;
    }

    if (*off) {
        *off = inq->off;
    }

    if (cupkee_buf_is_empty(inq->buf)) {
        cupkee_buf_release(inq->buf);
        inq->buf = NULL;
    } else {
        inq->off += n;
    }

    return n;
}

static void do_wait_nbusy(uint32_t i2c, hw_i2c_t *control)
{
    if (I2C_SR2(i2c) & I2C_SR2_BUSY) {
        return;
    }

    I2C_CR1(i2c) |= I2C_CR1_START;
    control->state = I2C_STATE_WAIT_START;
}

static void do_wait_start(uint32_t i2c, hw_i2c_t *control)
{
    if (!hw_i2c_match_event(i2c, I2C_EVENT_MASTER_MODE_SELECT)) {
        return;
    }

    req = request_peek(control);
    if (!req) {
        I2C_CR1(i2c) |= I2C_CR1_STOP;
        control->state = I2C_STATE_IDLE;
        return;
    }

    if (req->offset == 0xffff) {
        // simple read & write
        I2C_DR(i2c) = req->addr;
        control->state = I2C_STATE_WAIT_ADDR;
    } else {
        // offset read & write
        I2C_DR(i2c) = (req->addr & ~1);
        control->state = I2C_STATE_WAIT_OADDR;
    }
}

static void do_wait_addr(uint32_t i2c, hw_i2c_t *control)
{
    if (!hw_i2c_match_event(i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECT)) {
        return;
    }

    req = request_peek(control);
    if (!req) {
        do_error_state(i2c, control)
    }

    if (req->addr & 1) {
        do_start_receive(i2c, req);
        control->state = I2C_STATE_WAIT_DATA;
    } else {
        do_start_transmite(i2c, req);
        control->state = I2C_STATE_SEND_DATA;
    }
}

static void do_wait_oaddr(uint32_t i2c, hw_i2c_t *control)
{
    if (!hw_i2c_match_event(i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECT)) {
        return;
    }

    req = request_peek(control);
    if (!req) {
        do_error_state(i2c, control)
    }

    do_start_offset(i2c, req);
    control->state = I2C_STATE_SEND_OFFSET;
}

static void do_send_offset(uint32_t i2c, hw_i2c_t *control)
{
}

static void do_send_data(uint32_t i2c, hw_i2c_t *control)
{
}

static void do_wait_data(uint32_t i2c, hw_i2c_t *control)
{
}

static void do_try_start(int instance)
{
    hw_i2c_t *control = hw_i2c_control(instance);
    uint32_t i2c = instance == 0 ? I2C1 : I2C2;

    if (control->state == I2C_STATE_IDLE) {
        if (hw_i2c_has_req(&control)) {
            if (I2C_SR2(i2c) & I2C_SR2_BUSY) {
                control->state = I2C_STATE_WAIT_NBUSY;
                control->tout = 0;
            } else {
                I2C_CR1(i2c) |= I2C_CR1_START;
                control->state = I2C_STATE_WAIT_START;
            }
        }
    } else {
        // Nothing to do
    }
}

static void hw_i2c_release(int instance)
{
    hw_i2c_t *control = hw_i2c_control(instance);
}

static void hw_i2c_reset(int instance)
{
    hw_i2c_t *control = hw_i2c_control(instance);
    uint32_t i2c = instance == 0 ? I2C1 : I2C2;

}

static int hw_i2c_setup(int instance, uint8_t dev_id, const hw_config_t *conf)
{
    hw_i2c_t *control = hw_i2c_control(instance);
    const hw_config_i2c_t *config = (const hw_config_i2c_t *) conf;
    uint32_t i2c, i2c_clk = config->freq;
    uint8_t freq;

    if (control == NULL || hw_i2c_setup_pin(instance)) {
        return CUPKEE_ERESOURCE;
    }

    if (instance == 0) {
        rcc_periph_clock_enable(RCC_I2C1);
        i2c = I2C1;
    } else {
        rcc_periph_clock_enable(RCC_I2C2);
        i2c = I2C2;
    }
    I2C_CR1(i2c) = I2C_CR1_SWRST;

    if (i2c_clk > 400000) {
        i2c_clk = 400000;
    } else
    if (i2c_clk < 10000) {
        i2c_clk = 10000;
    }
    freq = SYS_PCLK / 1000000;

    I2C_CR1(i2c) = 0;
    I2C_CR2(i2c) = freq;
    I2C_OAR1(i2c) = config->addr & 0xFE;

    // Set ccr
    if (i2c_clk <= 100000) {
        uint16_t ccr = SYS_PCLK / (i2c_clk * 2);
        if (ccr < 4) {
            ccr = 4;
        }
        I2C_TRISE(i2c) = freq + 1;
        I2C_CCR(i2c) = ccr;
    } else {
        uint16_t ccr = SYS_PCLK/ (i2c_clk * 3);
        if (ccr < 1) {
            ccr = 1;
        }
        I2C_TRISE(i2c) = freq * 3 / 10 + 1;
        I2C_CCR(i2c) = I2C_CCR_FS | ccr;
    }

    //enable all interrupts
    //I2C_CR2(i2c) |= I2C_CR2_ITBUFEN | I2C_CR2_ITEVTEN | I2C_CR2_ITERREN;
    I2C_CR1(i2c) |= I2C_CR1_ACK | I2C_CR1_PE;

    return CUPKEE_OK;
}

static int hw_i2c_prop_get(int instance, int key, uint32_t *prop)
{
    (void) instance;
    (void) key;
    (void) prop;

    return 0;
}

static int hw_i2c_prop_set(int instance, int key, uint32_t prop)
{
    (void) instance;
    (void) key;
    (void) prop;

    return 0;
}

static int hw_i2c_prop_num(int instance)
{
    (void) instance;

    return 0;
}

static void hw_i2c_poll(int instance)
{
    hw_i2c_t *control = hw_i2c_control(instance);
    uint32_t i2c = instance == 0 ? I2C1 : I2C2;

    switch (control->state) {
    case I2C_STATE_IDLE:
    case I2C_STATE_ERROR: break;
    case I2C_STATE_WAIT_NBUSY: do_wait_nbusy(i2c, control); break;
    case I2C_STATE_WAIT_START: do_wait_start(i2c, control); break;
    case I2C_STATE_WAIT_OADDR: do_wait_oaddr(i2c, control); break;
    case I2C_STATE_WAIT_ADDR:  do_wait_addr(i2c, control); break;
    case I2C_STATE_WAIT_DATA:  do_wait_data(i2c, control); break;
    case I2C_STATE_SEND_OFFSET: do_send_offset(i2c, control); break;
    case I2C_STATE_SEND_DATA:  do_send_data(i2c, control); break;
    }
}

static void hw_i2c_sync(int instance, uint32_t systick)
{
    hw_i2c_t *control = hw_i2c_control(instance);

    (void) systick;

    if (control->state == I2C_STATE_IDLE) {
        return;
    }

    control->tout++;
    if (control->tout > 20) {
        cupkee_device_set_error(control->dev_id, CUPKEE_ETIMEOUT);
        control->state = I2C_STATE_ERROR;
    }
}

static int hw_i2c_req_read(int instance, int size, uint32_t off)
{
    hw_i2c_t *control = hw_i2c_control(instance);
    int status;

    if (!control || size < 1) {
        return -CUPKEE_EINVAL;
    }

    status = request_append_read(control, size, off);
    if (!status) {
        uint32_t i2c = instance == 0 ? I2C1 : I2C2;
        do_try_start(i2c, control);
    }
    return status;
}

static int hw_i2c_req_write(int instance, int size, uint32_t off, void *data)
{
    hw_i2c_t *control = hw_i2c_control(instance);
    int status;

    if (!control) {
        return -CUPKEE_EINVAL;
    }

    status = request_append_write(control, size, off, data);
    if (!status) {
        uint32_t i2c = instance == 0 ? I2C1 : I2C2;
        do_try_start(i2c, control);
    }
    return status;
}

static int hw_i2c_read(int instance, int size, uint32_t *off, void *buf)
{
    hw_i2c_t *control = hw_i2c_control(instance);

    if (!control || size < 0) {
        return -CUPKEE_EINVAL;
    }
    resp = response_peek(control);
    n = copy_to();
    if (resp_is_empty()) {
        response_shift();
    }

    return n;
}

static hw_driver_t i2c_driver = {
    .release = hw_i2c_release,
    .reset   = hw_i2c_reset,
    .setup   = hw_i2c_setup,
    .sync    = hw_i2c_sync,
    .poll    = hw_i2c_poll,

    .get     = hw_i2c_prop_get,
    .set     = hw_i2c_prop_set,
    .size    = hw_i2c_prop_num,
};

const hw_driver_t *hw_request_i2c(int instance)
{
    if (instance >= HW_INSTANCES_I2C || 0 == hw_use_instance(instance, &use_map)) {
        return NULL;
    }

    i2c_controls[instance].dev_id = DEVICE_ID_INVALID;
    i2c_controls[instance].state  = 0;
    i2c_controls[instance].addr   = 0;
    i2c_controls[instance].tout   = 0;
    i2c_controls[instance].rx_buff= NULL;
    i2c_controls[instance].tx_buff= NULL;

    return &i2c_driver;
}

void hw_setup_i2c(void)
{
    use_map = 0;
}

