lib_NAMES = bsp

bsp_SRCS = ${notdir ${wildcard ${BSP_DIR}/${BOARD_SRC_DIR}/*.c}}
$(info ${bsp_SRCS})

bsp_CPPFLAGS = -I${BSP_DIR}/include
bsp_CFLAGS   =
bsp_LDFLAGS  =

include ${MAKE_DIR}/cupkee.ruls.mk

VPATH = ${BSP_DIR}/${BOARD_SRC_DIR}

