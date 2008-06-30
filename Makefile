############################################################################
#
# Makefile for the LCV Framework
#
# Author: Markus Berner
# 
############################################################################

# The Library name is suffix depending on the target
OUT = liblcv
HOST_SUFFIX = _host.a
TARGET_SUFFIX = _target.a
TARGET_SIM_SUFFIX = _target_sim.a

# The type of the hardware platform. Must be either of the following:
# TARGET_TYPE_LCV_IND		Industrial Low-Cost-Vision Platform
# TARGET_TYPE_LCV		Original Low-Cost-Vision Platform
TARGET_TYPE = TARGET_TYPE_LCV

# The names of the subfolders with the modules
MODULES = cam
MODULES += log
MODULES += cpld
MODULES += sim
MODULES += bmp
MODULES += lgx
MODULES += swr
MODULES += srd
MODULES += ipc
MODULES += sup
MODULES += frd
MODULES += rtl
MODULES += dma
MODULES += hsm
MODULES += cfg
MODULES += clb
MODULES += vis

# Directories where the library and header files are placed after 
# compilation
STAGING_DIR = staging

# Header files needed by an application using this framework
FW_HEADERS = framework.h framework_error.h framework_dependencies.h

# Header file suffix for headers marked as public by modules
MOD_HEADER_SUFFIX = _pub.h

# Executable to create the static library
HOST_CREATE_LIB = ar rcs
TARGET_CREATE_LIB = bfin-uclinux-ar rcs

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -O2 -I./ -DLCV_HOST -D$(TARGET_TYPE) -g 

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = -Wall -pedantic -ggdb3 -I./ -DLCV_TARGET -D$(TARGET_TYPE)

# Source files of the camera module
SOURCES = framework.c

# Default target
all: $(OUT)

$(OUT): target host target_sim

# Compiles the library and moves everything to a staging directory
target_sim: framework_target modules_target_sim lib_target_sim
	@echo "Moving to target staging directory..."
	@mkdir -p $(STAGING_DIR)/inc
	@mkdir -p $(STAGING_DIR)/lib
	@# Creating a staging dir with all necessary data for the application
	@mv $(OUT)$(TARGET_SIM_SUFFIX) $(STAGING_DIR)/lib
	@for i in $(FW_HEADERS) ; do  \
		cp $$i $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@cp framework_types_target.h $(STAGING_DIR)/inc/
	@cp framework_target.h $(STAGING_DIR)/inc/
	@for i in $(MODULES) ; do \
		cp $$i/*$(MOD_HEADER_SUFFIX)  $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@echo "Target framework done."
	
# Compiles the library and moves everything to a staging directory
target: framework_target modules_target lib_target
	@echo "Moving to target staging directory..."
	@mkdir -p $(STAGING_DIR)/inc
	@mkdir -p $(STAGING_DIR)/lib
	@# Creating a staging dir with all necessary data for the application
	@mv $(OUT)$(TARGET_SUFFIX) $(STAGING_DIR)/lib
	@for i in $(FW_HEADERS) ; do  \
		cp $$i $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@cp framework_types_target.h $(STAGING_DIR)/inc/
	@cp framework_target.h $(STAGING_DIR)/inc/
	@for i in $(MODULES) ; do \
		cp $$i/*$(MOD_HEADER_SUFFIX)  $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@echo "Target framework done."
	
# Compiles the library and moves everything to a staging directory
host: framework_host modules_host lib_host
	@echo "Moving to host staging directory..."
	@mkdir -p $(STAGING_DIR)/inc
	@mkdir -p $(STAGING_DIR)/lib
	@# Creating a staging dir with all necessary data for the application
	@mv $(OUT)$(HOST_SUFFIX) $(STAGING_DIR)/lib/
	@for i in $(FW_HEADERS) ; do  \
		cp $$i $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@cp framework_types_host.h $(STAGING_DIR)/inc/
	@cp framework_host.h $(STAGING_DIR)/inc/
	@for i in $(MODULES) ; do\
		cp $$i/*$(MOD_HEADER_SUFFIX)  $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@echo "Host framework done."
	
# Compile the framework main object file
framework_host: $(SOURCES) framework.h framework_priv.h
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES) -o framework_host.o
	
framework_target: $(SOURCES) framework.h framework_priv.h
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES) -o framework_target.o
	
# Compile the modules
modules_target: 
	for i in $(MODULES) ; do  make target EXTRA_CFLAGS="-D$(TARGET_TYPE)" -C $$i  || exit $? ; done
	
modules_target_sim: 
	for i in $(MODULES) ; do  make target EXTRA_CFLAGS="-D$(TARGET_TYPE)" -C $$i || exit $? ; done

modules_host:
	for i in $(MODULES) ; do  make host EXTRA_CFLAGS="-D$(TARGET_TYPE)" -C $$i || exit $? ; done
	
# Create the library
lib_target:
	$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SUFFIX) framework_target.o
	for i in $(MODULES) ; do  \
		$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SUFFIX) $$i/*_target.o || \
		exit $? ; \
	done
	@echo "Library for Blackfin created."

lib_target_sim:	
	$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SIM_SUFFIX) framework_target.o
	for i in $(MODULES) ; do  \
		$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SIM_SUFFIX) $$i/*_sim.o ||  \
		exit $? ; \
	done
	@echo "Library for Blackfin created."
	
lib_host:
	$(HOST_CREATE_LIB) $(OUT)$(HOST_SUFFIX) framework_host.o
	for i in $(MODULES) ; do  \
		$(HOST_CREATE_LIB) $(OUT)$(HOST_SUFFIX) $$i/*_host.o || \
		exit $? ; \
	done
	@echo "Library for Host created"
	
# Cleanup
clean:	
	for i in $(MODULES) ; do  make clean -C $$i || exit $? ; done
	rm -f $(OUT)$(HOST_SUFFIX) $(OUT)$(TARGET_SUFFIX)
	rm -rf $(STAGING_DIR)
	rm -f framework.o
	@echo "Directory cleaned"

	
