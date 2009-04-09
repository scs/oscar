# Makefile for the jpg module of the Oscar framework

# Defines for C files
HOST_FEATURES += -DOSC_HOST
TARGET_FEATURES += -DOSC_TARGET
TARGET_SIM_FEATURES += -DOSC_TARGET -DOSC_SIM

# Host-Compiler executables and flags
HOST_CC := gcc -std=gnu99 -Wall -Wno-long-long -pedantic -O2 -I.. -g

# Cross-Compiler executables and flags
TARGET_CC := bfin-uclinux-gcc -std=gnu99 -Wall -Wno-long-long -ggdb3 -I..

.PHONY: all host target clean
all: target host

host: swr_host.o
target: swr_target.o swr_target_sim.o

clean:
	rm -f *.o *.elf *.gdb
	
# Host object file
%_host.o: %.c *.h
	$(HOST_CC) $(HOST_FEATURES) -c $< -o $@
	
# Target object file
%_target.o: %.c *.h
	$(TARGET_CC) $(TARGET_FEATURES) -c $< -o $@

%_target_sim.o: %.c *.h
	$(TARGET_CC) $(TARGET_SIM_FEATURES) -c $< -o $@
