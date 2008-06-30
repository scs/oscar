/*! @file clb_priv.h
 * @brief Private calibration module definitions
 * 
 * @author Samuel Zahnd
 */
#ifndef CLB_PRIV_H_
#define CLB_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef LCV_HOST
#include <framework_types_host.h>
#endif
#ifdef LCV_TARGET
#include <framework_types_target.h>
#endif

#include "../cam/mt9v032.h"

/*! @brief The calibration data file */
#define CALIB_FILE "/calib"
/*! @brief The calibration file's magic number */
#define CALIB_MAGIC 0x12345678

/*! @brief Generic two-dimensional vector. */
struct VEC_2D
{
    uint16 x;
    uint16 y;
};

/*! @brief The structure representing sensor calibration data
  */
struct LCV_CLB_CALIBRATION_DATA
{
    /*! @brief FixPatternNoise (offset) correction: 
     * 5bit [0..32), lsb aligend */
    uint8 fpn[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT];
    /*! @brief PhotoResponseNonUniformity (gain) correction:
     * fixpoint [0..16), 00000xxx.xxxxxxxx
     * 3bit integeter part
     * 8bit fraction part 
     * */    
    uint16 prnu[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT];
    /*! @brief Number of identified hotpixels */
    uint16 nHotpixel;
    /*! @brief Coordinates of all hotpixels */
    struct VEC_2D hotpixels[MAX_NR_HOTPIXEL];        
};

/*! @brief Object struct of the clb module. This
 * sturcture root is placed cache alined. */
struct LCV_CLB
{  
#ifdef LCV_TARGET      
    /*! Sensor calibration configuration data.
     * Must be cache-line aligned! */
    struct LCV_CLB_CALIBRATION_DATA calibData;        
    
    /*! @brief Model for slope calibration correction */
    enum EnLcvClbCalibrateSlope calibSlope;
    /*! @brief Enable hotpixel removal */
    bool bHotpixel;    
    
    /*! @brief The current 'area-of-interest' capture window */
    struct capture_window capWin;
#endif /*LCV_TARGET*/    
    uint16 dummy;    
};

/*======================= Private methods ==============================*/

#ifdef LCV_TARGET
/*********************************************************************//*!
 * @brief Load the calibration data from file.
 * 
 * @param strCalibFN File name of the calibration file.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
LCV_ERR LoadCalibrationData(const char strCalibFN[]);

#if 0
/*********************************************************************//*!
 * @brief Store the calibration data to file.
 * 
 * @param strCalibFN File name of the calibration file.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
LCV_ERR StoreCalibrationData(const char strCalibFN[]);
#endif /*0*/

/*********************************************************************//*!
 * @brief Target only: Correct FPN (offset) and PRNU (gain) 
 * 
 * Correction is applied in place on the current frame's AOI.
 * 
 * @param pImg  Pointer to image data.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVClbCorrectFpnPrnu(uint8* pImg);

/*********************************************************************//*!
 * @brief Target only: Interpolate hotpixels 
 * 
 * Hotpixels are interpolated with the neighours to the left and right.
 * 
 * @param pImg  Pointer to image data.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVClbCorrectHotpixel(uint8* pImg);
#endif /*LCV_TARGET*/

#endif /* CLB_PRIV_H_ */
