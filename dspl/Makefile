############################################################################
#
# Makefile for the Blackfin DSP runtime library Module of the Oscar 
# framework
#
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

# Source files of the realtime library module
SOURCES_TARGET = dspl_target.c
SOURCES_HOST = dspl_host.c 


all: target host

target target_sim: dspl_target.o

host: dspl_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
dspl_host.o: *.c *.h ../oscar*.h
	@echo "Compiling dspl Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
########################### Target object file ####################################
dspl_target.o: *.c *.h ../oscar*.h
	@echo "Compiling dspl Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@cp dspl_target.o dspl_target_sim.o
	
