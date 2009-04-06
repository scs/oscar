############################################################################
#
# Makefile for the Camera Module of the Oscar framework
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

# Source files of the camera module
SOURCES_TARGET = cam_target.c
SOURCES_TARGET_SIM = cam_host.c
SOURCES_HOST = cam_host.c

all: target host

target: cam_target

host: cam_host

clean:
	rm -f *.o *.elf *.gdb
	
######################## Host object file ################################
cam_host: *.c *.h ../oscar*.h
	@echo "Compiling Cam Module for host."
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES_HOST)
	$(HOST_CC) $(HOST_CFLAGS) -c cam_multibuffer.c \
		-o cam_multibuffer_host.o ;
	$(HOST_CC) $(HOST_CFLAGS) -c cam_shared.c \
		-o cam_shared_host.o ;
	
####################### Target object file ###############################
cam_target: *.c *.h ../oscar*.h
	@echo "Compiling Cam Module for target."
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES_TARGET)
	$(TARGET_CC) $(TARGET_CFLAGS) -c cam_multibuffer.c \
		-o cam_multibuffer_target.o ;
	$(TARGET_CC) $(TARGET_CFLAGS) -c cam_shared.c \
		-o cam_shared_target.o ;
	$(TARGET_CC) $(TARGET_SIM_CFLAGS) -c $(SOURCES_TARGET_SIM) \
	-o cam_target_sim.o
	@cp cam_multibuffer_target.o cam_multibuffer_sim.o
	@cp cam_shared_target.o cam_shared_sim.o
	
