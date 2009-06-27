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
 * @brief API definition for Oscar framework
 * 
 * Must be included by the application.
 */

#ifndef OSCAR_INCLUDE_OSCAR_H_
#define OSCAR_INCLUDE_OSCAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Support file for the NIH design pattern. */
#include "nih.h"
#include "version.h"
#include "error.h"
#include "pool.h"
#include "list.h"
#include "support.h"

/*! @brief Describes an OSC module and keeps track of how many users
 * hold references to it. */
struct OscModule {
	OSC_ERR (* create) ();
	OSC_ERR (* destroy) ();
	int useCount;
	struct OscModule * dependencies[];
};

/*! @brief Constructor for framework
	@param modules A list of modules to load as defined in enum OscModule.
	@return SUCCESS or appropriate error code otherwise.
*/
#define OscCreate(modules ...) \
	({ \
		static struct OscModule * _modules[] = { modules, NULL }; \
		OscCreateFunction(&_modules); \
	})

OSC_ERR OscCreateFunction(struct OscModule ** modules);

/*********************************************************************//*!
 * @brief Destructor for framework
 * 
 * Fails if not all loaded modules have been destroyed.
 * 
 * @param hFw Pointer to the handle of the framework to be destroyed.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDestroy();

/*********************************************************************//*!
 * @brief Get framework version numbers
 * 
 * Used scheme: major.minor[.revsion]
 * 
 * The major number is used for significant changes in functionality or 
 * supported plattform. Instable pre releases use a major number of 0.
 * The minor number decodes small feature changes.
 * The patch number is intended for bug fixes without changes of API.
 * 
 * @param hMajor Pointer to major version number.
 * @param hMinor Pointer to minor version number.
 * @param hPatch Pointer to patch number.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGetVersionNumber(char *hMajor, char *hMinor, char *hPatch);

/*********************************************************************//*!
 * @brief Get framework version string
 * 
 * Version string format: v<major>.<minor>[-p<patch>]  eg: v1.3  or v1.3-p1
 * The patch number is not printed if no bug-fixes are available (patch=0).
 *  
 * See @see OscGetVersionNumber for number interpretation.
 * 
 * @param hMajor Pointer to formated version string.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGetVersionString(char *hVersion);

/* Include the public header files of the different modules, which
 * contain the declarations of the API functions of the respective
 * module.
 */
#include "board.h"

#ifdef __cplusplus
}
#endif

#endif /* OSCAR_INCLUDE_OSCAR_H_ */
