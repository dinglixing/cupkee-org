

LDSCRIPT    = ${BASE_DIR}/ld/$(MCU).ld

DEFS		+= -DSTM32F1
DEFS		+= -I$(OPENCM3_DIR)/include

ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd

OPENCM3_DIR = ${BASE_DIR}/libopencm3
OPENCM3_LIB = opencm3_stm32f1

ARCH_LDFLAGS += -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib -l$(OPENCM3_LIB)
ARCH_LDFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
ARCH_LDFLAGS += --static -nostartfiles
ARCH_LDFLAGS += -Wl,--gc-sections
ifeq ($(V),99)
ARCH_LDFLAGS += -Wl,--print-gc-sections
endif

PREFIX  ?= arm-none-eabi-

