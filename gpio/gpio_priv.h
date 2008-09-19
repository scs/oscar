/*! @file gpio_priv.h
 * @brief Private GPIO module definition
 * 
 */
#ifndef GPIO_PRIV_H_
#define GPIO_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef OSC_HOST
    #include <oscar_types_host.h>
#else
    #include <oscar_types_target.h>
#endif /* OSC_HOST */

#include <log/log_pub.h>

/*! @brief How many GPIOs exist in hardware. */
#define NR_OF_DSP_GPIOS 48
/*! @brief The path to the device nodes of the driver (simple-gpio) for the GPIO
 * pins. The whole patch is built by suffixing with the pin number. */
#define PIN_DEVICE_NODE_PREFIX "/dev/gpio"


/*************************** GPIO flags *********************/

/*! @brief The bit mask for the direction field in the flags register. */
#define DIR_MASK 		0x1
/*! @brief Pin is input. */
#define DIR_INPUT  		0x0
/*! @brief Pin is output. */
#define DIR_OUTPUT 		0x1

/*! @brief The bit mask for the polarity field in the flags register. */
#define POL_MASK		0x2
/*! @brief Pin is high active or sensitive to rising flank is edge sensitive. */
#define POL_HIGHACTIVE 	0x0
/*! @brief Pin is low active or sensitive to falling flank is edge sensitive. */
#define POL_LOWACTIVE 	0x2

/*! @brief The bit mask for the function field in the flags register. */
#define FUN_MASK		0x10
/*! @brief Pin is no accessible directly from application, but rather
 * assigned to a specific internal function. */
#define FUN_RESERVED	0x10
/*! @brief Pin can be accessed from application as a GPIO. */
#define FUN_GPIO		0x0

struct GPIO_PIN_CONFIG
{
	/*! @brief The simple-gpio pin number of this pin. */
	int pinNr;
	/*! @brief The default flags used for this pin (Input/Output etc.). */
	int defaultFlags;
	/*! @brief The name of this pin. */
	char name[32];
	/*! @brief The default state after initalization (output pins only). 
	 * "0", "1".*/
	bool defaultState;
};

struct GPIO_PIN
{
	/*! @brief File descriptor to the char device driver of this pin. */
	int 					fd;
	/*! @brief The default config for this GPIO pin. This is used */
	struct GPIO_PIN_CONFIG 	*pDefConfig;	
	/*! @brief The currently configured flags. */
	int						flags;
	/*! @brief Current state of the pin (on or off). */
	bool					bState;
};

#ifdef TARGET_TYPE_LEANXCAM
	/* Pin mapping */
	#define PIN_IN1_N 			GPIO_IN1
	#define PIN_IN2_N 			GPIO_IN2
	#define PIN_OUT1_N 			GPIO_OUT1
	#define PIN_OUT2_N 			GPIO_OUT2
	#define PIN_EXPOSURE 		3
	#define PIN_FN_EX_TRIGGER_N 26
	#define PIN_TESTLED_R_N 	28
	#define PIN_TESTLED_G_N 	27
		
#endif /* TARGET_TYPE_INDXCAM */

#ifdef TARGET_TYPE_INDXCAM
	/* Pin mapping */
	#define PIN_TESTLED_N		27
	#define PIN_EXPOSURE		3
#endif /* TARGET_TYPE_INDXCAM */

#if defined(OSC_HOST) || defined(OSC_SIM)
	/*! @brief True if pins reserved for internal use are to be logged
	 * as well. */
	/*#define OSC_GPIO_LOG_RESERVED_PINS*/
	/*! @brief File where the output signals will be written to. */
	#define OSC_GPIO_WRITER_FILE "gpio_out.txt"
	/*! @brief File from where the input stimuli are read. */
	#define OSC_GPIO_READER_FILE "gpio_in.txt"
#endif /* OSC_HOST or OSC_SIM */

/*! @brief The object struct of the GPIO module */
struct OSC_GPIO
{
	/*! @brief File descriptors of all the pins used */
	struct GPIO_PIN		pins[NR_OF_DSP_GPIOS];
#ifdef TARGET_TYPE_LEANXCAM
  /*! @brief Whether external or internal triggering is currently configured. */
  enum EnTriggerConfig  enTriggerConfig;
#endif /* TARGET_TYPE_LEANXCAM */
#if defined(OSC_HOST) || OSC_SIM
	/*! @brief Handle to stimuli writer. */
	void 				*hWriter;
	/*! @brief Handle to the stimuli reader. */
	void				*hReader;
	/*! @brief Writer handles to output signals. */
	void				*phSignalOut[NR_OF_DSP_GPIOS];
	/*! @brief Reader handles to input signals. */
	void				*phSignalIn[NR_OF_DSP_GPIOS];
#endif /* OSC_HOST or OSC_SIM */
};

/*======================= Private methods ==============================*/
/*********************************************************************//*!
 * @brief Open the file descriptors to the pins and set the correct modes.
 * 
 * All the pins listed in aryPinConfig are opened and configured in 
 * accordance with the specified default flags.
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscGpioInitPins();

#if defined(OSC_HOST) || defined(OSC_SIM)

/*********************************************************************//*!
 * @brief Callback from the stimuli reader to allow to query the updated
 * signal values.
 *//*********************************************************************/
void OscGpioReaderCallback();

#endif /* OSC_HOST or OSC_SIM */

#endif /*GPIO_PRIV_H_*/
