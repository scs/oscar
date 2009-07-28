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
 * @brief Simulation module implementation for host
 */

#include "sim.h"

OSC_ERR OscSimCreate();

/*! @brief The module singelton instance. */
// FIXME: Why do we have this instance on the host, or even the whole module?
struct OSC_SIM_OBJ sim;

struct OscModule OscModule_sim = {
	.name = "sim",
	.create = OscSimCreate,
	.dependencies = {
		NULL // To end the flexible array.
	}
};

OSC_ERR OscSimCreate()
{
	sim = (struct OSC_SIM_OBJ) { };
		
	return SUCCESS;
}

/*********************************************************************//*!
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
OSC_ERR OscSimInitialize(void) {
}

OSC_ERR OscSimStep() {
}

uint32 OscSimGetCurTimeStep() {
	return SUCCESS; // FIXME: Doesn't SUCCESS kind of have the wrong type!?
}


/*********************************************************************//*!
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
OSC_ERR OscSimRegisterCycleCallback( void (*pCallback)(void))
{
	return SUCCESS;
}
