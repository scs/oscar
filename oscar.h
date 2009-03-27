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

/*! @file oscar.h
 * @brief API definition for Oscar framework
 * 
 * Must be included by the application.
 * 
 */

#ifndef OSCAR_H_
#define OSCAR_H_

/* Include the correct type header file, depending on the target
 */
#ifdef OSC_HOST
	#include "oscar_types_host.h"
	#include "oscar_host.h"
#endif
#ifdef OSC_TARGET
	#include "oscar_types_target.h"
	#include "oscar_target.h"
#endif

#include "oscar_version.h"
#include "oscar_error.h"
#include "oscar_dependencies.h"


/*********************************************************************//*!
 * @brief Constructor for framework
 * 
 * @param phFw Pointer to the handle location for the framework
 * @return SUCCESS or appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCreate(void ** phFw);

/*********************************************************************//*!
 * @brief Destructor for framework
 * 
 * Fails if not all loaded modules have been destroyed.
 * 
 * @param hFw Pointer to the handle of the framework to be destroyed.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDestroy(void *hFw);

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
#include "log_pub.h"
#include "dspl_pub.h"
#include "dma_pub.h"
#include "ipc_pub.h"
#include "bmp_pub.h"
#include "sup_pub.h"
#ifdef TARGET_TYPE_INDXCAM
#include "lgx_pub.h"
#endif
#ifndef TARGET_TYPE_MESA_SR4K
#include "cam_pub.h"
#include "cpld_pub.h"
#include "sim_pub.h"
#include "swr_pub.h"
#include "srd_pub.h"
#include "frd_pub.h"
#include "hsm_pub.h"
#include "cfg_pub.h"
#include "clb_pub.h"
#include "vis_pub.h"
#include "gpio_pub.h"
#endif

#endif /*OSCAR_H_*/
