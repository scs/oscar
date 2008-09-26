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

/* Include the public header files of the different modules, which
 * contain the declarations of the API functions of the respective
 * module.
 */
#include "log_pub.h"
#include "cam_pub.h"
#include "cpld_pub.h"
#ifdef TARGET_TYPE_INDXCAM
	#include "lgx_pub.h"
#endif
#include "sim_pub.h"
#include "bmp_pub.h"
#include "swr_pub.h"
#include "srd_pub.h"
#include "ipc_pub.h"
#include "sup_pub.h"
#include "frd_pub.h"
#include "dspl_pub.h"
#include "dma_pub.h"
#include "hsm_pub.h"
#include "cfg_pub.h"
#include "clb_pub.h"
#include "vis_pub.h"
#include "gpio_pub.h"

#endif /*OSCAR_H_*/
