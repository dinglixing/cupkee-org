
#include "app.h"

#define GPRS_BUF_SIZE  128
#define SETUP_STEPS    (sizeof(gprs_setup_tab) / sizeof(gprs_command_t))

#define GPRS_RESET_TIME         (500)         // 0.5 seconds
#define GPRS_IDLE_TIME          (500)         // 0.5 seconds
#define GPRS_INIT_TIMEOUT       (20 * 1000)   // 20 seconds
#define GPRS_CONNECT_TIMEOUT    (10 * 1000)   // 15 seconds

typedef struct gprs_command_t {
    const char *cmd;
    const char *exp;
    uint16_t   timeout;
} gprs_command_t;

enum GPRS_STATE {
    GPRS_RESET,
    GPRS_IDLE,
    GPRS_INIT,
    GPRS_SETUP,
    GPRS_READY,
    GPRS_CONNECT,
    GPRS_CONNECTED,
};

enum SESS_STATE {
    SESS_IDLE,
    SESS_SEND,
    SESS_TRAN,
    SESS_RECV,
};

static uint8_t  gprs_state;
static uint8_t  gprs_setup_step;
static uint8_t  gprs_host_curr;
static uint8_t  gprs_host_num = 0;
static uint8_t  sess_state;
static uint8_t  show_flags = 0;

static void (*gprs_hw_reset)(void) = NULL;
static void (*gprs_hw_enable)(void) = NULL;
static void (*gprs_hw_start)(void) = NULL;

static uint32_t gprs_wait;
static int  gprs_recv_len;
static int  gprs_send_len;
static int  gprs_send_pos;
static char gprs_recv_buf[GPRS_BUF_SIZE];
static char gprs_send_buf[GPRS_BUF_SIZE];
static const gprs_host_t *gprs_hosts = NULL;
static const gprs_command_t gprs_setup_tab[] = {
    {"+RST",        "OK",                  1000},
    {"+CPIN?",      "READY",               1000},
    {"+CREG?",      "CREG: 1",             1000},
    {"+CGATT=1",    "OK",                  15000},
    {"+CGDCONT=1,\"IP\",\"CMNET\"", "OK",  10000},
    {"+CGACT=1,1",  "OK",                  10000},
};

static void (*gprs_recv_cb)(int, void *) = NULL;
static cupkee_device_t *gprs_tty = NULL;

static void gprs_recv_default(int len, void *data)
{
    (void) data;

    console_log("GPRS: recv %d\r\n", len);
}

static inline void gprs_event_post(uint8_t which) {
    cupkee_event_post(EVENT_GPRS, which, 0);
}

static inline void gprs_msg_clean(void) {
}

static inline int gprs_take(int size, void *buf) {
    return cupkee_device_recv(gprs_tty, size, buf);
}

static inline int gprs_give(int len, const void *data) {
    return cupkee_device_send(gprs_tty, len, data);
}

static inline void gprs_translate_init(int len, const void *data) {
    gprs_send_len = len;
    gprs_send_pos = 0;
    if (len) {
        memcpy(gprs_send_buf, data, len);
    }
}

static void gprs_do_send(void)
{
    if (gprs_send_len > gprs_send_pos) {
        int n = gprs_give(gprs_send_len - gprs_send_pos, gprs_send_buf + gprs_send_pos);

        if (n > 0) {
            gprs_send_pos += n;
        }

        if (gprs_send_pos == gprs_send_len) {
            char end = 0x1a;
            gprs_send_pos = 0;
            gprs_send_len = 0;
            gprs_give(1, &end);
        }
    }
}

static void gprs_do_recv(void)
{
    int len = gprs_take(GPRS_BUF_SIZE, gprs_recv_buf);

    if (len > 0) {
        gprs_recv_len = len;
        gprs_recv_buf[len] = 0;

        gprs_event_post(GPRS_EV_MSG);

        /* For debug */
        if (show_flags) {
            console_log(gprs_recv_buf);
        }
    }
}

static void gprs_device_handle(cupkee_device_t *dev, uint8_t code, intptr_t param)
{
    (void) dev;
    (void) param;

    if (code == DEVICE_EVENT_DATA) {
        gprs_do_recv();
    } else
    if (code == DEVICE_EVENT_DRAIN) {
        gprs_do_send();
    }
}

static void gprs_reset(void) {
    if (gprs_hw_reset) {
        gprs_hw_reset();
    }

    gprs_state = GPRS_RESET;
    gprs_wait  = GPRS_RESET_TIME;

    gprs_translate_init(0, NULL);
    console_log("GPRS: reset\r\n");
}

