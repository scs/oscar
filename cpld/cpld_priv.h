/*! @file cpld_priv.h
 * @brief Private Cpld module definition
 * 
 * 
 * The CPLD interface (AMC bus, bank0) is provided by the amc static
 * module: drivers/char/bfin_amc.c out placed in the uclinux tree.
 * 
	************************************************************************/
#ifndef CPLD_PRIV_H_
#define CPLD_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef OSC_HOST
#include <oscar_types_host.h>
#else
#include <oscar_types_target.h>
#endif /* OSC_HOST */

/*! cpld device driver */
/*#include <cpldDrv/osc_cpld.h> */ /* not required @@@ */

/*! @brief Device node which represents the cpld */
#define OSC_CPLD_DRIVER_FILE "/dev/amc0"
/*! @brief Number of mapped CPLD registers, defines Mmap aera size */
#define OSC_CPLD_MAX_REGISTER_NR      0x1ff

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
	uint8 reg[ OSC_CPLD_MAX_REGISTER_NR];
};

/******************** Private methods ******************/


#endif /* CPLD_PRIV_H_ */
