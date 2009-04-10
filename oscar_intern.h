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

/*! @file oscar_intern.h
 * @brief Framework internal definitions
 */

#ifndef OSCAR_INTERN_H_
#define OSCAR_INTERN_H_

#include "oscar_dependencies.h"
#include "oscar_version.h"
#include "oscar_main_header_file_which_may_be_included_from_inside_a_module.h"

#ifdef OSC_HOST
	#include "oscar_types_host.h"
#endif
#ifdef OSC_TARGET
	#include "oscar_types_target.h"
#endif

#define RND_MOD_Unbiased
/*#define RND_MOD_Biased*/



/*! @brief Describes an OSC module and keeps track of how many users
 * hold references to it. */
struct OSC_MODULE
{
	/*! @brief Handle to the module. */
	void *      hHandle;
	/*! @brief Used to prevent premature unloading. */
	int         useCnt;
};

/*! @brief The Oscar framework object with handle to modules*/
struct OSC_FRAMEWORK
{
	struct OSC_MODULE log;     /*!< @brief logging */
	struct OSC_MODULE cam;     /*!< @brief camera */
	struct OSC_MODULE cpld;    /*!< @brief cpld */
	struct OSC_MODULE lgx;     /*!< @brief logic */
	struct OSC_MODULE sim;     /*!< @brief simulation */
	struct OSC_MODULE bmp;     /*!< @brief bitmap */
	struct OSC_MODULE swr;     /*!< @brief stimulation writer */
	struct OSC_MODULE srd;     /*!< @brief stimulation reader*/
	struct OSC_MODULE ipc;     /*!< @brief interprocess communication */
	struct OSC_MODULE sup;     /*!< @brief support */
	struct OSC_MODULE frd;     /*!< @brief filename reader */
	struct OSC_MODULE dspl;    /*!< @brief DSP runtime library */
	struct OSC_MODULE dma;     /*!< @brief Memory DMA */
	struct OSC_MODULE hsm;     /*!< @brief hierarchical state machine*/
	struct OSC_MODULE cfg;     /*!< @brief configuration file reader and writer*/
	struct OSC_MODULE clb;     /*!< @brief camera calibration*/
	struct OSC_MODULE vis;     /*!< @brief Vision library*/
	struct OSC_MODULE gpio;    /*!< @brief GPIO */
	struct OSC_MODULE jpg;     /*!< @brief JPG*/
};

#endif /* OSCAR_INTER_H_ */

