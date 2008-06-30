############################################################################
#
# Makefile for the vision library Module of the LCV Framework
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
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -pedantic -O2 -I.. $(EXTRA_CFLAGS)

# Source files of the vision module
SOURCES_TARGET = vis.c bayer_lincorr.c
SOURCES_HOST = vis.c bayer_lincorr.c 


all: target host

target: objects_target

host: objects_host

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object files ####################################
objects_host: *.c *.h ../framework*.h
	@echo "Compiling vision Module for host."
	for i in $(SOURCES_HOST) ; \
	do \
		x=`echo "$${i}" | sed 's/.c$$//' ` ; \
		$(HOST_CC) $(HOST_CFLAGS) -c $${i} -o $${x}_host.o || exit $? ;\
	done
	
########################### Target object files ####################################
objects_target: *.c *.h ../framework*.h
	@echo "Compiling vision Module for target."
	for i in $(SOURCES_TARGET) ; \
	do \
		x=`echo "$${i}" | sed 's/.c$$//' ` ; \
		$(TARGET_CC) $(TARGET_CFLAGS) -c $${i} -o $${x}_target.o || exit $? ;\
	done
	for x in `ls -d *_target.o | sed 's/_target.o$$//' ` ; \
		do cp $${x}_target.o $${x}_sim.o; \
	done
	
