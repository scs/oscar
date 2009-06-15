/*	Oscar, a hardware abstraction framework for the LeanXcam and IndXcam.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*! @file
 * @brief API definition for camera module
 * 
 */
#ifndef CAM_PUB_H_
#define CAM_PUB_H_

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscCamErrors
{
	ENO_VIDEO_DEVICE_FOUND = OSC_CAM_ERROR_OFFSET,
	EPICTURE_TOO_OLD,
	ENO_MATCHING_PICTURE,
	ENO_CAPTURE_STARTED,
	EFRAME_BUFFER_BUSY,
	EPICTURE_TOO_SMALL,
	ENO_AREA_OF_INTEREST_SET,
	ENO_COLOR_SENSOR
};

/*! @brief Camera perspective adaptation */
enum EnOscCamPerspective
{
	OSC_CAM_PERSPECTIVE_DEFAULT,
	OSC_CAM_PERSPECTIVE_HORIZONTAL_MIRROR,
	OSC_CAM_PERSPECTIVE_VERTICAL_MIRROR,
	OSC_CAM_PERSPECTIVE_180DEG_ROTATE
};

/*! @brief Configuration value string for default perspective */
#define OSC_CAM_PERSPECTIVE_CFG_STR_DEFAULT              "DEFAULT"
/*! @brief Configuration value string for horizontal mirrored perspective*/
#define OSC_CAM_PERSPECTIVE_CFG_STR_HORIZONTAL_MIRROR    "HMIRROR"
/*! @brief Configuration value string for vertical mirrored perspective */
#define OSC_CAM_PERSPECTIVE_CFG_STR_VERTICAL_MIRROR      "VMIRROR"
/*! @brief Configuration value string for 180deg rotated perspective */
#define OSC_CAM_PERSPECTIVE_CFG_STR_180DEG_ROTATE        "ROT180"

/*! @brief Specify this as frame buffer number if you want to access
 * the multi buffer. */
#define OSC_CAM_MULTI_BUFFER 254
/*! @brief Buffer ID of an invalid buffer. */
#define OSC_CAM_INVALID_BUFFER_ID 255

/*! @brief The width of the biggest image that can be captured with this
 * sensor. */
#define OSC_CAM_MAX_IMAGE_WIDTH 752
/*! @brief The height of the biggest image that can be captured with this
 * sensor. */
#define OSC_CAM_MAX_IMAGE_HEIGHT 480

/*! @brief The order in which the colored pixels of a bayer pattern
 * appear in a row.
 * 
 * The colors are abbreviated as follows:
 * - G: Green
 * - R: Red
 * - B: Blue
 * 
 * The enum is constructed from two bools; one saying whether the first
 * pixel in the row is green and the other whether it is a red or blue
 * row.
 *          firstGreen      firstOther
 * red          11              01
 * blue         10              00
 * */
