############################################################################
#
# Makefile for the Interprocess communication Module of the LCV Framework
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
SOURCES_TARGET = ipc_shared.c ipc_target.c
SOURCES_HOST = ipc_shared.c ipc_host.c 


all: target host

target: ipc_target.o

host: ipc_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
ipc_host.o: *.c *.h ../framework*.h
	@echo "Compiling IPC Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	mv ipc_shared.o ipc_shared_host.o
	
########################### Target object file ####################################
ipc_target.o: *.c *.h ../framework*.h
	@echo "Compiling IPC Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@mv ipc_shared.o ipc_shared_target.o
	@cp ipc_shared_target.o ipc_shared_target_sim.o
	@cp ipc_target.o ipc_target_sim.o
	
