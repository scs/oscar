############################################################################
#
# Makefile for the StimuliWriter (swr) Module of the LCV Framework
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

# Source files 
SOURCES_TARGET = swr_target.c
SOURCES_TARGET_SIM = swr_host.c
SOURCES_HOST = swr_host.c
CLASSES_SHARED = swr_multibuffer

all: target host

target: swr_target

host: swr_host

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
swr_host: *.c *.h
	@echo "Compiling module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
####################### Target object file ###############################
swr_target: *.c *.h
	@echo "Compiling module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM) \
	-o swr_target_sim.o

	
