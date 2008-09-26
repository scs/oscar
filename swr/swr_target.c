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

/*! @file swr_target.c
 * @brief Stimuli writer module implementation for target.
 * 
	************************************************************************/

#include "oscar_types_target.h"

#include "swr_pub.h"
#include "swr_priv.h"
#include "oscar_intern.h"

struct OSC_SWR swr;     /*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY swr_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscSwrCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->swr.useCnt != 0)
	{
		pFw->swr.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	/* Load the module swr_deps of this module. */
	err = OscLoadDependencies(pFw,
			swr_deps,
			sizeof(swr_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load swr_deps! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	memset(&swr, 0, sizeof(struct OSC_SWR));
	
	/* Increment the use count */
	pFw->swr.hHandle = (void*)&swr;
	pFw->swr.useCnt++;
	
	return SUCCESS;
}

void OscSwrDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->swr.useCnt--;
	if(pFw->swr.useCnt > 0)
	{
		return;
	}
	
	OscUnloadDependencies(pFw,
			swr_deps,
			sizeof(swr_deps)/sizeof(struct OSC_DEPENDENCY));
	
	memset(&swr, 0, sizeof(struct OSC_SWR));
}

