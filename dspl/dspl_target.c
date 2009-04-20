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

/*! @file dspl_target.c
 * @brief Blackfin DSP runtime library implementation for host.
 * 
 * All functions of the DSP runtime library used on the target must
 * be implemented for the host. The implementation for the target on the
 * other hand must not be done in the framework, since it already exists
 * in optimized form in the library.
 * 
 */

#include "oscar_types_target.h"

#include "dspl_pub.h"
#include "dspl_priv.h"
#include "oscar_intern.h"

/*! @brief The module singelton instance.  */
struct OSC_DSPL osc_dspl;


OSC_ERR OscDsplCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->dspl.useCnt != 0)
	{
		pFw->dspl.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
		
	memset(&osc_dspl, 0, sizeof(struct OSC_DSPL));
		
	/* Increment the use count */
	pFw->dspl.hHandle = (void*)&osc_dspl;
	pFw->dspl.useCnt++;

	return SUCCESS;
}

void OscDsplDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->dspl.useCnt--;
	if(pFw->dspl.useCnt > 0)
	{
		return;
	}
	
	memset(&osc_dspl, 0, sizeof(struct OSC_DSPL));
}


inline float OscDsplFr16ToFloat(fract16 n)
{
	return (((float)n)/FR16_SCALE);
}

inline fract16 OscDsplFloatToFr16(float n)
{
	float ret = (n*FR16_SCALE);
	if(ret > FR16_MAX)
		ret = FR16_MAX;
	return (fract16)ret;
}
