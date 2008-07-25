/*! @file gpio_target.c
 * @brief GPIO module implementation for host 
 * 
 */

#include "oscar_types_target.h"

#include "gpio_pub.h"
#include "gpio_priv.h"
#include "oscar_intern.h"
#include "pflags.h"

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
                                           
OSC_ERR OscGpioSetupPolarity(enum EnGpios enGpio, bool bLowActive)
{
	struct GPIO_PIN 	*pPin = &gpio.pins[enGpio];
	bool 				bPinLowActive;
	int 				ret;
	
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
	
	/* Write the information to the driver. */
	ret = ioctl(pPin->fd, 
	   		SET_FIO_POLAR,
	   		((pPin->flags & POL_MASK) && ACTIVELOW_FALLINGEDGE));
	if(unlikely(ret != 0))
    {
        OscLog(ERROR, "%s: Unable to set polarity for pin %s (%s)!\n", 
        __func__, pPin->pDefConfig->name, strerror(errno));
        return -EDEVICE;
    }
	return SUCCESS;
}

OSC_ERR OscGpioWrite(enum EnGpios enGpio, bool bState)
{
	struct GPIO_PIN 	*pPin = &gpio.pins[enGpio];
	int 				ret;
	
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
	
	/* Write to the driver. */
	if(bState)
	{
		ret = write(pPin->fd, "1", sizeof("1"));
	} else {
		ret = write(pPin->fd, "0", sizeof("0"));
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
	struct GPIO_PIN 	*pPin = &gpio.pins[enGpio];
	char				buf[2];
	int 				ret;
	
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
	ret = read(pPin->fd, buf, 2);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR, "%s: Error reading from pin %s (%s)\n",
				__func__, pPin->pDefConfig->name, strerror(errno));
		return -EDEVICE;
	}
	
	if(buf[0])
	{
		*pbState = TRUE;
	} else {
		*pbState = FALSE;
	}		

	return SUCCESS;	
}

#ifdef TARGET_TYPE_INDXCAM
OSC_ERR OscGpioSetTestLed(bool bOn)
{
	int ret;
	struct GPIO_PIN		*pPin = &gpio.pins[PIN_TESTLED_N];
	
	ret = write(pPin->fd, (bOn ? "1" : "0"), 2);
	
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
	struct GPIO_PIN		*pPin = &gpio.pins[PIN_TESTLED_N];
	
	ret = write(pPin->fd, "T", 2);
	
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
	struct GPIO_PIN		*pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN		*pGreen = &gpio.pins[PIN_TESTLED_G_N];
	
	ret = write(pRed->fd, (bOn ? "1" : "0"), 2);
	ret |= write(pGreen->fd, (bOn ? "1" : "0"), 2);
	
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
	struct GPIO_PIN		*pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN		*pGreen = &gpio.pins[PIN_TESTLED_G_N];
	
	ret = write(pRed->fd, "T", 2);
	ret |= write(pGreen->fd, "T", 2);
	
	if(ret < 0)
	{
		OscLog(ERROR, "%s: Unable to toggle LED (%s)\n",
				__func__, strerror(errno));
		return -EDEVICE;
	}
	return SUCCESS;
}
#endif /* TARGET_TYPE_LEANXCAM */

OSC_ERR OscGpioTriggerImage()
{
	int ret;
	struct GPIO_PIN		*pPin = &gpio.pins[PIN_EXPOSURE];
	
	/* Create a pulse on the Exposure pin of the image sensor. Sensor
	 * is triggered by rising flank, so high flank should not have to
	 * be too broad.*/
	ret = write(pPin->fd, "1", 2); /* Rising flank */
	if(unlikely(ret < 0))
	{
		goto exit_fail;
	}

	ret = write(pPin->fd, "0", 2); /* Falling flank */
	if(unlikely(ret < 0))
	{
		goto exit_fail;
	}
	
	return SUCCESS;
exit_fail:
	OscLog(ERROR, "%s: Unable to create trigger pulse (%s)\n",
			__func__, strerror(errno));
	return -EDEVICE;
}

OSC_ERR OscGpioInitPins()
{
	uint16 		pin;
	int			pinNr;
	int 		ret;
	char		deviceNodePath[256];
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
	    ret = ioctl(gpio.pins[pinNr].fd, 
	    			SET_FIO_DIR, 
	    			((pPinConfig->defaultFlags & DIR_MASK) && OUTPUT));
	    ret |= ioctl(gpio.pins[pinNr].fd, 
	    			SET_FIO_INEN,
	    			!((pPinConfig->defaultFlags & DIR_MASK) && INPUT_ENABLE));
	    			
	    /* Polarity */
	    ret |= ioctl(gpio.pins[pinNr].fd, 
	    			SET_FIO_POLAR,
	    			((pPinConfig->defaultFlags & POL_MASK) && ACTIVELOW_FALLINGEDGE));
	    			
	    /* Edge sensitivity */
	    ret |= ioctl(gpio.pins[pinNr].fd, 
	    			SET_FIO_EDGE,
	    			((pPinConfig->defaultFlags & SEN_MASK) && EDGE));
	    			
	    /* Edge multiciplity */
	    ret |= ioctl(gpio.pins[pinNr].fd, 
	    			SET_FIO_BOTH,
	    			((pPinConfig->defaultFlags & EDGE_MASK) && BOTHEDGES));
	    			
	    if(unlikely(ret != 0))
	    {
	        OscLog(ERROR, "%s: Unable to set flags for pin %s (%d)!\n", 
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
    			ret = write(gpio.pins[pinNr].fd, "1", sizeof("1"));
    		} else {
    			ret = write(gpio.pins[pinNr].fd, "0", sizeof("0"));
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

#ifdef TARGET_TYPE_LEANXCAM

OSC_ERR OscGpioSetTestLedColor(uint8 red, uint8 green)
{
	int ret;
	struct GPIO_PIN		*pRed = &gpio.pins[PIN_TESTLED_R_N];
	struct GPIO_PIN		*pGreen = &gpio.pins[PIN_TESTLED_G_N];
	
	/* Color transitions currently not supported. */
	ret = write(pRed->fd, (red ? "1" : "0"), 2);
	ret |= write(pGreen->fd, (green ? "1" : "0"), 2);
	
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
	struct GPIO_PIN		*pPin = &gpio.pins[PIN_FN_EX_TRIGGER_N];
	int 				ret;
	
	if(enConfig == TRIGGER_INTERNAL)
	{
		ret = write(pPin->fd, "0", sizeof("0"));
	} else if(enConfig == TRIGGER_EXTERNAL_IN2){
		ret = write(pPin->fd, "1", sizeof("1"));
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
	
	return SUCCESS;
}

#endif /* TARGET_TYPE_LEANXCAM */
