elf_NAMES = out

out_SRCS = source/main.c \
		   source/shell.c \
		   source/misc.c \
		   source/timeout.c

out_CPPFLAGS = -I../hal/include -I../sal/include -I../../panda/include
out_CFLAGS   =
out_LDFLAGS  = -L../sal -L../hal -L../lang -lsal -lhal -llang

include ../../mk/cupkee.ruls.mk

