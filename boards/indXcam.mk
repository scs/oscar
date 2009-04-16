MODULES := bmp cam cfg clb cpld dma dspl frd gpio hsm ipc jpg lgx log sim srd sup swr vis

# This target will be called after the configuration process if this board has been selected.
$(RECONFIGURE):
ifneq '$(CONFIG_FIRMWARE_PATH)' ''
	@ if ! [ -e "lgx" ] || [ -h "lgx" ]; then ln -fs $(CONFIG_FIRMWARE_PATH) "lgx"; else echo "Warning: The symlink to the lgx module could not be created as the file ./lgx already exists and is something other than a symlink. Pleas remove it and run 'make reconfigure' to create the symlink."; fi
endif
