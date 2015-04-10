DISK = disk

all:	  xsm

lex.yy.c:   data.h instr.h decode.lex
	flex -i decode.lex

xsm:	simulator.c lex.yy.c  disk.h disk.c timer.h boot.h boot.c  interrupt.h interrupt.c utility.h utility.c debug.h debug.c error_helper.h error_helper.c
	gcc -g -o xsm simulator.c disk.c boot.c interrupt.c utility.c debug.c error_helper.c
clean:
	find . -type f -name 'lex.yy.c' | xargs -r rm
	find . -type f -name 'xsm' | xargs -r rm
