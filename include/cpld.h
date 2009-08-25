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
 * @brief API definition for Cpld module.
 * 
 * Cpld provides the basic functionality to access
 * a CPLD device and its register set.
 * 
	************************************************************************/
#ifdef TARGET_TYPE_INDXCAM

#ifndef CPLD_PUB_H_
#define CPLD_PUB_H_

extern struct OscModule OscModule_cpld;

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscCpldErrors
{
	ENO_CPLD_DEVICE_FOUND = OSC_CPLD_ERROR_OFFSET
};

/*======================= API functions ================================*/

/*********************************************************************//*!
 * @brief Write 16bit register
 * 
 * @param redId	register offset
 * @param val   register value
 * @return SUCCESS
 *//*********************************************************************/
OSC_ERR OscCpldRset( const uint16 regId, const uint16 val);

/*********************************************************************//*!
 * @brief Write field of register
 * 
 * @param regId	register offset
 * @param field field mask
 * @param val   field value
 * @return SUCCESS
 *//*********************************************************************/
OSC_ERR OscCpldFset( const uint16 regId, const uint16 field, const uint16 val);

/*********************************************************************//*!
 * @brief Read register
 * 
 * @param regId	register offset
 * @param val	return register value
 * @return SUCCESS
 *//*********************************************************************/
OSC_ERR OscCpldRget( const uint16 regId, uint16* val);

/*********************************************************************//*!
 * @brief Read field of register
 * 
 * @param regId		register offset
 * @param field 	field mask
 * @param val   	return field value
 * @return SUCCESS
 *//*********************************************************************/
OSC_ERR OscCpldFget( const uint16 regId, const uint16 field, uint16* val);


#endif /*CPLD_PUB_H_*/

#endif /* TARGET_TYPE_INDXCAM */
