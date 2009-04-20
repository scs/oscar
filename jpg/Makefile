############################################################################
#
# Makefile for the jpg module of the Oscar framework
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
SOURCES = jpg_dct_.c jpg_huffman_.c jpg_quant_.c jpg_encoder_.c jpg_marker_.c jpg_readYUV_.c

all: target host 

target: target1 sim1

clean:
	rm -f *.o *.elf *.gdb
	
############################# Host object file ####################################
host: *.c *.h ../oscar*.h
	@echo "Compiling JPG Module for host."
	@rm -f *_host.o
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES) 
#	@rename "s/\_.o/_host\.o/" *_.o
	for i in *_.o; do mv $$i $$(echo $$i | sed -r 's|_\.o$$|_host.o|'); done
#	mv jpg_shared.o jpg_shared_host.o
	
########################### Target object file ####################################
target1: *.c *.h ../oscar*.h
	@echo "Compiling JPG Module for target."
	@rm -f *_target.o
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES)
#	@rename "s/\_.o/_target\.o/" *_.o
	for i in *_.o; do mv $$i $$(echo $$i | sed -r 's|_\.o$$|_target.o|'); done
#	@mv ipc_shared.o ipc_shared_target.o
#	@cp ipc_shared_target.o ipc_shared_target_sim.o
#	@cp ipc_target.o ipc_target_sim.o
	
sim1: *.c *.h ../oscar*.h
	@echo "Compiling JPG Module for target."
	@rm -f *_target_sim.o
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES)
#	@rename "s/\_.o/_target_sim\.o/" *_.o
	for i in *_.o; do mv $$i $$(echo $$i | sed -r 's|_\.o$$|_target_sim.o|'); done
#	@mv ipc_shared.o ipc_shared_target.o
#	@cp ipc_shared_target.o ipc_shared_target_sim.o
#	@cp ipc_target.o ipc_target_sim.o
	