static void gprs_enable(void) {
    if (gprs_hw_enable) {
        gprs_hw_enable();
    }

    gprs_state = GPRS_IDLE;
    gprs_wait  = GPRS_IDLE_TIME;
    console_log("GPRS: enable\r\n");
}

static void gprs_power_on(void) {
    if (gprs_hw_start) {
        gprs_hw_start();
    }

    gprs_state = GPRS_INIT;
    gprs_wait  = GPRS_INIT_TIMEOUT;
    console_log("GPRS: power on\r\n");
}

static void gprs_init_msg_proc(void)
{
    char *key;
    if (gprs_recv_len && (key = strstr(gprs_recv_buf, "+CREG: "))) {
        char stat = key[7];
        if (stat == '1' || stat == '5') {
            gprs_event_post(GPRS_EV_DONE);
        }
    }
}

static void gprs_do_setup(void) {
    if (gprs_setup_step < SETUP_STEPS) {
        gprs_send_command(gprs_setup_tab[gprs_setup_step].cmd);
        gprs_wait = gprs_setup_tab[gprs_setup_step].timeout;
    } else {
        gprs_reset();
    }
}

static void gprs_setup_next(void) {
    if (++gprs_setup_step >= SETUP_STEPS) {
        gprs_event_post(GPRS_EV_DONE);
    } else {
        gprs_do_setup();
    }
}

static void gprs_setup_msg_proc(void)
{
    if (gprs_setup_step < SETUP_STEPS) {
        const char *exp = gprs_setup_tab[gprs_setup_step].exp;
        if (gprs_recv_len && strstr(gprs_recv_buf, exp)) {
            gprs_setup_next();
        }
    }
}

static void gprs_setup(void) {
    gprs_state = GPRS_SETUP;
    gprs_setup_step = 0;

    console_log("GPRS: setup\r\n");
    gprs_do_setup();
}

static void gprs_do_connect(const gprs_host_t *host) {
    console_log("GPRS: connect to %s:%s\r\n", host->domain, host->port);

    gprs_wait = GPRS_CONNECT_TIMEOUT;

    // Send command
    gprs_give(13, "AT+CIPSTART=\"");
    gprs_give(strlen(host->protocol), host->protocol);
    gprs_give(3, "\",\"");
    gprs_give(strlen(host->domain), host->domain);
    gprs_give(2, "\",");
    gprs_give(strlen(host->port), host->port);
    gprs_give(2, "\r\n");
}

static void gprs_connect_next(void) {
    console_log("GPRS: connect host %d fail\r\n", gprs_host_curr);

    gprs_host_curr++;
    gprs_reset();
}

static void gprs_connect_msg_proc(void) {
    if (gprs_recv_len && strstr(gprs_recv_buf, "CONNECT OK")) {
        gprs_event_post(GPRS_EV_DONE);
    } else
    if (gprs_recv_len && strstr(gprs_recv_buf, "+CME ")) {
        gprs_connect_next();
    }
}

static void gprs_connect(void) {
    gprs_state = GPRS_CONNECT;

    if (gprs_host_curr >= gprs_host_num) {
        gprs_host_curr = 0;
    }
    console_log("GPRS: host %d\r\n", gprs_host_curr);
    gprs_do_connect(&gprs_hosts[gprs_host_curr]);
}

static void gprs_sess_recv(char *rcv, int end)
{
    int len = 0;
    int pos = 8;

    while (rcv[pos] != ',' && pos < end) {
        len = len * 10 + (rcv[pos++] - '0');
    }

    if (rcv[pos++] != ',') {
        console_log("GPRS: Invlaid +CIPRCV len\r\n");
        return;
    }

    if (pos + len >= end) {
        console_log("GPRS: recvice not end\r\n");
    } else {
        if (gprs_recv_cb) {
            gprs_recv_cb(len, rcv + pos);
        }
    }
}

static void gprs_sess_msg_proc(void)
{
    char *rcv;

    if (strstr(gprs_recv_buf, "+CME ")) {
        gprs_reset();
    }
    if (sess_state == SESS_SEND) {
        if (gprs_recv_len && strstr(gprs_recv_buf, "\r\n>")) {
            sess_state = SESS_TRAN;
            gprs_wait = 10 * 1000;
            gprs_do_send();
        }
    }
    if (sess_state == SESS_TRAN) {
        if (gprs_send_pos == gprs_send_len && strstr(gprs_recv_buf, "OK\r\n")) {
            gprs_wait = 0;
            sess_state = SESS_IDLE;
        }
    }
    if (NULL != (rcv = strstr(gprs_recv_buf, "+CIPRCV:"))) {
        gprs_sess_recv(rcv, gprs_recv_len - (rcv - gprs_recv_buf));
    }
    if (strstr(gprs_recv_buf, "+TCPCLOSED")) {
        gprs_connect();
    }
}

