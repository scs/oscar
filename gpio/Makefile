############################################################################
#
# Makefile for the GPIO Module of the Oscar framework
#
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DOSC_TARGET
HOST_FEATURES += -DOSC_HOST
SIM_FEATURES += -DOSC_TARGET -DOSC_SIM

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)
TARGET_SIM_CFLAGS = $(SIM_FEATURES) -Wall -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files of the logging module
SOURCES_TARGET = gpio_target.c gpio_shared.c
SOURCES_HOST = gpio_host.c gpio_shared.c
SOURCES_TARGET_SIM = gpio_host.c gpio_shared.c

all: target host

target: gpio_target.o

host: gpio_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
gpio_host.o: *.c *.h ../oscar*.h
	@echo "Compiling GPIO Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	@mv gpio_shared.o gpio_shared_host.o
	
########################### Target object file ####################################
gpio_target.o: *.c *.h ../oscar*.h
	@echo "Compiling GPIO Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	@mv gpio_shared.o gpio_shared_target.o
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM)
	@mv gpio_host.o gpio_target_sim.o
	@mv gpio_shared.o gpio_shared_target_sim.o
	
