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

/*! @file gpio_host.c
 * @brief GPIO module implementation for host
 * 
 */

#include "oscar_types_host.h"

#include "oscar_intern.h"
#include "include/gpio.h"
#include "gpio.h"
#include <time.h>

#include "../include/srd.h"
#include "../include/swr.h"

/*! @brief The module singelton instance. */
extern struct OSC_GPIO gpio;
extern struct GPIO_PIN_CONFIG aryPinConfig[];
extern const uint16 nrOfPins;

OSC_ERR OscGpioSetupPolarity(enum EnGpios enGpio, bool bLowActive)
{
	struct GPIO_PIN     *pPin = &gpio.pins[enGpio];
	bool                bPinLowActive;
	
	/* Since the default polarity setup of pin at the plug is always high
	 * active, we can consult the default config to deduce the correct setup
	 * of the DSP pin for the desired innput/output polarity. */
	if(pPin->pDefConfig->defaultFlags & POL_MASK)
	{
		bPinLowActive = !bLowActive;
	} else {
		bPinLowActive = bLowActive;
	}
	
	pPin->flags = (pPin->flags & ~POL_MASK) | bPinLowActive;
	
	return SUCCESS;
}

OSC_ERR OscGpioWrite(enum EnGpios enGpio, bool bState)
{
	struct GPIO_PIN     *pPin = &gpio.pins[enGpio];
	
	if(unlikely(!pPin->fd))
	{
		OscLog(ERROR, "%s: No file descriptor for pin %d found. "
				"This probably "
				"means that this GPIO is not available on your "
				"hardware platform.", __func__, enGpio);
		return -EINVALID_PARAMETER;
	}
	
	/* Sanity check */
	if(unlikely(!(pPin->flags & DIR_OUTPUT)))
	{
		OscLog(ERROR, "%s: Cannot write to an input (%s)\n",
				__func__, pPin->pDefConfig->name);
		return -EINVALID_PARAMETER;
	}
	
	if(pPin->flags & FUN_RESERVED)
	{
		OscLog(WARN, "%s: Pin %s is reserved internally and can not "
					"currently be accessed by the application!\n",
					__func__, pPin->pDefConfig->name);
		return -EDEVICE_BUSY;
	}
	
	pPin->bState = bState;
	
	/* Write the change to the output log-file after applying polarity. */
	if(pPin->flags & POL_LOWACTIVE)
	{
		bState = !bState;
	}
	
	return OscSwrUpdateSignal(gpio.phSignalOut[enGpio], &bState);
}

OSC_ERR OscGpioRead(enum EnGpios enGpio, bool *pbState)
{
	struct GPIO_PIN     *pPin = &gpio.pins[enGpio];
	
	if(unlikely(!pPin->fd))
	{
		OscLog(ERROR, "%s: No file descriptor for pin %d found. "
				"This probably "
				"means that this GPIO is not available on your "
				"hardware platform.", __func__, enGpio);
		return -EINVALID_PARAMETER;
	}
	
	if(pPin->flags & FUN_RESERVED)
	{
		OscLog(WARN, "%s: Pin %s is reserved internally and can not "
					"currently be accessed by the application!\n",
					__func__, pPin->pDefConfig->name);
		return -EDEVICE_BUSY;
	}
	
	*pbState = pPin->bState;

	return SUCCESS;
}

#ifdef TARGET_TYPE_INDXCAM
OSC_ERR OscGpioSetTestLed(bool bOn)
{
	OSC_ERR             err = SUCCESS;
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_TESTLED_N];
	
	pPin->bState = bOn;

#ifdef OSC_GPIO_LOG_RESERVED_PINS
		bOn = (pPin->flags & POL_LOWACTIVE) ? !pPin->bState : pPin->bState;
		err = OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_N], &bOn);
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	return err;
}

OSC_ERR OscGpioToggleTestLed()
{
	OSC_ERR             err = SUCCESS;
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_TESTLED_N];
#ifdef OSC_GPIO_LOG_RESERVED_PINS
	bool                bOn;
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	
	pPin->bState = !pPin->bState;

