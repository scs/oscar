############################################################################
#
# Makefile for the Hierarchical State Machine (hsm) Module of LCV Framework
#
# Author: Samuel Zahnd
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DLCV_TARGET
HOST_FEATURES += -DLCV_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -O2 -I.. -g

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -ggdb3 -I..

# Source files 
SOURCES_TARGET = hsm.c
SOURCES_HOST = hsm.c

all: target host

target: hsm_target.o

host: hsm_host.o

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
hsm_host.o: *.c *.h
	@echo "Compiling module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST) -o hsm_host.o
	
####################### Target object file ###############################
hsm_target.o: *.c *.h
	@echo "Compiling module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET) -o hsm_target.o
	@cp hsm_target.o hsm_target_sim.o

	
