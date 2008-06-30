############################################################################
#
# Makefile for the bitmap Module of the LCV Framework
#
# Author: Markus Berner
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DLCV_TARGET
HOST_FEATURES += -DLCV_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files of the logging module
SOURCES_TARGET = cfg.c
SOURCES_HOST = cfg.c 


all: target host

target: cfg_target.o

host: cfg_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
cfg_host.o: *.c *.h ../framework*.h
	@echo "Compiling bitmap Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST) -o cfg_host.o
	
########################### Target object file ####################################
cfg_target.o: *.c *.h ../framework*.h
	@echo "Compiling bitmap Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET) -o cfg_target.o
	@cp cfg_target.o cfg_target_sim.o
	
