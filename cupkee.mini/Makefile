
export MCU = stm32f103ze

all: hal sal lang main
	@printf "build ok\n"


hal:
	@make -C hal

sal:
	@make -C sal

lang:
	@make -C lang

main:
	@make -C main

clean:
	@make -C hal clean
	@make -C sal clean
	@make -C lang clean
	@make -C main clean

.PHONY: hal sal lang main clean
