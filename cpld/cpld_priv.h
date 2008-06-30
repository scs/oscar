/*! @file cpld_priv.h
 * @brief Private Cpld module definition
 * 
 * @author Samuel Zahnd
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

#ifdef LCV_HOST
#include <framework_types_host.h>
#else
#include <framework_types_target.h>
#endif /* LCV_HOST */

/*! cpld device driver */
/*#include <cpldDrv/lcv_cpld.h> */ /* not required @@@ */

/*! @brief Device node which represents the cpld */
#define LCV_CPLD_DRIVER_FILE "/dev/amc0"
/*! @brief Number of mapped CPLD registers, defines Mmap aera size */
#define LCV_CPLD_MAX_REGISTER_NR      0x1ff

/*! @brief Object struct of the cpld module */
struct LCV_CPLD
{
    /*! Handle to device file */
    FILE* file;
    /*! Base address of mapped CPLD register space, 16bit AMC bus */
    uint16* addr;
    
    /*! @brief Register set
     * Host: Used for emulation on host.
     * Target: Used as local copy (field set on write only registers) */
    uint8 reg[ LCV_CPLD_MAX_REGISTER_NR];        
};

/******************** Private methods ******************/


#endif /* CPLD_PRIV_H_ */
