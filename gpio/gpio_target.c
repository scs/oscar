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

/*! @file gpio_target.c
 * @brief GPIO module implementation for host
 * 
 */

#include "oscar_types_target.h"
#include "oscar_intern.h"

#include "gpio_pub.h"
#include "gpio_priv.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>

/*! @brief The module singelton instance. */
extern struct OSC_GPIO gpio;
extern struct GPIO_PIN_CONFIG aryPinConfig[];
extern const uint16 nrOfPins;
								
static const char on = '1';
static const char off = '0';
static const char toggle = 'T';
											
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
	bool                bLowPolarity;
	int                 ret;
	
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
	
	/* Write to the driver. XOR with the pin polarity to get the correct value to write.*/
	bLowPolarity = ((pPin->flags & POL_LOWACTIVE) != 0);
	if(bState ^ bLowPolarity)
	{
		ret = write(pPin->fd, &on, 1);
	} else {
		ret = write(pPin->fd, &off, 1);
	}
				
	if(unlikely(ret < 0))
	{
		OscLog(ERROR, "%s: Error writing to pin %s (%s)\n",
				__func__, pPin->pDefConfig->name, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}

OSC_ERR OscGpioRead(enum EnGpios enGpio, bool *pbState)
{
	struct GPIO_PIN     *pPin = &gpio.pins[enGpio];
	bool                bLowPolarity;
	char                buf;
	int                 ret;
	
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
	
	/* Read from the driver. */
	ret = read(pPin->fd, &buf, 1);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR, "%s: Error reading from pin %s (%s)\n",
				__func__, pPin->pDefConfig->name, strerror(errno));
		return -EDEVICE;
	}
	
	/* Get the current pin polarity and calculate the resulting value*/
	bLowPolarity = ((pPin->flags & POL_LOWACTIVE) != 0);
	if(buf != off)
	{
		*pbState = !bLowPolarity;
	} else {
		*pbState = bLowPolarity;
	}

	return SUCCESS;
}

