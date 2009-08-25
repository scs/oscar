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
 * @brief Cpld module implementation for host.
 */

#include "cpld.h"

OSC_ERR OscCpldCreate();

struct OSC_CPLD cpld; /*!< The cpld module singelton instance */

struct OscModule OscModule_cpld = {
	.name = "cpld",
	.create = OscCpldCreate,
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

OSC_ERR OscCpldCreate()
{
	cpld = (struct OSC_CPLD) { };
	return SUCCESS;
}

OSC_ERR OscCpldRset(
		const uint16 regId,
		const uint16 val)
{
	cpld.reg[ regId] = val;
	return SUCCESS;
}


OSC_ERR OscCpldFset(
		uint16 regId,
		uint16 field,
		uint16 val)
{
	uint16 current;
	current = cpld.reg[ regId];
	
	/* Set bits. */
	current = current | (field & val);
	
	/* Clear bits. */
	current = current & ~(field & (~val));
	
	cpld.reg[ regId] = current;
	return SUCCESS;
}

OSC_ERR OscCpldRget(
		const uint16 regId,
		uint16* val)
{
	*val = cpld.reg[ regId];
	return SUCCESS;
}

OSC_ERR OscCpldFget(
		const uint16 regId,
		const uint16 field,
		uint16* val)
{
	uint16 current = cpld.reg[ regId];
	if( current & field)
	{
		*val = 1;
	}
	else
	{
		*val = 0;
	}
	return SUCCESS;
}
