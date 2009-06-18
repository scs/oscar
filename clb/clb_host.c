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
 * @brief Calibration module implementation
 * 
 * No calibration is applied to host pictures.
 */

#include "clb.h"

#include <stdlib.h>

struct OscModule OscModule_clb = {
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

OSC_ERR OscClbSetupCalibrate(
		enum EnOscClbCalibrateSlope calibSlope,
		bool bHotpixel)
{
	return SUCCESS;
}

OSC_ERR OscClbApplyCorrection( uint8 *pImg,
		const uint16 lowX, const uint16 lowY,
		const uint16 width, const uint16 height)
{
	return SUCCESS;
}

