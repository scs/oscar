############################################################################
#
# Makefile for the DMA Module of the OSC Framework
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

# Source files of the DMA module
SOURCES_TARGET = dma_shared.c dma_target.c
SOURCES_HOST = dma_shared.c dma_host.c 


all: target host

target: dma_target.o

host: dma_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
dma_host.o: *.c *.h ../framework*.h
	@echo "Compiling dma Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	mv dma_shared.o dma_shared_host.o
	
########################### Target object file ####################################
dma_target.o: *.c *.h ../framework*.h
	@echo "Compiling dma Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@mv dma_shared.o dma_shared_target.o
	@cp dma_shared_target.o dma_shared_target_sim.o
	@cp dma_target.o dma_target_sim.o
	