#ifdef OSC_GPIO_LOG_RESERVED_PINS
	bOn = (pPin->flags & POL_LOWACTIVE) ? !pPin->bState : pPin->bState;
	err = OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_N], &bOn);
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	return err;
}
#endif /* TARGET_TYPE_INDXCAM */

#ifdef TARGET_TYPE_LEANXCAM
OSC_ERR OscGpioSetTestLed(bool bOn)
{
	OSC_ERR             err = SUCCESS;
	struct GPIO_PIN     *pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN     *pGreen = &gpio.pins[PIN_TESTLED_G_N];
	
	pRed->bState = pGreen->bState = bOn;

#ifdef OSC_GPIO_LOG_RESERVED_PINS
		bOn = (pRed->flags & POL_LOWACTIVE) ? !pRed->bState : pRed->bState;
		err = OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_R_N], &bOn);
		bOn = (pGreen->flags & POL_LOWACTIVE) ? !pGreen->bState : pGreen->bState;
		err |= OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_G_N], &bOn);
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	return err;
}

OSC_ERR OscGpioToggleTestLed()
{
	OSC_ERR             err = SUCCESS;
	struct GPIO_PIN     *pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN     *pGreen = &gpio.pins[PIN_TESTLED_G_N];
#ifdef OSC_GPIO_LOG_RESERVED_PINS
	bool                bOn;
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	
	pRed->bState = !pRed->bState;
	pGreen->bState = !pGreen->bState;

#ifdef OSC_GPIO_LOG_RESERVED_PINS
	bOn = (pRed->flags & POL_LOWACTIVE) ? !pRed->bState : pRed->bState;
	err = OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_R_N], &bOn);
	bOn = (pGreen->flags & POL_LOWACTIVE) ? !pGreen->bState : pGreen->bState;
	err |= OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_G_N], &bOn);
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	return err;
}
#endif /* TARGET_TYPE_LEANXCAM */

OSC_ERR OscGpioTriggerImage()
{
	/* Host does not need to be triggered. */
	return SUCCESS;
}

#ifdef TARGET_TYPE_INDXCAM
OSC_ERR OscGpioSetTestLedColor(uint8 red, uint8 green)
{
	return SUCCESS;
}
#endif /* TARGET_TYPE_INDXCAM */

#ifdef TARGET_TYPE_LEANXCAM
OSC_ERR OscGpioSetTestLedColor(uint8 red, uint8 green)
{
	OSC_ERR             err = SUCCESS;
	struct GPIO_PIN     *pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN     *pGreen = &gpio.pins[PIN_TESTLED_G_N];
#ifdef OSC_GPIO_LOG_RESERVED_PINS
	bool                bOn;
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	
	/* Color transitions currently not supported. */
	pRed->bState = red ? TRUE : FALSE;
	pGreen->bState = green ? TRUE : FALSE;

#ifdef OSC_GPIO_LOG_RESERVED_PINS
	bOn = (pRed->flags & POL_LOWACTIVE) ? !pRed->bState : pRed->bState;
	err = OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_R_N], &bOn);
	bOn = (pGreen->flags & POL_LOWACTIVE) ? !pGreen->bState : pGreen->bState;
	err |= OscSwrUpdateSignal(gpio.phSignalOut[PIN_TESTLED_G_N], &bOn);
#endif /* OSC_GPIO_LOG_RESERVED_PINS */
	
	return err;
}

OSC_ERR OscGpioConfigImageTrigger(enum EnTriggerConfig enConfig)
{
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_FN_EX_TRIGGER_N];
	
	/* This function does not really have an effect on host. */
	if(enConfig == TRIGGER_INTERNAL)
	{
		pPin->bState = FALSE;
	} else if(enConfig == TRIGGER_EXTERNAL_IN2){
		pPin->bState = TRUE;
	} else {
		OscLog(ERROR, "%s: Invalid trigger config for this hardware (%d)!\n",
				__func__, enConfig);
		return -EINVALID_PARAMETER;
	}
	
	return SUCCESS;
}
#endif /*TARGET_TYPE_LEANXCAM*/

