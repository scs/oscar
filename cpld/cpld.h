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
 * @brief Private Cpld module definition
 * 
 * 
 * The CPLD interface (AMC bus, bank0) is provided by the amc static
 * module: drivers/char/bfin_amc.c out placed in the uclinux tree.
 */

#ifndef CPLD_PRIV_H_
#define CPLD_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <oscar.h>

/*! @brief Device node which represents the cpld */
#define OSC_CPLD_DRIVER_FILE "/dev/amc0"
/*! @brief Number of mapped CPLD 16bit registers, defines Mmap aera size
 * FIXME: The size should be application specific. */
#define OSC_CPLD_MAX_REGISTER_NR	0x1ff

/*! @brief Object struct of the cpld module */
struct OSC_CPLD
{
	/*! Handle to device file */
	FILE* file;
	/*! Base address of mapped CPLD register space, 16bit AMC bus */
	uint16* addr;
	
	/*! @brief Register set
	 * Host: Used for emulation on host.
	 * Target: Used as local copy (field set on write only registers) */
	uint16 reg[ OSC_CPLD_MAX_REGISTER_NR];
};

/******************** Private methods ******************/


#endif /* CPLD_PRIV_H_ */
