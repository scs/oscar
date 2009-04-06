############################################################################
#
# Makefile for the Simulation Module of the Oscar framework
#
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DOSC_TARGET
TARGET_SIM_FEATURES += -DOSC_TARGET -DOSC_SIM
HOST_FEATURES += -DOSC_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -Wno-long-long -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -Wno-long-long -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)
TARGET_SIM_CFLAGS = $(TARGET_SIM_FEATURES) -Wall -Wno-long-long -pedantic -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files of the logging module
SOURCES_TARGET = sim_target.c
SOURCES_TARGET_SIM = sim_host.c
SOURCES_HOST = sim_host.c 


all: target host

target: sim_target.o

host: sim_host.o

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
sim_host.o: *.c *.h ../oscar*.h
	@echo "Compiling Sim Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
########################### Target object file ####################################
sim_target.o: *.c *.h ../oscar*.h
	@echo "Compiling Sim Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM) \
	-o sim_target_sim.o

	
