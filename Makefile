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

# Disable make's built-in rules.
MAKE += -RL --no-print-directory
SHELL := $(shell which bash)

# This includes the framework configuration.
include Makefile_config

# Executable to create the static library
AR_host := ar -rcs
AR_target := bfin-uclinux-ar -rcs

# Modes to compile this module in.
MODES := host target target_sim
MODES += $(addsuffix _dbg, $(MODES))

# Helper function to access stacked, eg. mode-dependent variables.
varnames = $(filter $(.VARIABLES), $(foreach i, $(shell seq 1 $(words $(subst _, , $(1)))), $(subst $() ,_,$(wordlist 1, $i, $(subst _, , $(1))))))
firstvar = $($(lastword $(call varnames, $(1))))
allvars = $(foreach i, $(call varnames, $(1)), $($i))

# Default target which builds all modules for the selected board configuration.
.PHONY: all $(MODES)
all: $(MODES)

# Targets to build in a specific build mode and create the library.
$(MODES): %: library/libosc_%.a
library/libosc_%.a: modules_% oscar_%
	$(call firstvar, AR_$*) $@ $(addsuffix /build/*_$*.o, $(MODULES) .)

# Targets to compile the modules only in a specific mode.
MODULE_TARGETS := $(addprefix modules_, $(MODES))
.PHONY: $(MODULE_TARGETS)
$(MODULE_TARGETS): modules_%: $(addsuffix /%, $(MODULES)) needs_config

# Allow a module to be given on the command line to build that module.
.PHONY: $(MODULES)
$(MODULES):
	$(MAKE) -C $@

# Call this as oscar_$(MODE) to only build them main oscar files.
.PHONY: oscar_%
oscar_%:
	$(MAKE) -f Makefile_module $*

# Produce a target of the form "foo/%" for every directory foo that contains a Makefile
define subdir_target
.PHONY: $(1)%
$(1)%:
	$(MAKE) -C $(1) $$*
endef
SUBDIRS := $(dir $(wildcard */Makefile))
$(foreach i, $(SUBDIRS), $(eval $(call subdir_target, $(i))))

# Routing individual object file requests directly to the compile Makefile
.PHONY: %.o
%.o: needs_config
	$(MAKE) -f Makefile_module $@

# Use this target as a prerequisite in a target that should fail if the framework has not yet been configured.
.PHONY: needs_config
needs_config:
	@ [ -e ".config" ] || { echo "The framework has to be configured using 'make config' first!"; false; }
$(sort $(MAKEFILE_LIST) .config):;

# Target to explicitly start the configuration process.
.PHONY: config
config:
	@ ./configure

# Target that gets called by the configure script after the configuration.
.PHONY: reconfigure
reconfigure: needs_config
	ln -sf "../boards/$(CONFIG_BOARD).h" "include/board.h"
ifeq '$(CONFIG_USE_FIRMWARE)' 'y'
	@ if ! [ -e "lgx" ] || [ -h "lgx" ]; then ln -fs $(CONFIG_FIRMWARE_PATH) "lgx"; else echo "Warning: The symlink to the lgx module could not be created as the file ./lgx already exists and is something other than a symlink. Pleas remove it and run 'make reconfigure' to create the symlink."; fi
endif

# Builds the doxygen documentation.
.PHONY: doc
doc:
	rm -rf doc/{html,latex}
	doxygen documentation/oscar.doxygen
	ln -sf html/index.html documentation/index.html

# Cleans the framework and all modules
.PHONY: clean
clean: %: $(addsuffix /%, $(SUBDIRS)) oscar_clean
	rm -rf library/*.a
	rm -rf doc/{html,latex,index.html}

# Cleans everything not intended for source distribution
.PHONY: distclean
distclean: clean
	rm -f .config
	rm -rf lgx