enum EnBayerOrder {
	ROW_BGBG = 0,
	ROW_RGRG = 1,
	ROW_GBGB = 2,
	ROW_GRGR = 3
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
OSC_ERR OscCamCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscCamDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Host only: Set the file name reader the host implementation
 * reads the pictures from.
 * 
 * Host only:
 * If the camera configuration file is present, the filename reader
 * is opened according to the information contained therein.
 * If not, the file name reader has to be set over this function for
 * the host.
 * It can be changed afterwards, but the old file name handle cannot
 * be closed and thus it will produce a warning.
 * 
 * @param hReaderHandle The handle of the reader to be set.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamSetFileNameReader(void* hReaderHandle);

/*********************************************************************//*!
 * @brief Set the rectangle read out from the CMOS sensor
 * 
 * @see OscCamGetAreaOfInterest
 * Writes the configuration to limit the image format captured by the CMOS
 * sensor to the specified values. Becomes valid with the next image
 * captured.
 * ! Only even widths are supported !
 * To restore the default (full) image size, specify 0 for all parameters.
 * 
 * @param lowX X coordinate limiting the capture rectangle on the left
 * side
 * @param lowY Y coordinate limiting the capture rectangle on the bottom
 * side
 * @param width Width of the rectangle. !Must be an even value!
 * @param height Height of the rectangle.
 * @return SUCCESS or  an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamSetAreaOfInterest(const uint16 lowX, const uint16 lowY,
		const uint16 width, const uint16 height);

/*********************************************************************//*!
 * @brief Read back the rectangle currently being read out by the sensor.
 * 
 * @see OscCamSetAreaOfInterest
 * @param pLowX X coordinate limiting the capture rectangle on the left
 * side
 * @param pLowY Y coordinate limiting the capture rectangle on the bottom
 * side
 * @param pWidth Width of the rectangle. !Must be an even value!
 * @param pHeight Height of the rectangle.
 * @return SUCCESS or  an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamGetAreaOfInterest(uint16 *pLowX,
		uint16 *pLowY,
		uint16 *pWidth,
		uint16 *pHeight);

/*********************************************************************//*!
 * @brief Set the exposure time
 * 
 * Write the configuration for the shutter width (exposure) in useconds
 * to the camera sensor. Does become valid with the next image captured.
 * Value is wrapped onto the valid range and rounded. An argument of 0
 * activates the automatic exposure control (AEC) of the sensor.
 * 
 * @param usecs Desired exposure time in microseconds
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamSetShutterWidth(const uint32 usecs);

/*********************************************************************//*!
 * @brief Get the current exposure time
 * 
 * Reads out the current exposure time in microseconds. A return value of
 * 0 indicates that the automatic exposure control (AEC) of the camera
 * is active.
 * 
 * @param pResult Configured exposure time in microseconds
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamGetShutterWidth(uint32 *pResult);

/*********************************************************************//*!
 * @brief Set the black level offset
 * 
 * Write the Row Noise Constant to the camera sensor. The parameter is
 * used to control the black-level response. In conjunction with the
 * sensor calibration this may have to be adapted in order to get a
 * undistorted gaussian response for a black image (lens covered).
 * By default sensor applies a certain offset.
 * A offset increment generated a boosted sensor response by one grey
 * level (8bit output mode). Maximum offset is 63.
 * This offset parameter is used only in function with the enabled
 * row noise cancellation algorithm.
 * 
 * @param offset Boost the sensor response by N grey levels
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamSetBlackLevelOffset(const uint16 offset);

/*********************************************************************//*!
 * @brief Get the current black level offset
 * 
 * @param pOffset Configured exposure time in microseconds
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamGetBlackLevelOffset(uint16 *pOffset);

/*********************************************************************//*!
 * @brief Set a register value.
 * 
 * Sets the register described by reg to the specified value.
 * This should only be used for testing an debugging and the
 * framework should be extended by designated functions
 * controlling the used registers.
 * 
 * @param reg Identifier specifying the register to write to
 * @param value What is to be written into reg.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamSetRegisterValue(const uint32 reg, const uint16 value);

/*********************************************************************//*!
 * @brief Read a register value
 * 
 * Reads the register described by reg and returns its value.
 * This should only be used for testing an debugging and the
 * framework should be extended by designated functions controlling
 * the used registers.
 * 
 * @param reg Identifier specifying the register to read
 * @param pResult Pointer where the return value should be stored.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamGetRegisterValue(const uint32 reg, uint16 *pResult);

/*********************************************************************//*!
 * @brief Configure multiple frame buffer to form a multi buffer.
 * 
 * This can be used e.g. to allow automatic management of double or
 * triple buffering. Just specify the depth of the multi buffer and the
 * frame buffer IDs forming the multi buffer. Afterwards the commands
 * requiring a frame buffer number can be supplied with
 * OSC_CAM_MULTI_BUFFER to access the data in the automatically managed
 * multi buffer.
 * Only one multi buffer can exist per camera module.
 * The specified frame buffers must first be configured by calls
 * to OscCamCreateFrameBuffer.
 * @see OscCamCreateFrameBuffer
 * @see OscCamDeleteMultiBuffer
 * 
 * @param multiBufferDepth The depth of the multi buffer.
 * @param bufferIDs Array of the buffer IDs forming the multi buffer.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamCreateMultiBuffer(const uint8 multiBufferDepth,
		const uint8 bufferIDs[]);

/*********************************************************************//*!
 * @brief Delete a previously configured multi buffer
 * @see OscCamCreateMultiBuffer
 * 
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamDeleteMultiBuffer();

/*********************************************************************//*!
 * @brief Set one of the frame buffers used by the camera driver
 * 
 * Determines where the camera saves captured pictures. The memory
 * location pointed to by pData needs to provide enough
 * space for the selected image size. When multiple framebuffers
 * are specified, they can be configured as a multi buffer with
 * OscCamCreateMultiBuffer.
 * A frame buffer is unregistered by specifying NULL for pData and
 * a size of zero.
 * Double-Buffering is therefore easily implemented by just
 * specifying 2 framebuffers.
 * @see OscCamCreateMultiBuffer
 * 
 * @param fbID ID of the frame buffer
 * @param uSize Size of the frame buffer
 * @param pData Memory used to store images, allocated by caller.
 * @param bCached True if memory location is cached, false otherwise
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamSetFrameBuffer(const uint8 fbID, const uint32 uSize,
		const void * pData, const int bCached);

/*********************************************************************//*!
 * @brief Prepare the capture of a new picture
 * 
 * This method needs to be called before OscCamReadPicture(). It does not
 * block.
 * The camera sensor must be triggered either through an external trigger
 * signal or over the GPIOs, respectively CPLD, depending on the hardware
 * platform.
 * @see OscCamReadPicture
 * @see OscCamCancelCapture
 * 
 * @param fbID ID of the frame buffer to capture to.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamSetupCapture(uint8 fbID);

/*********************************************************************//*!
 * @brief Cancel the capture of a picture
 * 
 * Cancels a previous call to OscCamSetupCapture if the picture
 * is not yet in the frame-buffer. In case the
 * picture is currently being captured, this action will be
 * cancelled, resulting in a corrupted picture in the frame
 * buffer.
 * Does not block.
 * @see OscCamSetupCapture
 * 
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamCancelCapture();

/*********************************************************************//*!
 * @brief Reads a new picture from the camera.
 * 
 * Blocks until the picture from a previous call to OscCamSetupCapture
 * is stored fully in RAM (the frame-buffer). It then supplies the
 * pointer of the frame-buffer that contains the image. To prevent
 * an infinite blocking on this function a timeout can be supplied.
 * After the timeout, the function should just be called again.
 * Additionally a maximum age of the picture can be specified
 * after which the error -EPICTURE_TOO_OLD is returned. The picture
 * can still be used if desired.
 * No image processing is performed, which means that when reading a
 * picture from a color CMOS sensor, Bayer pattern filtering has to be
 * done before being able to use the color content of the picture.
 * @see OscCamSetupCapture
 * 
 * Host: Implementation does not currently support image age or
 * timeout. The trigger mode is also not relevant as the picture
 * is assumed to be ready as soon as this function is called.
 * The host loads .bmps from the hard disk. The file names are generated
 * by a reader in the Sim module.
 * 
 * @param fbID ID of the framebuffer to read the image from.
 * @param ppPic Location where to save the pointer to the frame-buffer.
 * @param maxAge Maximum age of the returned image in milliseconds. 0
 * means no maximum age.
 * @param timeout Timeout in milliseconds. 0 means no timeout.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscCamReadPicture(const uint8 fbID, uint8 ** ppPic, const uint16 maxAge, const uint16 timeout);

/*********************************************************************//*!
 * @brief Returns the latest picture from the camera
 * 
 * Returns a pointer to the frame-buffer where the last successfully
 * captured picture resides.
 * 
 * @param ppPic Location where to save the pointer to the frame-buffer
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamReadLatestPicture(uint8 ** ppPic);

/*********************************************************************//*!
 * @brief Register a callback function for image correction
 * 
 * ReadPicture calls this registered function at the very end. Depending
 * on the configured noise removal methods (OscClbSetupCalibrate) the
 * image is corrected in place.
 * 
 * Host: No effect
 * 
 * @param pCallback Pointer to the function to be called.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscCamRegisterCorrectionCallback(
		int (*pCallback)(
				uint8 *pImg,
				const uint16 lowX,
				const uint16 lowY,
				const uint16 width,
				const uint16 height));

/*********************************************************************//*!
 * @brief Setup the camera to scene perspective relation
 * 
 * This function allows to compensate for a non-upright camera and scene
 * relation. The received image is modified as follow:
 * no modification; horizontal mirror; vertical mirror; 180 degree rotation.
 * 
 * Host: No effect
 * 
 * @param perspective Identifier of the desired camera perspective.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamSetupPerspective(const enum EnOscCamPerspective perspective);

/*********************************************************************//*!
 * @brief Convert a string identifying a perspective to the corresponding
 * enum value.
 * 
 * @param str String identifier.
 * @param per Returned perspective identifier
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamPerspectiveStr2Enum( const char *str,
		enum EnOscCamPerspective *per);

/*********************************************************************//*!
 * @brief Convert perspective enum value to the corresponding string.
 * 
 * @param per Perspective identifier
 * @param str Returned string identifier.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamPerspectiveEnum2Str( const enum EnOscCamPerspective per,
		char *str);

/*********************************************************************//*!
 * @brief Host only: Set image sensor registers to standard configuration.
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCamPresetRegs();

/*********************************************************************//*!
 * @brief Returns an identifier describing the order of the bayer colors
 * of the row at the specified coordinates.
 * 
 * This does !NOT! depend on the configured perspective, since the sensor
 * automatically adjusts so the readout always starts at the same color. It
 * does however depend on the area of interest configured.
 * @see OscCamSetAreaOfInterest
 * 
 * Only on hardware with color sensor.
 * 
 * @param pBayerOrderFirstRow Returned identifier desribing the pixel color
 * order.
 * @param xPos X-Position of the row.
 * @param yPos Y-Position of the row.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscCamGetBayerOrder(enum EnBayerOrder *pBayerOrderFirstRow,
							const uint16 xPos,
							const uint16 yPos);
							

/*********************************************************************//*!
 * @brief Configure whether the LED_OUT pin from the image sensor is routed
 * to OUT2 or whether it can be used as a GPIO.
 * 
 * On the leanXcam hardware, the output OUT2 is a logical OR between the
 * corresponding DSP pin and the LED_OUT pin from the image sensor. This
 * means that if one wants to use it as a GPIO, the LED_OUT from the
 * sensor has to be disabled (done by this function) and in the
 * opposite case, where OUT2 should be assigned to LED_OUT, the DSP pin
 * has to be wired to zero (done by OscGpioConfigSensorLedOut).
 * 
 * ! Must also call OscGpioConfigSensorLedOut !
 * 
 * @see OscGpioConfigSensorLedOut
 * 
 * leanXcam only.
 * 
 * @param bSensorLedOut If TRUE, the sensor LED out is routed to OUT2 and
 * it cannot be used as GPIO anymore.
 * @param bInvert Invert the output of LED_OUT. Non-inverting means that
 * there is a rising-flank pulse at the plug.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscCamConfigSensorLedOut(bool bSensorLedOut, bool bInvert);


#endif /*CAM_PUB_H_*/
