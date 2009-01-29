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

/*! @file vis.c
 * @brief Vision module implementation for target and host
 * 
 */

#include "vis_pub.h"
#include "../dma/dma_pub.h"
#include "vis_priv.h"
#include "oscar_intern.h"
#include <stdio.h>
#include <string.h>

/*! @brief The module singelton instance. */
struct OSC_VIS vis;

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY vis_deps[] = {
		{"log", OscLogCreate, OscLogDestroy},
		{"dma", OscDmaCreate, OscDmaDestroy}
};


OSC_ERR OscVisCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->vis.useCnt != 0)
	{
		pFw->vis.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	/* Load the module dependencies of this module. */
	err = OscLoadDependencies(pFw,
			vis_deps,
			sizeof(vis_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load dependencies! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	memset(&vis, 0, sizeof(struct OSC_VIS));
	
	/* Increment the use count */
	pFw->vis.hHandle = (void*)&vis;
	pFw->vis.useCnt++;
	
	return SUCCESS;
}

void OscVisDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->vis.useCnt--;
	if(pFw->vis.useCnt > 0)
	{
		return;
	}
	
	OscUnloadDependencies(pFw,
			vis_deps,
			sizeof(vis_deps)/sizeof(struct OSC_DEPENDENCY));
	
	memset(&vis, 0, sizeof(struct OSC_VIS));
}

