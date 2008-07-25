/*! @file cam_priv.h
 * @brief Private camera module definitions
 * 
 */
#ifndef CAM_PRIV_H_
#define CAM_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef OSC_HOST
#include <oscar_types_host.h>
#endif
#ifdef OSC_TARGET
#include <oscar_types_target.h>
#endif

#if defined(OSC_HOST) || defined(OSC_SIM)
/* To load test images from bmp's */
#include "bmp/bmp_pub.h" 
/* Generate file names for test images depending on time step */
#include "frd/frd_pub.h"
#endif /* OSC_HOST  or OSC_SIM*/

/*! @brief For communication with the frame capture device driver */
#include "mt9v032.h" 
/*! @brief The multi buffer class */
#include "cam_multibuffer.h"

/*! @brief The device node which represents the camera */
#define VIDEO_DEVICE_FILE "/dev/video0"
/*! @brief Maximum number of registers in the camera sensor */
#define NUM_CAM_REGS 256

#define	CAM_PIX_CLK 			25000000 	/*!< @brief 25 Mhz PixClk */
#define CAM_ROW_TIME 			846 		/*!< @brief PixCkls per row */
#define CAM_EXPOSURE            15000       /*!< @brief Default exposure time [us]*/
#define CAM_BLACKLEVEL          13          /*!< @brief Offset for black image */

/* Adressmap of used registers in image sensor */
#define CAM_REG_CHIP_CONTROL    0x07
#define CAM_REG_SHUTTER_WIDTH 	0x0B
#define CAM_REG_RESET           0x0C
#define CAM_REG_READ_MODE       0x0D
    #define CAM_REG_READ_MODE_ROW_FLIP    4
    #define CAM_REG_READ_MODE_COL_FLIP    5
#define CAM_REG_PIXEL_OP_MODE   0x0F
#define CAM_REG_RESERVED_0x20   0x20
#define CAM_REG_AEC_AGC_ENA     0xAF
#define CAM_REG_ROW_NOISE_CONST 0x72
#define CAM_REG_HORIZ_BLANK     0x05
#define CAM_REG_COL_START       0x01
#define CAM_REG_ROW_START       0x02
#define CAM_REG_WIN_HEIGHT      0x03
#define CAM_REG_WIN_WIDTH       0x04
#define CAM_REG_LED_OUT_CONTROL 0x1B

/*! @brief The minimum row time of the mt9v032 sensor in pixel clocks. 
 * Used for shutter width calculations. */
#define CAM_MIN_ROW_CLKS 660

/*! @brief The config file used to set up the filename reader. */
#define FILENAME_READER_CONFIG_FILE "cam.frdconf"

#if defined(OSC_HOST) || defined(OSC_SIM)
/*! @brief Host only: The different states a frame buffer can be in */
enum EnOscFrameBufferStatus {
    STATUS_UNITIALIZED,
    STATUS_READY,
    STATUS_CAPTURING_SINGLE,
    STATUS_VALID,
    STATUS_CORRUPTED
};
#endif /* OSC_HOST or OSC_SIM*/
/*! @brief Object struct of the camera module */
struct OSC_CAM
{
    /*! @brief The configured frame buffers */
    struct frame_buffer fbufs[MAX_NR_FRAME_BUFFERS];
    /*! @brief Current number of frame buffers */
    uint16 nFrameBuffers;
    /*! @brief Describes a multi buffer for automatic buffer management
     * if activated */
    struct OSC_CAM_MULTIBUFFER multiBuffer;
    /*! @brief Video driver device file descriptor */
    int vidDev;

    /*! @brief The current 'area-of-interest' capture window */
    struct capture_window capWin;
    
    /*! @brief Sensor register: flip image horizontally */
    bool flipHorizontal;
    /*! @brief Sensor register: flip image vertically */
    bool flipVertical;
    
    /*! @brief The number of pixel clocks spent to read out a row from
     * the camera. */
    uint32 curCamRowClks;
    /*! @brief The current horizontal blanking time in pixel clocks. */
    uint16 curHorizBlank;
    /*! @brief The current exposure time in microseconds. */
    uint32 curExpTime;
    
#ifdef OSC_TARGET   
    /*! @brief Pointer to callback fxn for image correction */      
    int (*pCallback)(
            uint8 *pImg,
                    const uint16 lowX, 
                    const uint16 lowY,
                    const uint16 width, 
                    const uint16 height);
#endif /*OSC_TARGET*/   
    
    /* Members only needed for the host implementation */
#if defined(OSC_HOST) || defined(OSC_SIM) 
    /*! @brief Host only: All registers of the mt9v032 CMOS sensor */
    struct reg_info regs[NUM_CAM_REGS];
    /*! @brief Host only: The current status of the frame buffers */
    enum EnOscFrameBufferStatus fbStat[MAX_NR_FRAME_BUFFERS];
    /*! @brief Host only: The capture window during the last call to
     * OscCamSetupCapture() */
    struct capture_window lastCapWin;
    /*! @brief Host only: ID of the last buffer where a valid image was 
     * captured
     * */
    uint8 lastValidID;
    /*! @brief Host only: The handle to the file name reader used to 
     * generate the file names of the test images. */
    void *hFNReader;
#endif /* OSC_HOST or OSC_SIM*/
};

/*======================= Private methods ==============================*/  

#endif /* CAM_PRIV_H_ */
