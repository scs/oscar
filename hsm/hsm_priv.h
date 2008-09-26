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

/*! @file hsm_priv.h
 * @brief Private HSM module definition
 * 
 * Original code August 2000, published in www.embedded.com
 * State-Oriented Programming by M. Samek & P.Y. Montgomery
 * 
	************************************************************************/
#ifndef HSM_PRIV_H_
#define HSM_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef OSC_HOST
#include <oscar_types_host.h>
#else
#include <oscar_types_target.h>
#endif /* OSC_HOST */


/*!@brief object struct */
struct OSC_HSM
{
	bool dummy;                    /*!< @brief dummy variable */
	/* Wie genau fliess hier das HSM pattern ein?
	 * Im framework test braucht es auch ein HSM Test.
	 * Wie ist die Unterstützung für follow up events?
	 * 
	 * */
};





#endif /* HSM_PRIV_H_ */
