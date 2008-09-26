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

/*! @file sup_priv.h
 * @brief Private support module definition
 * 
 */
#ifndef SUP_PRIV_H_
#define SUP_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef OSC_HOST
	#include <oscar_types_host.h>
#else
	#include <oscar_types_target.h>
#endif /* OSC_HOST */

#include <log/log_pub.h>

/*! @brief The frequency of the target CPU in Hz. */
#define CPU_FREQ 500000000

/*! @brief The length of the L1 SRAM Block A */
#define SRAM_L1A_LENGTH     0x4000 /* 16 kB */
/*! @brief The length of the L1 SRAM Block B */
#define SRAM_L1B_LENGTH     0x4000 /* 16 kB */
/*! @brief The length of the L1 Scratchpad memory */
#define SRAM_SCRATCH_LENGTH 0x1000 /*  4 kB */
/*! @brief The length of the instruction memory. */
#define SRAM_INSTR_LENGTH   0xC000 /* 48 kB */

/*! @brief The object struct of the sup module */
struct OSC_SUP
{
	int fdWatchdog; /*!< @brief The file descriptor of the watchdog. */
#ifdef TARGET_TYPE_INDXCAM
	/*! @brief The file descriptor of the onboard LED. */
	int fdLed;
#endif
};

/*======================= Private methods ==============================*/


#endif /*SUP_PRIV_H_*/
