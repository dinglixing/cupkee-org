lib_NAMES = bsp

bsp_SRCS = setup.c \
		   usb.c

bsp_CPPFLAGS = -I${BSP_DIR}/include
bsp_CFLAGS   =
bsp_LDFLAGS  =

include ${MAKE_DIR}/cupkee.ruls.mk

VPATH = ${BSP_DIR}/stm32f1x

