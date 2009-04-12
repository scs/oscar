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

/*! @file vis.h
 * @brief Private vision support module definitions with object structure
 * 
 */
#ifndef VIS_PRIV_H_
#define VIS_PRIV_H_

#ifdef OSC_HOST
	#include <oscar_types_host.h>
#else
	#include <oscar_types_target.h>
#endif /* OSC_HOST */

#include <include/log.h>

/*! @brief Object struct of the bitmap module */
struct OSC_VIS {
	uint32      vis_dummy;  /*!< @brief dummy member */
};

/*======================= Private methods ==============================*/


#endif /*VIS_PRIV_H_*/
