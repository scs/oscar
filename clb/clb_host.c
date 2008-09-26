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

/*! @file clb_host.c
 * @brief Calibration module implementation for host
 * 
 * No calibration is applied to host pictures.
 * 
 */
#include "oscar_intern.h"
#include "clb_pub.h"
#include "clb_priv.h"
#include <stdlib.h>

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY clb_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
};

struct OSC_CLB clb; /*!< @brief The clbera module singelton instance */


OSC_ERR OscClbCreate(void *hFw)
{
	struct OSC_FRAMEWORK    *pFw;
	OSC_ERR                 err;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->clb.useCnt != 0)
	{
		pFw->clb.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}

	/* Load the module clb_deps of this module. */
	err = OscLoadDependencies(pFw,
			clb_deps,
			sizeof(clb_deps)/sizeof(struct OSC_DEPENDENCY));
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load clb_deps! (%d)\n",
				__func__,
				err);
		return err;
	}
		
	memset(&clb, 0, sizeof(struct OSC_CLB));

	/* Increment the use count */
	pFw->clb.hHandle = (void*)&clb;
	pFw->clb.useCnt++;
	
	return SUCCESS;
}

void OscClbDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
			
	pFw = (struct OSC_FRAMEWORK *)hFw;
	
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->clb.useCnt--;
	if(pFw->clb.useCnt > 0)
	{
		return;
	}
		
	OscUnloadDependencies(pFw,
			clb_deps,
			sizeof(clb_deps)/sizeof(struct OSC_DEPENDENCY));
	
	memset(&clb, 0, sizeof(struct OSC_CLB));
}

OSC_ERR OscClbSetupCalibrate(
		enum EnOscClbCalibrateSlope calibSlope,
		bool bHotpixel)
{
	return SUCCESS;
}

OSC_ERR OscClbApplyCorrection( uint8 *pImg,
		const uint16 lowX, const uint16 lowY,
		const uint16 width, const uint16 height)
{
	return SUCCESS;
}

