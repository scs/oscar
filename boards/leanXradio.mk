MODULES := bmp cam cfg cpld dma dspl frd gpio hsm ipc jpg lgx log sim srd sup swr vis

# This target will be called after the configuration process if this board has been selected.
$(RECONFIGURE):
	boards/$(CONFIG_BOARD).sh lgx $(CONFIG_FIRMWARE_PATH)
