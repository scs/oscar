############################################################################
#
# Makefile for the StimuliReader (srd) Module of the Oscar Framework
#
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DOSC_TARGET
TARGET_SIM_FEATURES += -DOSC_TARGET -DOSC_SIM
HOST_FEATURES += -DOSC_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -Wno-long-long -pedantic -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -Wno-long-long -ggdb3 -I.. $(EXTRA_CFLAGS)
TARGET_SIM_CFLAGS = $(TARGET_SIM_FEATURES) -Wall -Wno-long-long -ggdb3 -I.. $(EXTRA_CFLAGS)

# Source files 
SOURCES_TARGET = srd_target.c
SOURCES_TARGET_SIM = srd_host.c
SOURCES_HOST = srd_host.c

all: target host

target: srd_target

host: srd_host

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
srd_host: *.c *.h
	@echo "Compiling module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	
####################### Target object file ###############################
srd_target: *.c *.h
	@echo "Compiling module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM) \
	-o srd_target_sim.o

	
