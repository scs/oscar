# Makefile for the Oscar Framework.
# Copyright (C) 2008 Supercomputing Systems AG
# 
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
# 
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this library; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# The Library name is suffix depending on the target
OUT = libosc
HOST_SUFFIX = _host.a
TARGET_SUFFIX = _target.a
TARGET_SIM_SUFFIX = _target_sim.a

# Disable make's built-in rules
MAKEFLAGS += -r

# this includes the framework configuration
# MAKEFILES += .config
-include .config

  # The names of the subfolders with the modules
  MODULES = cam
  MODULES += log
  MODULES += cpld
  MODULES += sim
  MODULES += bmp
  MODULES += swr
  MODULES += srd
  MODULES += ipc
  MODULES += sup
  MODULES += frd
  MODULES += dspl
  MODULES += dma
  MODULES += hsm
  MODULES += cfg
  MODULES += clb
  MODULES += vis
  MODULES += gpio
  
# decide whether we are building or dooing something other like cleaning or configuring
ifeq ($(filter $(MAKECMDGOALS), clean distclean config), )
  # check whether a .config file has been found
  $(info $(MAKEFILE_LIST))
  ifeq ($(filter .config,$(MAKEFILE_LIST)), )
    $(error "Cannot make the target '$(MAKECMDGOALS)' without configuring the framework. Please run make config to do this.")
  endif
  
# Prevent using a cpld firmware when compiling for the LEANXCAM target
  ifeq ($(CONFIG_BOARD), LEANXCAM)
    CONFIG_FIRMWARE =
  endif
  
  # The type of the hardware platform. Must be either of the following:
  # TARGET_TYPE_INDXCAM		Industrial OpenSourceCamera Platform
  # TARGET_TYPE_LEANXCAM		Original OpenSourceCamera Platform
  ifeq ($(CONFIG_BOARD), INDXCAM)
    TARGET_TYPE = TARGET_TYPE_INDXCAM
  else ifeq ($(CONFIG_BOARD), LEANXCAM)
    TARGET_TYPE = TARGET_TYPE_LEANXCAM
  else
    $(error Neither INDXCAM nor LEANXCAM has been configured as target)
  endif
  
  # This may need to be generalized by a board-to-feature-mapping table
  ifeq ($(CONFIG_BOARD), INDXCAM)
    ifeq ($(CONFIG_FIRMWARE), )
      $(error The INDXCAM target requires a firmware.)
    endif
  endif
  
  # The lgx module may be configured to not being used, so it needs special treatment
  ifneq ($(CONFIG_FIRMWARE), )
    MODULES += lgx
  endif
endif

# Directories where the library and header files are placed after 
# compilation
STAGING_DIR = staging

# Header files needed by an application using this framework
FW_HEADERS = oscar.h oscar_error.h oscar_dependencies.h

# Header file suffix for headers marked as public by modules
MOD_HEADER_SUFFIX = _pub.h

# Executable to create the static library
HOST_CREATE_LIB = ar rcs
TARGET_CREATE_LIB = bfin-uclinux-ar rcs

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -O2 -I./ -DOSC_HOST -D$(TARGET_TYPE) -g 

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = -Wall -pedantic -ggdb3 -I./ -DOSC_TARGET -D$(TARGET_TYPE)

# Source files of the camera module
SOURCES = oscar.c

# Default target
all: $(OUT)

$(OUT): target host target_sim

