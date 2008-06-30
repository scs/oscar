############################################################################
#
# Makefile for the Cpld Module of the LCV Framework
#
# Author: Samuel Zahnd
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DLCV_TARGET
TARGET_SIM_FEATURES += -DLCV_TARGET -DLCV_SIM
HOST_FEATURES += -DLCV_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -O2 -I.. $(EXTRA_CFLAGS) -g 

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -ggdb3 -I.. $(EXTRA_CFLAGS)
TARGET_SIM_CFLAGS = $(TARGET_SIM_FEATURES) -Wall -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files 
SOURCES_TARGET = cpld_target.c
SOURCES_TARGET_SIM = cpld_host.c
SOURCES_HOST = cpld_host.c

all: target host

target: cpld_target

host: cpld_host

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
cpld_host: *.c *.h
	@echo "Compiling Cpld Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
####################### Target object file ###############################
cpld_target: *.c *.h
	@echo "Compiling Cpld Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM) \
	-o cpld_target_sim.o

	
