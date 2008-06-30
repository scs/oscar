/*! @file clb_pub.h
 * @brief API definition for calibration module
 *
 * Provides support for camera calibration including correction methods as: 
 * FPN, PRNU, hotpixel. The specific methods are configurable.
 *
 * In case no reference the module is used by the application, the image 
 * corretion is disabled. This save time and memory in case no calibration
 * is required.
 * 
 * @author Samuel Zahnd
 */
#ifndef CLB_PUB_H_
#define CLB_PUB_H_

#include "framework_error.h"

#define MAX_NR_HOTPIXEL         1000    /*!< @brief Max number of hotpixels */

/*! Module-specific error codes. */
/*enum EnLcvClbErrors
 *{
 *    Exxx = LCV_CLB_ERROR_OFFSET
 *}; */


/*! @brief The different sensor calibration modes */
enum EnLcvClbCalibrateSlope
{
    /*! @brief No slope calibration data is applied */
    LCV_CLB_CALIBRATE_OFF,        
    /*! @brief Fix Pattern Noise (offset) */
    LCV_CLB_CALIBRATE_FPN,    
    /*! @brief Photo Receptive Non-Uniformity (gain) */
    LCV_CLB_CALIBRATE_PRNU,     
    /*! @brief combined method */
    LCV_CLB_CALIBRATE_FPN_PRNU  
};


/*========================== API functions =============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * Initialize the member variables and set everything up for later.
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVClbCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVClbDestroy(void *hFw);


/*********************************************************************//*!
 * @brief Setup the sensor calibration method.
 * 
 * To compensate for CMOS sensor non-linearity different calibration 
 * methods are applicabale: slope correction, hot pixel correction.
 * The required calibration data is loaded from dedicated Flash partition
 * and stored to SDRAM.
 * 
 * Host: No calibration is applied.
 * 
 * @param calibSlope Used model for slope calibration
 * @param bHotpixel  Enable hotpixel removal
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVClbSetupCalibrate(
        enum EnLcvClbCalibrateSlope calibSlope,
        bool bHotpixel);

/*********************************************************************//*!
 * @brief Apply image correction.
 * 
 * Image correction is usually called from the camera module in 
 * conjunction with LCVCamReadPicture postprocessing. Therefor the 
 * function can be registered with LCVCamRegisterPostProc.
 * 
 * @see LCVClbSetupCalibrate To configure the correction methods.
 * 
 * Host: No effect
 * 
 * @param pImg   image for in-place correction
 * @param lowX X coordinate limiting the capture rectangle on the left side
 * @param lowY Y coordinate limiting the capture rectangle on the bottom side
 * @param width  Width of the rectangle.
 * @param height Height of the rectangle. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVClbApplyCorrection( uint8 *pImg,
        const uint16 lowX, const uint16 lowY,
        const uint16 width, const uint16 height);                
        



#endif /*CLB_PUB_H_*/
