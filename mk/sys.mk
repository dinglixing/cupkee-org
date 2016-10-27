lib_NAMES = sys

sys_SRCS = init.c \
		   util.c \
		   misc.c \
		   event.c \
		   rbuff.c \
		   console.c \
		   shell.c \
		   timeout.c \
		   device.c \
		   gpio.c

sys_CPPFLAGS = -I${BSP_DIR}/include -I${SYS_DIR}/include -I${LANG_DIR}/include
sys_CFLAGS   =
sys_LDFLAGS  =

include ${MAKE_DIR}/cupkee.ruls.mk

VPATH = ${SYS_DIR}/source