OSC_ERR OscGpioInitPins()
{
	uint16      pin;
	int         pinNr;
	struct GPIO_PIN_CONFIG* pPinConfig;
	uint32      defaultValue;
	OSC_ERR     err;
	bool        bReaderCreated = FALSE;
	bool        bWriterCreated = FALSE;

	for(pin = 0; pin < nrOfPins; pin++)
	{
		pPinConfig = &aryPinConfig[pin];

		/* Sanity checks. */
		pinNr = pPinConfig->pinNr;
		if(pinNr < 0 || pinNr >= NR_OF_DSP_GPIOS)
		{
			OscLog(ERROR,
					"%s: Fatal! Invalid pin number for %s configured! (%d)\n",
					__func__, pPinConfig->name, pinNr);
			return -EDEVICE;
		}

		/***************** Register the pin signals. ******************/
		if(pPinConfig->defaultFlags & DIR_OUTPUT)
		{
#ifndef OSC_GPIO_LOG_RESERVED_PINS
			if((pPinConfig->defaultFlags & FUN_RESERVED))
			{
				err = SUCCESS;
				goto skip_logging;
			}
#endif
			if(!bWriterCreated)
			{
				/* Create a writer for the outputs */
				err = OscSwrCreateWriter(
						&gpio.hWriter, OSC_GPIO_WRITER_FILE,
						TRUE, TRUE); /*report property: time, cyclic */
				if(err != SUCCESS)
				{
					OscLog(ERROR, "%s: Unable to create stimuli writer! (%d)\n",
							__func__, err);
					return err;
				}
				bWriterCreated = TRUE;
			}
			defaultValue = pPinConfig->defaultState ? 1 : 0;
			err = OscSwrRegisterSignal(&gpio.phSignalOut[pinNr],
					gpio.hWriter,
					pPinConfig->name,
					SWR_INTEGER,
					&defaultValue,
					"%d");
		} else {
			if(!bReaderCreated)
			{
				/* Create a stimuli reader for the inputs */
				err = OscSrdCreateReader( OSC_GPIO_READER_FILE,
						&OscGpioReaderCallback,
						&gpio.hReader);
				if(err != SUCCESS)
				{
					OscLog(ERROR, "%s: Unable to create stimuli reader! (%d)\n",
							__func__, err);
					return err;
				}
				bReaderCreated = TRUE;
			}
			err = OscSrdRegisterSignal( gpio.hReader,
					pPinConfig->name,
					&gpio.phSignalIn[pinNr]);
		}
skip_logging:
		if(unlikely(err != SUCCESS))
		{
			OscLog(ERROR, "%s: Error registering signal %s! (%d)\n",
					__func__, pPinConfig->name, err);
			return err;
		}

		/* Save a pointer to the pin configuration. */
		gpio.pins[pinNr].pDefConfig = pPinConfig;
		/* Save the default flags as the currently active flags. */
		gpio.pins[pinNr].flags = pPinConfig->defaultFlags;
		gpio.pins[pinNr].bState = pPinConfig->defaultState;
	}

	return SUCCESS;
}

void OscGpioReaderCallback()
{
	void    *phSignalIn;
	bool    bPolState;
	uint16  pinNr;
	
	for(pinNr = 0; pinNr < NR_OF_DSP_GPIOS; pinNr++)
	{
		if(gpio.phSignalIn[pinNr])
		{
			phSignalIn = gpio.phSignalIn[pinNr];
			
			/* Get the new signal value. */
			OscSrdGetUpdateSignal(phSignalIn,
					&bPolState);
			/* Apply polarity. */
			if(gpio.pins[pinNr].flags & POL_LOWACTIVE)
			{
				gpio.pins[pinNr].bState = !bPolState;
			} else {
				gpio.pins[pinNr].bState = bPolState;
			}
			
		}
	}
}
