#ifndef __CUPKEE_INC__
#define __CUPKEE_INC__

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


/* Todo: put to cupkee_type.h ? */
#define CUPKEE_TRUE                 1
#define CUPKEE_FALSE                0

/* User configure ? */
#define APP_DEV_MAX                 8

#include "cupkee_bsp.h"
#include "cupkee_errno.h"
#include "cupkee_utils.h"
#include "cupkee_event.h"
#include "cupkee_buffer.h"
#include "cupkee_device.h"
#include "cupkee_console.h"
#include "cupkee_auto_complete.h"
#include "cupkee_history.h"
#include "cupkee_shell.h"

void cupkee_init(void);
void cupkee_loop(void);

int  cupkee_event_handle_register(cupkee_event_handle_t handle);
uint32_t cupkee_systicks(void);

#endif /* __CUPKEE_INC__ */

