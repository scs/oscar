############################################################################
#
# Makefile for the bitmap Module of the OScar Framework
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
SOURCES_TARGET = bmp.c
SOURCES_HOST = bmp.c 


all: target host

target: bmp_target.o

host: bmp_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
bmp_host.o: *.c *.h ../oscar*.h
	@echo "Compiling bitmap Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST) -o bmp_host.o
	
########################### Target object file ####################################
bmp_target.o: *.c *.h ../oscar*.h
	@echo "Compiling bitmap Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET) -o bmp_target.o
	@cp bmp_target.o bmp_target_sim.o
	
