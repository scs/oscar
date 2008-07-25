/*! @file gpio_pub.h
 * @brief Private GPIO module definition
 * 
 * Contains stuff that is too small to be put in a separate module.
 * 
 */
#ifndef GPIO_PUB_H_
#define GPIO_PUB_H_

#include "oscar_error.h"
#ifdef OSC_HOST
    #include "oscar_types_host.h"
    #include "oscar_host.h"
#else
    #include "oscar_types_target.h"
    #include "oscar_target.h"
#endif /* OSC_HOST */

#ifdef TARGET_TYPE_LEANXCAM
/*! @brief The GPIO pins that can be accessed from the application. 
 * 
 * These are assigned to the same identifier as the corresponding pflags pin
 * driving them. */
enum EnGpios
{
	GPIO_IN1 = 4,
	GPIO_IN2 = 7,
	GPIO_OUT1 = 2,
	GPIO_OUT2 = 6
};

/*! @brief Defines whether the image trigger of the camera sensor is activated
 * internally from the camera module or from externally over an input */
enum EnTriggerConfig
{
	/*! @brief Default: Internal triggering. */
	TRIGGER_INTERNAL,		
	/*! @brief Trigger from external input2 (leanXcam) */
	TRIGGER_EXTERNAL_IN2	
};

#endif /* TARGET_TYPE_LEANXCAM */
/*======================== API functions ===============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscGpioCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscGpioDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Set the state of an output
 * 
 * @param enGpio Identifier of the GPIO to be set.
 * @param bState Active state if true, inactive otherwise.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioWrite(enum EnGpios enGpio, bool bState);

/*********************************************************************//*!
 * @brief Read the state of an input
 * 
 * @param enGpio Identifier of the GPIO to be read.
 * @param pbState GPIO input state will be written to this pointer.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioRead(enum EnGpios enGpio, bool *pbState);

/*********************************************************************//*!
 * @brief Set the output/input polarity of a GPIO
 * 
 * The polarity in this sense is defined not as the polarity of the DSP
 * pin driving a GPIO, but rather as the polarity of the input/output of 
 * the board after opto-couplers etc.
 * 
 * @param enGpio Identifier of the GPIO to be configured.
 * @param bLowActive True if pin is to be low-active, false otherwise.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioSetupPolarity(enum EnGpios enGpio, bool bLowActive);

/*********************************************************************//*!
 * @brief Turn the onboard test-led on and off.
 * 
 * @param bOn TRUE turns LED on, FALSE turns it off.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioSetTestLed(bool bOn);

/*********************************************************************//*!
 * @brief Toggle the onboard test led.
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioToggleTestLed();

/*********************************************************************//*!
 * @brief Trigger a new image with the internal trigger.
 * 
 * Both Gpio module and camera module must be configured to use
 * the internal image trigger.
 * @see OscGpioConfigImageTrigger
 * @see OscCamConfigImageTrigger
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioTriggerImage();

#ifdef TARGET_TYPE_LEANXCAM
/*********************************************************************//*!
 * @brief Set the input for the image trigger of the camera sensor.
 * 
 * The default trigger input is set to internal triggering. Depending on the
 * hardware, external trigger inputs can be selected.
 * 
 * leanXcam only.
 * 
 * @param enConfig Trigger select.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioConfigImageTrigger(enum EnTriggerConfig enConfig);

/*********************************************************************//*!
 * @brief Configure whether the LED_OUT pin from the image sensor is routed
 * to OUT2 or whether it can be used as a GPIO.
 * 
 * On the leanXcam hardware, the output OUT2 is a logical OR between the
 * corresponding DSP pin and the LED_OUT pin from the image sensor. This 
 * means that if one wants to use it as a GPIO, the LED_OUT from the
 * sensor has to be disabled (done by OscCamConfigSensorLedOut) and in the 
 * opposite case, where OUT2 should be assigned to LED_OUT, the DSP pin 
 * has to be wired to zero (done by this function).
 * 
 * ! Must also call OscCamConfigSensorLedOut !
 * 
 * @see OscCamConfigSensorLedOut
 * 
 * leanXcam only.
 * 
 * @param bSensorLedOut If TRUE, the sensor LED out is routed to OUT2 and
 * it cannot be used as GPIO anymore.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioConfigSensorLedOut(bool bSensorLedOut);

/*********************************************************************//*!
 * @brief Set the color of the onboard test led.
 * 
 * The leanXcam features a 2-color test led with a green and a red 
 * channel that can be controlled separately. 
 * 
 * Smooth color transitions are not supported yet.
 * 
 * leanXcam only.
 * 
 * @param red Brightness of the red channel.
 * @param green Brighntess of the green channel.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGpioSetTestLedColor(uint8 red, uint8 green);

#endif /* TARGET_TYPE_LEANXCAM */

#endif /* GPIO_PUB_H_ */