# Compiles the library and moves everything to a staging directory
target_sim: oscar_target modules_target_sim lib_target_sim
	@echo "Moving to target staging directory..."
	@mkdir -p $(STAGING_DIR)/inc
	@mkdir -p $(STAGING_DIR)/lib
	@# Creating a staging dir with all necessary data for the application
	@mv $(OUT)$(TARGET_SIM_SUFFIX) $(STAGING_DIR)/lib
	@for i in $(FW_HEADERS) ; do  \
		cp $$i $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@cp oscar_types_target.h $(STAGING_DIR)/inc/
	@cp oscar_target.h $(STAGING_DIR)/inc/
	@for i in $(MODULES) ; do \
		cp $$i/*$(MOD_HEADER_SUFFIX)  $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@echo "Target framework done."
	
# Compiles the library and moves everything to a staging directory
target: oscar_target modules_target lib_target
	@echo "Moving to target staging directory..."
	@mkdir -p $(STAGING_DIR)/inc
	@mkdir -p $(STAGING_DIR)/lib
	@# Creating a staging dir with all necessary data for the application
	@mv $(OUT)$(TARGET_SUFFIX) $(STAGING_DIR)/lib
	@for i in $(FW_HEADERS) ; do  \
		cp $$i $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@cp oscar_types_target.h $(STAGING_DIR)/inc/
	@cp oscar_target.h $(STAGING_DIR)/inc/
	@for i in $(MODULES) ; do \
		cp $$i/*$(MOD_HEADER_SUFFIX)  $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@echo "Target framework done."
	
# Compiles the library and moves everything to a staging directory
host: oscar_host modules_host lib_host
	@echo "Moving to host staging directory..."
	@mkdir -p $(STAGING_DIR)/inc
	@mkdir -p $(STAGING_DIR)/lib
	@# Creating a staging dir with all necessary data for the application
	@mv $(OUT)$(HOST_SUFFIX) $(STAGING_DIR)/lib/
	@for i in $(FW_HEADERS) ; do  \
		cp $$i $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@cp oscar_types_host.h $(STAGING_DIR)/inc/
	@cp oscar_host.h $(STAGING_DIR)/inc/
	@for i in $(MODULES) ; do\
		cp $$i/*$(MOD_HEADER_SUFFIX)  $(STAGING_DIR)/inc/ || exit $? ; \
	done
	@echo "Host framework done."
	
# Compile the framework main object file
oscar_host: $(SOURCES) oscar.h oscar_priv.h
	$(HOST_CC) $(HOST_CFLAGS) -c $(SOURCES) -o oscar_host.o
	
oscar_target: $(SOURCES) oscar.h oscar_priv.h
	$(TARGET_CC) $(TARGET_CFLAGS) -c $(SOURCES) -o oscar_target.o
	
# Compile the modules
modules_target:
	for i in $(MODULES) ; do  make target EXTRA_CFLAGS="-D$(TARGET_TYPE)" -C $$i  || exit $? ; done
	
modules_target_sim:
	for i in $(MODULES) ; do  make target EXTRA_CFLAGS="-D$(TARGET_TYPE)" -C $$i || exit $? ; done

modules_host:
	for i in $(MODULES) ; do  make host EXTRA_CFLAGS="-D$(TARGET_TYPE)" -C $$i || exit $? ; done
	
# Create the library
lib_target:
	$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SUFFIX) oscar_target.o
	for i in $(MODULES) ; do  \
		$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SUFFIX) $$i/*_target.o || \
		exit $? ; \
	done
	@echo "Library for Blackfin created."

lib_target_sim:	
	$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SIM_SUFFIX) oscar_target.o
	for i in $(MODULES) ; do  \
		$(TARGET_CREATE_LIB) $(OUT)$(TARGET_SIM_SUFFIX) $$i/*_sim.o ||  \
		exit $? ; \
	done
	@echo "Library for Blackfin created."
	
lib_host:
	$(HOST_CREATE_LIB) $(OUT)$(HOST_SUFFIX) oscar_host.o
	for i in $(MODULES) ; do  \
		$(HOST_CREATE_LIB) $(OUT)$(HOST_SUFFIX) $$i/*_host.o || \
		exit $? ; \
	done
	@echo "Library for Host created"

# Target to explicitly start the configuration process
.PHONY: config
config:
	@ ./configure
	@ $(MAKE) --no-print-directory get_lgx

# Target to implicitly start the configuration process
#.config:
#	@ echo "No config file has been found. Starting the configuration process now:"
#	@ ./configure
#	@ $(MAKE) --no-print-directory get_lgx

# Target to get the lgx framework explicitly
.PHONY: get_lgx
get_lgx: .config
ifeq ($(CONFIG_FIRMWARE), )
	@ echo "No firmware has been configured."
else
	@ echo "Copying the lgx firmware from $(CONFIG_FIRMWARE)"
	@ [ -e "lgx" ] && rm -rf "lgx"; exit 0
	@ cp -r $(CONFIG_FIRMWARE)/lgx .
endif

## Target to get the lgx framework implicitly
#lgx: 
#	$(MAKE) get_lgx

# Cleanup
.PHONY: clean
clean:
	for i in $(MODULES) ; do  make clean -C $$i || exit $? ; done
	rm -f $(OUT)$(HOST_SUFFIX) $(OUT)$(TARGET_SUFFIX)
	rm -rf $(STAGING_DIR)
	rm -f *.o
	@echo "Directory cleaned"

# Cleans everything not intended for source distribution
.PHONY: distclean
distclean: clean
	rm -f .config
	rm -rf lgx
