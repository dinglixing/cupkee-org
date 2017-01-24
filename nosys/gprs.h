#ifndef __GPRS_INC__
#define __GPRS_INC__

typedef struct gprs_host_t {
    const char *protocol;
    const char *domain;
    const char *port;
} gprs_host_t;

void gprs_init(device_t *gprs_dev, int host_num, const gprs_host_t *hosts);

void gprs_start(void (*reset)(void),
                void (*enable)(void),
                void (*start)(void));

void gprs_systick_proc(void);
void gprs_event_proc(uint8_t which);

int  gprs_recv_cb_register(void (*)(int, void *));
int  gprs_send(int len, const void *data);

/* debug api */
void gprs_show_msg(int show);
void gprs_send_command(const char *cmd);
void gprs_send_data(const char *data);

#endif /* __GPRS_INC__ */

