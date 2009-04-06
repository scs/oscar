############################################################################
#
# Makefile for the Logging Module of the Oscar framework
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

# Source files of the logging module
SOURCES_TARGET = log_target.c
SOURCES_HOST = log_host.c 


all: target host

target: log_target.o

host: log_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
log_host.o: *.c *.h ../oscar*.h
	@echo "Compiling Log Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
########################### Target object file ####################################
log_target.o: *.c *.h ../oscar*.h
	@echo "Compiling Log Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@cp log_target.o log_target_sim.o
	
