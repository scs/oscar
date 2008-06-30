############################################################################
#
# Makefile for the Calibration Module of the LCV Framework
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
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -ggdb3 -I.. $(EXTRA_CFLAGS)
TARGET_SIM_CFLAGS = $(TARGET_SIM_FEATURES) -Wall -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files of the module
SOURCES_TARGET = clb_target.c
SOURCES_TARGET_SIM = clb_host.c
SOURCES_HOST = clb_host.c

all: target host

target: clb_target

host: clb_host

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
clb_host: *.c *.h ../framework*.h
	@echo "Compiling Clb Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
####################### Target object file ###############################
clb_target: *.c *.h ../framework*.h
	@echo "Compiling Clb Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM) \
	-o clb_target_sim.o
	
