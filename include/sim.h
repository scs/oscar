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

/*! @file sim_pub.h
 * @brief API definition for simulation module
 * 
 * Simulation is only used for host. Target implementation provides
 * stump function definition.
 * 
 */
#ifndef SIM_PUB_H_
#define SIM_PUB_H_

#include "oscar.h"

/*! @brief Module-specific error codes.
 * 
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscSimErrors {
	ENUM_CALLBACK_EXHAUSTED = OSC_SIM_ERROR_OFFSET
};

/*======================== API functions ===============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSimCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscSimDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Initialize simulation
 * 
 * After creation of all required modules the application has to init
 * the simulation module. Time variable is set to 0. Callbacks to
 * stimuli reader and writer are issued to applie default signal values.
 *//*********************************************************************/
void OscSimInitialize(void);

/*********************************************************************//*!
 * @brief Increment the simulation time step.
 * 
 * The application should call this function after every 'frame' of
 * the simulation. This automatically adjusts the current test image
 * file name fo the next cycle. Callback for stimuli reader and
 * writer is issued after timer increment. This prepares the the input
 * signals for the next cycle and writes current output signals (to
 * next cycle!)
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
void OscSimStep();

/*********************************************************************//*!
 * @brief Get the current simulation time step.
 * 
 * Target: Stump since simulation is only done on host.
 * 
 * @return The current simulation time step.
 *//*********************************************************************/
uint32 OscSimGetCurTimeStep();

/*********************************************************************//*!
 * @brief Register a callback function to be called every new timestep.
 * 
 * Target: Stump since simulation is only done on host.
 * 
 * @param pCallback Pointer to the function to be called.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscSimRegisterCycleCallback( void (*pCallback)(void));

#endif /*SIM_PUB_H_*/
