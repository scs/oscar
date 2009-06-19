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
	.create = OscCpldCreate,
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

// FIXME: Why do we have this module on targets other than the indXcam?
OSC_ERR OscCpldCreate()
{
#ifdef TARGET_TYPE_INDXCAM
	cpld = (struct OSC_CPLD) { };
#else
	OscLog(ERROR, "%s: No CPLD available on this hardware platform!\n",
				__func__);
	return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldRset(
		const uint16 regId,
		const uint8 val)
{
#ifdef TARGET_TYPE_INDXCAM
	cpld.reg[ regId] = val;
	return SUCCESS;
#else
	return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldFset(
		uint16 regId,
		uint8 field,
		uint8 val)
{
#ifdef TARGET_TYPE_INDXCAM
	uint8 current;
	current = cpld.reg[ regId];
	
	/* Set bits. */
	current = current | (field & val);
	
	/* Clear bits. */
	current = current & ~(field & (~val));
	
	cpld.reg[ regId] = current;
	return SUCCESS;
#else
	return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldRget(
		const uint16 regId,
		uint8* val)
{
#ifdef TARGET_TYPE_INDXCAM
	*val = cpld.reg[ regId];
	return SUCCESS;
#else
	return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldFget(
		const uint16 regId,
		const uint8 field,
		uint8* val)
{
#ifdef TARGET_TYPE_INDXCAM
	uint8 current = cpld.reg[ regId];
	if( current & field)
	{
		*val = 1;
	}
	else
	{
		*val = 0;
	}
	return SUCCESS;
#else
	return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}
