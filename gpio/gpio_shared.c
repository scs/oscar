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

/*! @file gpio_shared.c
 * @brief GPIO module code shared between host and target.
 * 
 */
#ifdef OSC_HOST
#include "oscar_types_host.h"
#endif

#ifdef OSC_TARGET
#include "oscar_types_target.h"
#endif

#include "oscar_intern.h"

#include "include/gpio.h"
#include "gpio.h"

#if defined(OSC_HOST) || defined(OSC_SIM)
#include "../srd/srd_pub.h"
#include "../swr/swr_pub.h"
#endif

/*! @brief The module singelton instance. */
struct OSC_GPIO gpio;

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY gpio_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
#if defined(OSC_HOST) || defined(OSC_SIM)
		,{"srd", OscSrdCreate, OscSrdDestroy}
		,{"swr", OscSwrCreate, OscSwrDestroy}
#endif /* OSC_HOST or OSC_SIM */
};
		
/*! @brief The length of the dependency array of this module. */
#define DEP_LEN (sizeof(gpio_deps)/sizeof(struct OSC_DEPENDENCY))

#ifdef TARGET_TYPE_LEANXCAM
/*! @brief Array with the default config of all the pins used on the
 * leanXcam.
 * All I/Os are configured to be high active at the actual plug. */
struct GPIO_PIN_CONFIG aryPinConfig[] = {
	/* {pinNr, defaultFlags, name, defaultState} */
	{PIN_IN1_N, (DIR_INPUT | POL_LOWACTIVE | FUN_GPIO), "IN1", FALSE},
	{PIN_IN2_N, (DIR_INPUT | POL_LOWACTIVE | FUN_GPIO), "IN2", FALSE},
	{PIN_OUT1_N, (DIR_OUTPUT | POL_LOWACTIVE | FUN_GPIO), "OUT1", FALSE},
	{PIN_OUT2_N, (DIR_OUTPUT | POL_LOWACTIVE | FUN_GPIO), "OUT2/DSP_LED_OUT", FALSE},
	{PIN_FN_EX_TRIGGER_N, (DIR_OUTPUT | POL_LOWACTIVE | FUN_RESERVED), "FN_EX_TRIGGER", FALSE},
	{PIN_TESTLED_R_N, (DIR_OUTPUT | POL_LOWACTIVE | FUN_RESERVED), "TESTLED_RED", FALSE},
	{PIN_TESTLED_G_N, (DIR_OUTPUT | POL_LOWACTIVE | FUN_RESERVED), "TESTLED_GREEN", FALSE},
	{PIN_EXPOSURE, (DIR_OUTPUT | POL_HIGHACTIVE | FUN_RESERVED), "EXPOSURE", FALSE}
};
#endif /* TARGET_TYPE_LEANXCAM */

#ifdef TARGET_TYPE_INDXCAM
/*! @brief Array with the default config of all the pins used on the
 * indXcam.
 * All I/Os are configured to be high active at the actual plug. */
struct GPIO_PIN_CONFIG aryPinConfig[] = {
	/* {pinNr, defaultFlags, name, defaultState} */
	{PIN_TESTLED_N, (DIR_OUTPUT | POL_LOWACTIVE | FUN_RESERVED), "TESTLED", FALSE}
};
#endif /* TARGET_TYPE_INDXCAM */

const uint16 nrOfPins = sizeof(aryPinConfig)/sizeof(aryPinConfig[0]);

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

#ifdef TARGET_TYPE_LEANXCAM

OSC_ERR OscGpioConfigSensorLedOut(bool bSensorLedOut)
{
	struct GPIO_PIN     *pPin = &gpio.pins[GPIO_OUT2];
	OSC_ERR             err = SUCCESS;
	
	if(bSensorLedOut)
	{
		/* Make sure to enable the LED_OUT signal in the CMOS
		 * sensor for correct operation (OscCamConfigSensorLedOut(TRUE, <Polarity>)). */
		/* OUT2 = !(PIN_OUT2_N | SENSOR_LED_OUT)
		 * => OUT2 = !SENSOR_LED_OUT with PIN_OUT2_N = 0 (before polarity inversion)*/
		err = OscGpioWrite(GPIO_OUT2, ((pPin->flags & POL_LOWACTIVE) != 0));
		/* Lock the pin from user access. */
		pPin->flags |= FUN_RESERVED;
	} else {
		/* Make sure to disable the LED_OUT signal in the CMOS sensor
		 *  for correct operation. (OscCamConfigSensorLedOut) */
		/* Clear the user lock access. */
		pPin->flags &= ~FUN_RESERVED;
	}
	return err;
}

#endif /* TARGET_TYPE_LEANXCAM */
