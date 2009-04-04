SHELL := $(shell which bash)

PEDANTIC := -Wall -pedantic
OPTIMIZE := -O3

INCLUDES_host := config_host.h
INCLUDES_target := config_target.h

CC_host := gcc -c
CC_target := bfin-uclinux-gcc -c

.PHONY: all host target
all: host target
host: dmtx_host.o
target: dmtx_target.o

dmtx_%.o: INCLUDES += $(addprefix -include , $(INCLUDES_$*))
dmtx_%.o: dmtx.c *.c *.h $(MAKEFILE_LIST)
	$(CC_$*) $(PEDANTIC) $(OPTIMIZE) $(INCLUDES) -o $@ $<

clean:
	rm dmtx_*.o
