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
 * @brief Support module implementation for host
 */

#include <time.h>
#include "sup.h"

/*! @brief The module singelton instance. */
struct OSC_SUP sup;

OSC_ERR OscSupCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->sup.useCnt != 0)
	{
		pFw->sup.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	memset(&sup, 0, sizeof(struct OSC_SUP));
	
	/* Increment the use count */
	pFw->sup.hHandle = (void*)&sup;
	pFw->sup.useCnt++;
	
	return SUCCESS;
}

void OscSupDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
			
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->sup.useCnt--;
	if(pFw->sup.useCnt > 0)
	{
		return;
	}
	
	/* Make sure the watchdog is closed, otherwise the board may
	 * be reset by it after the application has closed. */
	OscSupWdtClose();
	
	memset(&sup, 0, sizeof(struct OSC_SUP));
}

/*=========================== Watchdog =================================*/

OSC_ERR OscSupWdtInit()
{
	/* There is no watchdog on the host => do nothing. */

	return SUCCESS;
}

OSC_ERR OscSupWdtClose()
{
	/* There is no watchdog on the host => do nothing. */

	return SUCCESS;
}

inline void OscSupWdtKeepAlive()
{
	/* There is no watchdog on the host => do nothing. */
}

/*============================= Cycles =================================*/
inline uint32 OscSupCycGet()
{
	/* The host implementation uses the ANSI C function clock()
	 * with much lower precision than on the blackfin.
	 * The actual precision is dependent on the platform. */

	/* ANSI C function, low resolution */
	return ((uint32)clock());
}

inline long long OscSupCycGet64()
{
	/* The host implementation uses the ANSI C function clock()
	 * with much lower precision than on the blackfin.
	 * The actual precision is dependent on the platform. */

	/* ANSI C function, low resolution */
	return ((long long)clock());
}

inline uint32 OscSupCycToMicroSecs(uint32 cycles)
{
	return (cycles/(CLOCKS_PER_SEC/1000000));
}

inline uint32 OscSupCycToMilliSecs(uint32 cycles)
{
	return (cycles/((CPU_FREQ/1000000)*1000));
}

inline uint32 OscSupCycToSecs(uint32 cycles)
{
	return (cycles/((CPU_FREQ/1000000)*1000000));
}

inline long long OscSupCycToMicroSecs64(long long cycles)
{
	return (cycles/(CPU_FREQ/1000000));
}

inline long long OscSupCycToMilliSecs64(long long cycles)
{
	return (cycles/((CPU_FREQ/1000000)*1000));
}

inline long long OscSupCycToSecs64(long long cycles)
{
	return (cycles/((CPU_FREQ/1000000)*1000000));
}


/*============================== SRAM =================================*/
void* OscSupSramAllocL1DataA(unsigned int size)
{
	if(size > SRAM_L1A_LENGTH)
		return NULL;
		
	/* Just allocate normal memory on host. */
	return malloc(size);
}

void* OscSupSramAllocL1DataB(unsigned int size)
{
	if(size > SRAM_L1B_LENGTH)
		return NULL;
		
	/* Just allocate normal memory on host. */
	return malloc(size);
}

void* OscSupSramAllocL1Data(unsigned int size)
{
	if((size > SRAM_L1A_LENGTH) && (size > SRAM_L1B_LENGTH))
		return NULL;
		
	/* Just allocate normal memory on host. */
	return malloc(size);
}

void* OscSupSramAllocL1Instr(unsigned int size)
{
	if(size > SRAM_INSTR_LENGTH)
		return NULL;
		
	/* Just allocate normal memory on host. */
	return malloc(size);
}

void* OscSupSramAllocScratch(unsigned int size)
{
	OscLog(ERROR, "%s: Allocating scratchpad not supported since "
					"not supported by uClinux yet!\n",
					__func__);
	return NULL;
}

OSC_ERR OscSupSramFree(void *pAddr)
{
	free(pAddr);
	return SUCCESS;
}