#ifdef TARGET_TYPE_INDXCAM
OSC_ERR OscGpioSetTestLed(bool bOn)
{
	int ret;
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_TESTLED_N];
	
	ret = write(pPin->fd, (bOn ? &on : &off), 1);
	
	if(ret < 0)
	{
		OscLog(ERROR, "%s: Unable to set LED (%s)\n",
				__func__, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}

OSC_ERR OscGpioToggleTestLed()
{
	int ret;
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_TESTLED_N];
	
	ret = write(pPin->fd, &toggle, 1);
	
	if(ret < 0)
	{
		OscLog(ERROR, "%s: Unable to toggle LED (%s)\n",
				__func__, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}
#endif /* TARGET_TYPE_INDXCAM */

#ifdef TARGET_TYPE_LEANXCAM
OSC_ERR OscGpioSetTestLed(bool bOn)
{
	int ret;
	struct GPIO_PIN     *pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN     *pGreen = &gpio.pins[PIN_TESTLED_G_N];
	bool bRedPolarity, bGreenPolarity;
	
	bRedPolarity = ((pRed->flags & POL_LOWACTIVE) != 0);
	bGreenPolarity = ((pGreen->flags & POL_LOWACTIVE) != 0);
	
	ret = write(pRed->fd, ((bOn ^ bRedPolarity) ? &on : &off), 1);
	ret |= write(pGreen->fd, ((bOn ^ bGreenPolarity) ? &on : &off), 1);
	
	if(ret < 0)
	{
		OscLog(ERROR, "%s: Unable to set LED (%s)\n",
				__func__, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}

OSC_ERR OscGpioToggleTestLed()
{
	int ret;
	struct GPIO_PIN     *pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN     *pGreen = &gpio.pins[PIN_TESTLED_G_N];
	
	ret = write(pRed->fd, &toggle, 1);
	ret |= write(pGreen->fd, &toggle, 1);
	
	if(ret < 0)
	{
		OscLog(ERROR, "%s: Unable to toggle LED (%s)\n",
				__func__, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}
#endif /* TARGET_TYPE_LEANXCAM */

#ifdef TARGET_TYPE_INDXCAM

OSC_ERR OscGpioTriggerImage()
{
	/* Triggering is done over the CPLD and not over the GPIOs for indXcam. */
	return -EUNSUPPORTED;
}

#else /* TARGET_TYPE_INDXCAM */

OSC_ERR OscGpioTriggerImage()
{
	int ret;
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_EXPOSURE];
	
#ifdef TARGET_TYPE_LEANXCAM
	if(gpio.enTriggerConfig != TRIGGER_INTERNAL)
		{
		/* Don't allow internal triggering if external triggering is configured. */
		return -EDEVICE_BUSY;
		}
#endif

	/* Create a pulse on the Exposure pin of the image sensor. Sensor
	 * is triggered by rising flank, so high time should not have to
	 * be too broad.*/
	if((pPin->flags & POL_LOWACTIVE) != 0)
	{
		/* Lowactive */
		ret = write(pPin->fd, &off, 1); /* Rising flank */
	} else {
		ret = write(pPin->fd, &on, 1); /* Rising flank */
	}
	if(unlikely(ret < 0))
	{
		goto exit_fail;
	}

	/* Wait until the pin has been set. */
	asm("ssync;\n");

	if((pPin->flags & POL_LOWACTIVE) != 0)
	{
		/* Lowactive */
		ret = write(pPin->fd, &on, 1); /* Falling flank */
	} else {
		ret = write(pPin->fd, &off, 1); /* Falling flank */
	}
	
	if(unlikely(ret < 0))
	{
		goto exit_fail;
	}
	
	/* Wait until the pin has been set. */
	asm("ssync;\n");
	
	return SUCCESS;
exit_fail:
	OscLog(ERROR, "%s: Unable to create trigger pulse (%s)\n",
			__func__, strerror(errno));
	return -EDEVICE;
}

#endif /* TARGET_TYPE_INDXCAM*/

OSC_ERR OscGpioInitPins()
{
	uint16      pin;
	int         pinNr;
	char        dir;
	int         ret;
	char        deviceNodePath[256];
	struct GPIO_PIN_CONFIG* pPinConfig;
	
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
		
		/***************** Open the pin device nodes. ******************/
		sprintf(deviceNodePath, PIN_DEVICE_NODE_PREFIX "%d", pinNr);
		gpio.pins[pinNr].fd = open(deviceNodePath, O_RDWR, 0);
		if(gpio.pins[pinNr].fd < 0)
		{
			OscLog(ERROR, "%s: Unable to open device node for pin %d (%s)! (%s)\n",
					__func__,
					pinNr,
					pPinConfig->name,
					strerror(errno));
			return -EDEVICE;
		}
		
		/*************** Set the pin flags. ***************************/
		/* Direction */
		dir = (pPinConfig->defaultFlags & DIR_MASK) ? 'O' : 'I';
		ret = write(gpio.pins[pinNr].fd, &dir, 1);
					
		if(unlikely(ret < 0))
		{
			OscLog(ERROR, "%s: Unable to set direction for pin %s (%d)!\n",
			__func__, pPinConfig->name, pinNr);
			close(gpio.pins[pinNr].fd);
			return -EDEVICE;
		}
	
		/******** Set the default initialization state of the pin ********/
		if(pPinConfig->defaultFlags & DIR_MASK)
		{
			/* Output pin */
			if(pPinConfig->defaultState)
			{
				if(pPinConfig->defaultFlags & POL_LOWACTIVE)
				{
					ret = write(gpio.pins[pinNr].fd, &off, 1);
				} else {
					ret = write(gpio.pins[pinNr].fd, &on, 1);
				}
			} else {
				if(pPinConfig->defaultFlags & POL_LOWACTIVE)
				{
					ret = write(gpio.pins[pinNr].fd, &on, 1);
				} else {
					ret = write(gpio.pins[pinNr].fd, &off, 1);
				}
			}
			if(unlikely(ret < 0))
			{
				OscLog(ERROR, "%s: Error writing to pin %s (%s)\n",
						__func__, pPinConfig->name, strerror(errno));
				return -EDEVICE;
			}
		}
	
		/* Save a pointer to the pin configuration. */
		gpio.pins[pinNr].pDefConfig = pPinConfig;
		/* Save the default flags as the currently active flags. */
		gpio.pins[pinNr].flags = pPinConfig->defaultFlags;
	}
	
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
	int ret;
	struct GPIO_PIN     *pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN     *pGreen = &gpio.pins[PIN_TESTLED_G_N];
	bool				bRedOn, bGreenOn;
	bool bRedPolarity, bGreenPolarity;
	
	bRedPolarity = ((pRed->flags & POL_LOWACTIVE) != 0);
	bGreenPolarity = ((pGreen->flags & POL_LOWACTIVE) != 0);
	
	bRedOn = (red ? TRUE : FALSE);
	bGreenOn = (green ? TRUE : FALSE);
	
	/* Color transitions currently not supported. */
	ret = write(pRed->fd, ((bRedOn ^ bRedPolarity) ? &on : &off), 1);
	ret |= write(pGreen->fd, ((bGreenOn ^ bGreenPolarity) ? &on : &off), 1);
	
	if(ret < 0)
	{
		OscLog(ERROR, "%s: Unable to set LED color (%s)\n",
				__func__, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}

OSC_ERR OscGpioConfigImageTrigger(enum EnTriggerConfig enConfig)
{
	struct GPIO_PIN     *pPin = &gpio.pins[PIN_FN_EX_TRIGGER_N];
	int                 ret;
	
	if(enConfig == TRIGGER_INTERNAL)
	{
		/* Pin is lowactive. */
		ret = write(pPin->fd, &on, 1);
	} else if(enConfig == TRIGGER_EXTERNAL_IN2)
	{
		/* Pin is lowactive. */
		ret = write(pPin->fd, &off, 1);
	} else {
		OscLog(ERROR, "%s: Invalid trigger config for this hardware (%d)!\n",
				__func__, enConfig);
		return -EINVALID_PARAMETER;
	}
	
	if(unlikely(ret < 0))
	{
		OscLog(ERROR, "%s: Error writing to pin %s (%s)\n",
				__func__, pPin->pDefConfig->name, strerror(errno));
		return -EDEVICE;
	}
	
	gpio.enTriggerConfig = enConfig;
	return SUCCESS;
}

#endif /* TARGET_TYPE_LEANXCAM */
