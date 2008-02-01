# Defines for C files
TARGET_FEATURES += -DLCV_TARGET
HOST_FEATURES += -DLCV_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -O2 -I..

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -ggdb3 -I..
TARGET_LDFLAGS = -Wl,-elf2flt="-s 1048576" -fmudflap -lmudflap


all: 	target host

host:	test.c lib/liblcv_host.a inc/*.h
	@echo "Creating host executable..."
	$(HOST_CC) test.c lib/liblcv_host.a $(HOST_CFLAGS) -o test
	@echo "Host executable done."

target: test.c lib/liblcv_target.a inc/*.h
	@echo "Creating target executable..."
	$(TARGET_CC) test.c lib/liblcv_target.a $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o test
	cp test /tftpboot
	@echo "Target executable done."

get:
	@rm -r inc lib || exit $?
	@cp -r ../framework/staging/* . 
	@echo "Fetched LCV framework."

clean: 
	@rm test
	@echo "Cleaned directory."
