############################################################################
#
# Makefile for the Blackfin DSP runtime library Module of the LCV 
# Framework
#
# Author: Markus Berner, Samuel Zahnd
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

# Source files of the realtime library module
SOURCES_TARGET = rtl_target.c
SOURCES_HOST = rtl_host.c 


all: target host

target: rtl_target.o

host: rtl_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
rtl_host.o: *.c *.h ../framework*.h
	@echo "Compiling rtl Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
########################### Target object file ####################################
rtl_target.o: *.c *.h ../framework*.h
	@echo "Compiling rtl Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@cp rtl_target.o rtl_target_sim.o
	
