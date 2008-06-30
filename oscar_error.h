/*! @file oscar_error.h
 * @brief Error declarations for OSC framework
 * 
 * @date 22.1.2008
 * @version 1.0
 */
#ifndef FRAMEWORK_ERROR_H_
#define FRAMEWORK_ERROR_H_

/*! @brief Define general non-module-specific
 * error codes for the OSC framework */
enum EnOscErrors {
		SUCCESS = 0,
		EOUT_OF_MEMORY,
		ETIMEOUT,
		EUNABLE_TO_OPEN_FILE,
		EINVALID_PARAMETER,
		EDEVICE,
		ENOTHING_TO_ABORT,
		EDEVICE_BUSY,
		ECANNOT_DELETE,
		EBUFFER_TOO_SMALL,
		EFILE_ERROR,
		ECANNOT_UNLOAD,
		ENR_OF_INSTANCES_EXHAUSTED,
		EFILE_PARSING_ERROR,
	    EALREADY_INITIALIZED,
	    ENO_SUCH_DEVICE,
	    EUNABLE_TO_READ,
	    ETRY_AGAIN
};

/* Define an offset for all modules, which allows it to
 * define module-specific errors that do not overlap.
 */
/*! @brief Error identifier offset of the cam module. */
#define OSC_CAM_ERROR_OFFSET 	100
/*! @brief Error identifier offset of the cpld module. */
#define OSC_CPLD_ERROR_OFFSET 	200
/*! @brief Error identifier offset of the lgx module. */
#define OSC_LGX_ERROR_OFFSET    300
/*! @brief Error identifier offset of the log module. */
#define OSC_LOG_ERROR_OFFSET    400
/*! @brief Error identifier offset of the sim module. */
#define OSC_SIM_ERROR_OFFSET    500
/*! @brief Error identifier offset of the bmp module. */
#define OSC_BMP_ERROR_OFFSET    600
/*! @brief Error identifier offset of the swr module. */
#define OSC_SWR_ERROR_OFFSET    700
/*! @brief Error identifier offset of the srd module. */
#define OSC_SRD_ERROR_OFFSET    800
/*! @brief Error identifier offset of the ipc module. */
#define OSC_IPC_ERROR_OFFSET    900
/*! @brief Error identifier offset of the frd module. */
#define OSC_FRD_ERROR_OFFSET    1000
/*! @brief Error identifier offset of the dma module. */
#define OSC_DMA_ERROR_OFFSET    1100
/*! @brief Error identifier offset of the hsm module. */
#define OSC_HSM_ERROR_OFFSET    1200
/*! @brief Error identifier offset of the cfg module. */
#define OSC_CFG_ERROR_OFFSET    1300
/*! @brief Error identifier offset of the clb module. */
#define OSC_CLB_ERROR_OFFSET    1400

#endif /*FRAMEWORK_ERROR_H_*/
