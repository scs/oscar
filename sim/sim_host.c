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

/*! @file sim_host.c
 * @brief Simulation module implementation for host
 * 
 */

#include "oscar_intern.h"

#include "include/sim.h"
#include "sim.h"

/*! @brief The module singelton instance. */
struct OSC_SIM_OBJ sim;


OSC_ERR OscSimCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->sim.useCnt != 0)
	{
		pFw->sim.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	memset(&sim, 0, sizeof(struct OSC_SIM_OBJ));
	
	/* Increment the use count */
	pFw->sim.hHandle = (void*)&sim;
	pFw->sim.useCnt++;
	
	return SUCCESS;
}

void OscSimDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
			
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->sim.useCnt--;
	if(pFw->sim.useCnt > 0)
	{
		return;
	}
	
	memset(&sim, 0, sizeof(struct OSC_SIM_OBJ));
}

void OscSimInitialize(void)
{
	uint16 i;
	
	for( i=0; i< sim.numCycleCallback; i++)
	{
		(*sim.aryCycleCallback[ i])();
	}
}

void OscSimStep()
{
	uint16 i;
	
	/* pre advance simulation time */
	
	/* advance simulation time */
	sim.curTimeStep++;
	
	/* post advance operations */
	for( i=0; i< sim.numCycleCallback; i++)
	{
		(*sim.aryCycleCallback[ i])();
	}
}

uint32 OscSimGetCurTimeStep()
{
	return sim.curTimeStep;
}


OSC_ERR OscSimRegisterCycleCallback( void (*pCallback)(void))
{
	uint16 id;
	
	id = sim.numCycleCallback;
	if( MAX_NUMBER_CALLBACK_FUNCTION == id)
	{
		printf("Error: Maximum number of callback functions exhausted");
		return -ENUM_CALLBACK_EXHAUSTED;
	}
	sim.aryCycleCallback[ id] = pCallback;
	sim.numCycleCallback++;
	
	return SUCCESS;
}

