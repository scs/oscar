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
struct OSC_GPIO gpio;       

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY gpio_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};
		
/*! @brief The length of the dependency array of this module. */
#define DEP_LEN (sizeof(gpio_deps)/sizeof(struct OSC_DEPENDENCY))

#ifdef TARGET_TYPE_LEANXCAM
/*! @brief Array with the default config of all the pins used on the indXcam. 
 * All I/Os are configured to be high active at the actual plug. */
struct GPIO_PIN_CONFIG aryPinConfig[] = {
	/* {pinNr, defaultFlags, name, defaultState} */
	{PIN_IN1_N, (DIR_INPUT | POL_LOWACTIVE | SEN_LEVEL | FUN_GPIO), "IN1", FALSE},
	{PIN_IN2_N, (DIR_INPUT | POL_LOWACTIVE | SEN_LEVEL | FUN_GPIO), "IN2", FALSE},
	{PIN_OUT1_N, (DIR_OUTPUT | POL_LOWACTIVE | SEN_LEVEL | FUN_GPIO), "OUT1", FALSE},
	{PIN_OUT2_N, (DIR_OUTPUT | POL_LOWACTIVE | SEN_LEVEL | FUN_GPIO), "OUT2/DSP_LED_OUT", FALSE},
	{PIN_EXPOSURE, (DIR_OUTPUT | POL_HIGHACTIVE | SEN_LEVEL | FUN_RESERVED), "EXPOSURE", FALSE},
	{PIN_FN_EX_TRIGGER_N, (DIR_OUTPUT | POL_LOWACTIVE | SEN_LEVEL  | FUN_RESERVED), "FN_EX_TRIGGER", FALSE},
	{PIN_TESTLED_R_N, (DIR_OUTPUT | POL_LOWACTIVE | SEN_LEVEL  | FUN_RESERVED), "TESTLED_RED", FALSE},
	{PIN_TESTLED_G_N, (DIR_OUTPUT | POL_LOWACTIVE | SEN_LEVEL  | FUN_RESERVED), "TESTLED_GREEN", FALSE}
};
#endif /* TARGET_TYPE_INDXCAM */

/*********************************************************************//*!
 * @brief Open the file descriptors to the pins and set the correct modes.
 * 
 * All the pins listed in aryPinConfig are opened and configured in accordance
 * with the specified default flags.
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscGpioInitPins();

OSC_ERR OscGpioCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->gpio.useCnt != 0)
    {
        pFw->gpio.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module dependencies of this module. */
    err = OscLoadDependencies(pFw, 
            gpio_deps, 
            DEP_LEN);
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load dependencies! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&gpio, 0, sizeof(struct OSC_GPIO));
    
    /* Setup our pins. */
    err = OscGpioInitPins();
    if(err != SUCCESS)
    {
    	OscLog(ERROR, "%s: Unable to intialize GPIO pins (%d)!\n",
    				__func__, err);
    	return -EDEVICE;
    }
    
    /* Increment the use count */
    pFw->gpio.hHandle = (void*)&gpio;
    pFw->gpio.useCnt++; 
    
    return SUCCESS;
}

void OscGpioDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
            
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->gpio.useCnt--;
    if(pFw->gpio.useCnt > 0)
    {
        return;
    }
    
    
    OscUnloadDependencies(pFw, 
            gpio_deps, 
            DEP_LEN);
    
    memset(&gpio, 0, sizeof(struct OSC_GPIO));
}

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

OSC_ERR OscGpioConfigSensorLedOut(bool bSensorLedOut)
{
	struct GPIO_PIN		*pPin = &gpio.pins[GPIO_OUT2];
	OSC_ERR				err = SUCCESS;
	
	if(bSensorLedOut)
	{
		/* Make sure to enable and invert the LED_OUT signal in the CMOS 
		 * sensor for correct operation (OscGpioConfigSensorLedOut). */
		/* OUT2 = ^(PIN_OUT2_N | SENSOR_LED_OUT)
		 * => OUT2 = ^SENSOR_LED_OUT with PIN_OUT2_N = 0 */
		err = OscGpioWrite(GPIO_OUT2, (pPin->flags & POL_LOWACTIVE));
		/* Lock the pin from user access. */
		pPin->flags |= FUN_RESERVED;
	} else {
		/* Make sure to disable the LED_OUT signal in the CMOS sensor
		 *  for correct operation. (OscGpioConfigSensorLedOut) */
		/* Clear the user lock access. */
		pPin->flags &= ~FUN_RESERVED;
	}
	return err;
}

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

static OSC_ERR OscGpioInitPins()
{
	uint16 		pin;
	int			pinNr;
	int 		ret;
	char		deviceNodePath[256];
	struct GPIO_PIN_CONFIG* pPinConfig;
	
	for(pin = 0; pin < sizeof(aryPinConfig)/sizeof(aryPinConfig[0]); pin++)
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
