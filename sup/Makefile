############################################################################
#
# Makefile for the Support Module of the Oscar framework
#
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DOSC_TARGET
HOST_FEATURES += -DOSC_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files of the logging module
SOURCES_TARGET = sup_target.c
SOURCES_HOST = sup_host.c 


all: target host

target: sup_target.o

host: sup_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
sup_host.o: *.c *.h ../oscar*.h
	@echo "Compiling sup Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
########################### Target object file ####################################
sup_target.o: *.c *.h ../oscar*.h
	@echo "Compiling sup Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@cp sup_target.o sup_target_sim.o
	
