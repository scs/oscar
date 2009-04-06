############################################################################
#
# Makefile for the config Module of the Oscar framework
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DOSC_TARGET
HOST_FEATURES += -DOSC_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -Wno-long-long -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -Wno-long-long -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files of the config module
SOURCES_TARGET = cfg.c
SOURCES_HOST = cfg.c 

all: target host

target: cfg_target.o

host: cfg_host.o

clean:
	rm -f *.o *.elf *.gdb

############################# Host object file ####################################
cfg_host.o: *.c *.h ../oscar*.h
	@echo "Compiling config Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST) -o cfg_host.o

########################### Target object file ####################################
cfg_target.o: *.c *.h ../oscar*.h
	@echo "Compiling config Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET) -o cfg_target.o
	@cp cfg_target.o cfg_target_sim.o

