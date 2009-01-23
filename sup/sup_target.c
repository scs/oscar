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

/*! @file sup_target.c
 * @brief Support module implementation for host
 * 
 */

#include "oscar_types_target.h"

#include "sup_pub.h"
#include "sup_priv.h"
#include "oscar_intern.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <bfin_sram.h>

/*! @brief The module singelton instance. */
struct OSC_SUP sup;

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY sup_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
};

/*! @brief The length of the dependency array of this module. */
#define DEP_LEN (sizeof(sup_deps)/sizeof(struct OSC_DEPENDENCY))

OSC_ERR OscSupCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->sup.useCnt != 0)
	{
		pFw->sup.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	/* Load the module dependencies of this module. */
	err = OscLoadDependencies(pFw,
			sup_deps,
			DEP_LEN);
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load dependencies! (%d)\n",
				__func__,
				err);
		return err;
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
	if(sup.fdWatchdog > 0)
	{
		OscSupWdtClose();
	}
	
	OscUnloadDependencies(pFw,
			sup_deps,
			DEP_LEN);
	
	memset(&sup, 0, sizeof(struct OSC_SUP));
}

/*=========================== Watchdog =================================*/

OSC_ERR OscSupWdtInit()
{
	if(sup.fdWatchdog != 0)
	{
		OscLog(WARN, "%s: Watchdog already initialized!\n", __func__);
		return -EALREADY_INITIALIZED;
	}

	sup.fdWatchdog = open("/dev/watchdog", O_WRONLY);
	if (sup.fdWatchdog < 0)
	{
		OscLog(ERROR, "%s: Unable to open watchdog device node! (%s)\n",
				__func__,
				strerror(errno));
		return -EDEVICE;
	}

	return SUCCESS;
}

OSC_ERR OscSupWdtClose()
{
	char magicChar = 'V';
	
	if(sup.fdWatchdog <= 0)
	{
		/* No watchdog open. */
		OscLog(WARN, "%s: Watchdog not initialized!\n", __func__);
		return -ENO_SUCH_DEVICE;
	}

	/* Send the magic character to disable the watchdog
	 * just before closing the file. This is in case the watchdog
	 * driver has not been configured with CONFIG_WATCHDOG_NOWAYOUT.
	 * Otherwise it resets the board in any case after the
	 * application has been closed. */
	write(sup.fdWatchdog, &magicChar, 1);
	fsync(sup.fdWatchdog);

	close(sup.fdWatchdog);
	sup.fdWatchdog = 0;

	return SUCCESS;
}

inline void OscSupWdtKeepAlive()
{
	if(sup.fdWatchdog <= 0)
	{
		/* No watchdog open. */
		OscLog(WARN, "%s: Watchdog not initialized!\n", __func__);
		return;
	}
	/* Write anything to the watchdog to disable it and
	 * make sure it is flushed. */
	write(sup.fdWatchdog, "\0", 1);
	fsync(sup.fdWatchdog);
}

/*============================= Cycles =================================*/

/*! /todo Low priority: Use whole 64 bit cycles counter. */
inline uint32 OscSupCycGet()
{
	uint32 ret;

	/* Read out the blackfin-internal cycle count register.
	 * This is a running counter of the number of cycles since start.
	 * Together with CYCLES2 this would actually be a 64 bit value,
	 * but in this function we only read out the lower 32 bits.
	 * (See OscSupCycGet64 for full readout).
	 * This yields a wrap-around time of around 8 secs with a 500 Mhz CPU,
	 * which should be sufficient for most cases. */
	__asm__ __volatile__ ("%0 = CYCLES;\n":"=d"(ret));
	return ret;
}

inline long long OscSupCycGet64()
{
	unsigned long long t0;

	/* Read out the blackfin-internal cycle count register.
	 * This is a running counter of the number of cycles since start.
	 * Together with CYCLES2 this gives actually be a 64 bit value.
	 * The blackfin has a nice hardware mechanic that stores the current
	 * value of CYCLES2 into a shadow register when CYCLES is read out.
	 * Therefore, no overflow check is required. */
	__asm__ __volatile__ ("%0=cycles; %H0=cycles2;" : "=d" (t0));
	return t0;
}

inline uint32 OscSupCycToMicroSecs(uint32 cycles)
{
	return (cycles/(CPU_FREQ/1000000));
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
	return sram_alloc(size, L1_DATA_A_SRAM);
}

void* OscSupSramAllocL1DataB(unsigned int size)
{
	return sram_alloc(size, L1_DATA_B_SRAM);
}

void* OscSupSramAllocL1Data(unsigned int size)
{
	return sram_alloc(size, L1_DATA_SRAM);
}

void* OscSupSramAllocL1Instr(unsigned int size)
{
	return sram_alloc(size, L1_INST_SRAM);
}

void* OscSupSramAllocScratch(unsigned int size)
{
	OscLog(ERROR, "%s: Allocating scratchpad not supported since "
					"not supported by uClinux yet!\n",
					__func__);
	return NULL;
}

int OscSupSramFree(void *pAddr)
{
	if(!sram_free(pAddr))
	{
		return SUCCESS;
	} else {
		return -EINVALID_PARAMETER;
	}
}
