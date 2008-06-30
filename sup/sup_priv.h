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
#define SRAM_SCRATCH_LENGTH 0x0800 /*  2 kB */

#ifdef OSC_TARGET
/*! @brief The address of the L1 SRAM Block A */
#define SRAM_L1A            0xFF800000

/*! @brief The address of the L1 SRAM Block B */
#define SRAM_L1B            0xFF900000

/*! @brief The address of the L1 Scratchpad memory */
#define SRAM_SCRATCH        0xFFB00000
#endif /* OSC_TARGET */

/*! @brief The object struct of the sup module */
struct OSC_SUP
{
    int fdWatchdog; /*!< @brief The file descriptor of the watchdog. */
#ifdef TARGET_TYPE_INDXCAM
    /*! @brief The file descriptor of the onboard LED. */
    int fdLed;
#endif
#ifdef OSC_HOST
    /*! @brief Host only: Memory area for the L1 SRAM Block A */
    uint8 pL1A[SRAM_L1A_LENGTH];
    /*! @brief Host only: Memory area for the L1 SRAM Block B */
    uint8 pL1B[SRAM_L1B_LENGTH];
    /*! @brief Host only: Memory area for the L1 SRAM Scratchpad */
    uint8 pScratch[SRAM_SCRATCH_LENGTH];
#endif /* OSC_HOST */

};

/*======================= Private methods ==============================*/


#endif /*SUP_PRIV_H_*/
