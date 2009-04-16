# Defines for C files
TARGET_FEATURES += -DLCV_TARGET
HOST_FEATURES += -DLCV_HOST

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -Wno-long-long -O2 -I.. -g
HOST_LDFLAGS = -lm

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = $(TARGET_FEATURES) -Wall -Wno-long-long -ggdb3 -I..
TARGET_LDFLAGS = -Wl,-elf2flt="-s 1048576" -fmudflap -lmudflap -lbfdsp


all: 	target host

host:	test.c cgi_host lib/liblcv_host.a inc/*.h
	@echo "Creating host executable..."
	$(HOST_CC) test.c lib/liblcv_host.a $(HOST_CFLAGS) $(HOST_LDFLAGS) -o test

target: test.c cgi_target lib/liblcv_target.a inc/*.h
	$(TARGET_CC) test.c lib/liblcv_target.a $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o test
	cp test /tftpboot

cgi_host: cgitest.c
	$(HOST_CC) cgitest.c lib/liblcv_host.a $(HOST_CFLAGS) -o cgitest

cgi_target: cgitest.c
	$(TARGET_CC) cgitest.c lib/liblcv_target.a $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o cgitest
	cp cgitest /tftpboot
	
get:
	rm -r inc lib || continue
	cp -r ../framework/staging/* . 

clean: 
	rm -f test
