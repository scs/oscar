############################################################################
#
# Makefile for the vision library Module of the Oscar framework
#
# 
############################################################################

# Defines for C files
TARGET_FEATURES += -DOSC_TARGET
HOST_FEATURES += -DOSC_HOST

# Host-Compiler executables and flags
HOST_CC := gcc 
HOST_CFLAGS := $(HOST_FEATURES) -Wall -Wno-long-long -pedantic -O2 -I.. -g $(EXTRA_CFLAGS)

# Cross-Compiler executables and flags
TARGET_CC := bfin-uclinux-gcc 
TARGET_CFLAGS := $(TARGET_FEATURES) -Wall -Wno-long-long -pedantic -O2 -I.. $(EXTRA_CFLAGS)

# Source files of the vision module
SOURCES_HOST := vis.c bayer_fast.c transform.c bayer_lincorr.c 
SOURCES_TARGET := vis.c bayer_fast.c transform.c bayer_lincorr.c

OBJECTS_HOST := $(patsubst %.c, %_host.o, $(SOURCES_HOST))
OBJECTS_TARGET := $(patsubst %.c, %_target.o, $(SOURCES_TARGET)) vector_debayer_grey_target.o
OBJECTS_TARGET_SIM := $(patsubst %.c, %_target_sim.o, $(SOURCES_TARGET)) vector_debayer_grey_target.o

all: target host

vector_debayer_grey_target.o:
	bfin-uclinux-as vector_debayer_grey_target.asm -o vector_debayer_grey_target.o

host: $(OBJECTS_HOST) $(OBJECTS_TARGET_SIM)
target: $(OBJECTS_TARGET)

clean:
	rm -f *.o *.elf *.gdb

%_host.o: %.c *.h ../oscar*.h
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

%_target.o %_target_sim.o: %.c *.h ../oscar*.h
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@