static void gprs_session(void) {
    gprs_state = GPRS_CONNECTED;
    sess_state = SESS_IDLE;
    gprs_wait = 0;

    console_log("GPRS: connected\r\n");
}

static void gprs_process_tout(void)
{
    if (gprs_state == GPRS_RESET) {
        gprs_event_post(GPRS_EV_DONE);
    } else
    if (gprs_state == GPRS_IDLE) {
        gprs_event_post(GPRS_EV_DONE);
    } else
    if (gprs_state == GPRS_INIT) {
        gprs_reset();
    } else
    if (gprs_state == GPRS_SETUP) {
        gprs_reset();
    } else
    if (gprs_state == GPRS_CONNECT) {
        console_log("GPRS: connect timeout\r\n");
        gprs_connect_next();
    } else {
        gprs_reset();
    }
}

static void gprs_process_done(void)
{
    switch (gprs_state) {
    case GPRS_RESET:    gprs_enable();  break;
    case GPRS_IDLE:     gprs_power_on();break;
    case GPRS_INIT:     gprs_setup();   break;
    case GPRS_SETUP:    gprs_connect(); break;
    case GPRS_CONNECT:  gprs_session(); break;
    default: gprs_reset();
    }
}

static void gprs_process_msg(void)
{
    if (gprs_state == GPRS_INIT) {
        gprs_init_msg_proc();
    } else
    if (gprs_state == GPRS_SETUP){
        gprs_setup_msg_proc();
    } else
    if (gprs_state == GPRS_CONNECT) {
        gprs_connect_msg_proc();
    } else
    if (gprs_state == GPRS_CONNECTED) {
        gprs_sess_msg_proc();
    }

    gprs_msg_clean();
}

void gprs_systick_proc(void)
{
    if (gprs_wait) {
        if (--gprs_wait == 0) {
            gprs_event_post(GPRS_EV_TOUT);
        }
    }
}

void gprs_event_proc(uint8_t which)
{
    switch (which) {
    case GPRS_EV_TOUT:      gprs_process_tout(); break;
    case GPRS_EV_DONE:      gprs_process_done(); break;
    case GPRS_EV_MSG:       gprs_process_msg(); break;
    default: break;
    }
}

void gprs_init(cupkee_device_t *gprs_dev, int host_num, const gprs_host_t *hosts)
{
    gprs_host_curr = 0;
    gprs_recv_cb   = gprs_recv_default;


    gprs_tty = gprs_dev;
    gprs_tty->handle = gprs_device_handle;
    gprs_tty->handle_param = 0;

    gprs_host_num = host_num;
    gprs_hosts    = hosts;
}

void gprs_start(void (*reset)(void), void (*enable)(void), void (*start)(void))
{
    gprs_hw_reset  = reset;
    gprs_hw_enable = enable;
    gprs_hw_start  = start;

    if (gprs_host_num > 0) {
        gprs_reset();
    }
}

int gprs_send(int len, const void *data)
{
    if (len > GPRS_BUF_SIZE || gprs_state != GPRS_CONNECTED || sess_state != SESS_IDLE) {
        return -1;
    }

    gprs_translate_init(len, data);

    gprs_give(12, "AT+CIPSEND\r\n");
    sess_state = SESS_SEND;

    return 0;
}

int gprs_recv_cb_register(void (*cb)(int, void *))
{
    if (cb) {
        gprs_recv_cb = cb;
        return 0;
    } else {
        return -1;
    }
}

/* Debug API */
void gprs_send_command(const char *cmd)
{
    gprs_give(2, "AT");
    if (cmd) {
        gprs_give(strlen(cmd), cmd);
    }
    gprs_give(2, "\r\n");
}

void gprs_send_data(const char *data)
{
    int len;
    char end = 0x1a;

    if (!data) {
        data = "hello";
    }
    len = strlen(data);

    gprs_give(len, data);
    gprs_give(1, &end);
}

void gprs_show_msg(int show)
{
    if (show) {
        show_flags |= 1;
    } else {
        show_flags &= ~1;
    }
}

