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

/*! @file
 * @brief Stimuli reader module implementation for target.
 */

#ifdef OSC_SIM
 #include "srd_host.c"
#else /* OSC_SIM */

#include "srd.h"

struct OSC_SRD srd;     /*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY srd_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscSrdCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->srd.useCnt != 0)
	{
		pFw->srd.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}

	/* Load the module srd_deps of this module. */
	err = OscLoadDependencies(pFw,
			srd_deps,
			sizeof(srd_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load srd_deps! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	memset(&srd, 0, sizeof(struct OSC_SRD));
	
	/* Increment the use count */
	pFw->srd.hHandle = (void*)&srd;
	pFw->srd.useCnt++;
	
	return SUCCESS;
}

void OscSrdDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->srd.useCnt--;
	if(pFw->srd.useCnt > 0)
	{
		return;
	}
	
	OscUnloadDependencies(pFw,
			srd_deps,
			sizeof(srd_deps)/sizeof(struct OSC_DEPENDENCY));
	
	memset(&srd, 0, sizeof(struct OSC_SRD));
}

#endif /* OSC_SIM */
