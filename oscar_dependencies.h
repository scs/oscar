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

#ifndef OSCAR_DEPENDENCIES_H_
#define OSCAR_DEPENDENCIES_H_

#include "include/oscar.h"

/*! @brief Describes a module dependency of a module and all necessary
 * information to load and unload that module. */
struct OSC_DEPENDENCY
{
	/*! @brief The name of the dependency. */
	char                strName[24];
	/*! @brief The constructor of the dependency. */
	OSC_ERR             (*create)(void *);
	/*! @brief The destructor of the dependency. */
	void                (*destroy)(void *);
};

/*********************************************************************//*!
 * @brief Loads the module depencies give in a list of modules.
 * 
 * Goes through the given dependency array and tries to create all
 * member modules. If it fails at some point, destroy the dependencies
 * already created and return with an error code.
 * 
 * @param pFw Pointer to the framework
 * @param aryDeps Array of Dependencies to be loaded.
 * @param nDeps Length of the dependency array.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscLoadDependencies(void *pFw,
		const struct OSC_DEPENDENCY aryDeps[],
		const uint32 nDeps);

/*********************************************************************//*!
 * @brief Unloads the module depencies give in a list of modules.
 * 
 * Goes through the given dependency array backwards and destroys
 * all members.
 * 
 * @param pFw Pointer to the framework
 * @param aryDeps Array of Dependencies to be unloaded.
 * @param nDeps Length of the dependency array.
 *//*********************************************************************/
void OscUnloadDependencies(void *pFw,
		const struct OSC_DEPENDENCY aryDeps[],
		const uint32 nDeps);
#endif /*OSCAR_DEPENDENCIES_H_*/
