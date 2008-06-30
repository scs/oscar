############################################################################
#
# Makefile for the FileNameReader (frd) Module of the LCV Framework
#
# Author: Samuel Zahnd
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DLCV_TARGET
HOST_FEATURES += -DLCV_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files 
SOURCES_TARGET = frd.c
SOURCES_HOST = frd.c

all: target host

target: frd_target

host: frd_host

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
frd_host: *.c *.h
	@echo "Compiling module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST) -o frd_host.o
	
####################### Target object file ###############################
frd_target: *.c *.h
	@echo "Compiling module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET) -o frd_target.o
	@cp frd_target.o frd_target_sim.o

	
