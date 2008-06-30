/*! @file sup_priv.h
 * @brief Private support module definition
 * 
 * @author Markus Berner
 */
#ifndef SUP_PRIV_H_
#define SUP_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef LCV_HOST
    #include <framework_types_host.h>
#else
    #include <framework_types_target.h>
#endif /* LCV_HOST */

#include <log/log_pub.h>

/*! @brief The frequency of the target CPU in Hz. */
#define CPU_FREQ 500000000

/*! @brief The length of the L1 SRAM Block A */
#define SRAM_L1A_LENGTH     0x4000 /* 16 kB */
/*! @brief The length of the L1 SRAM Block B */
#define SRAM_L1B_LENGTH     0x4000 /* 16 kB */
/*! @brief The length of the L1 Scratchpad memory */
#define SRAM_SCRATCH_LENGTH 0x0800 /*  2 kB */

#ifdef LCV_TARGET
/*! @brief The address of the L1 SRAM Block A */
#define SRAM_L1A            0xFF800000

/*! @brief The address of the L1 SRAM Block B */
#define SRAM_L1B            0xFF900000

/*! @brief The address of the L1 Scratchpad memory */
#define SRAM_SCRATCH        0xFFB00000
#endif /* LCV_TARGET */

/*! @brief The object struct of the sup module */
struct LCV_SUP
{
    int fdWatchdog; /*!< @brief The file descriptor of the watchdog. */
#ifdef TARGET_TYPE_LCV_IND
    /*! @brief The file descriptor of the onboard LED. */
    int fdLed;
#endif
#ifdef LCV_HOST
    /*! @brief Host only: Memory area for the L1 SRAM Block A */
    uint8 pL1A[SRAM_L1A_LENGTH];
    /*! @brief Host only: Memory area for the L1 SRAM Block B */
    uint8 pL1B[SRAM_L1B_LENGTH];
    /*! @brief Host only: Memory area for the L1 SRAM Scratchpad */
    uint8 pScratch[SRAM_SCRATCH_LENGTH];
#endif /* LCV_HOST */

};

/*======================= Private methods ==============================*/


#endif /*SUP_PRIV_H_*/
