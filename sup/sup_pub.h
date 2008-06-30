/*! @file sup_pub.h
 * @brief Private support module definition
 * 
 * Contains stuff that is too small to be put in a separate module.
 * 
 */
#ifndef SUP_PUB_H_
#define SUP_PUB_H_

#include "oscar_error.h"
#ifdef OSC_HOST
    #include "oscar_types_host.h"
    #include "oscar_host.h"
#else
    #include "oscar_types_target.h"
    #include "oscar_target.h"
#endif /* OSC_HOST */


/*======================== API functions ===============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSupCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscSupDestroy(void *hFw);

/*--------------------------- Watchdog ---------------------------------*/

/*********************************************************************//*!
 * @brief Initialize and start the watchdog timer.
 * 
 * The default watchdog timeout is 1 minute. Depending on the 
 * setting in the linux driver configuration (CONFIG_WATCHDOG_NOWAYOUT), 
 * the watchdog is closed when closing the file (e.g. due to a crash of
 * the application using the framework, or it can not be disabled.
 * 
 * Host: Since there is usually no watchdog support on regular PCs,
 * the functions are implemented for compability but they do not exert
 * any function.
 * 
 * Target: For the watchdog commands to work, the watchdog char driver 
 * has to be enabled in the uCLinux Kernel configuration. For hardware
 * watchdog support, KGDB has to be disabled; only then will the option 
 * for the built-in hardware watchdog appear. Otherwise, the less
 * reliable software watchdog can be used.
 * 
 * @see OscSupWdtInit
 * @see OscSupWdtClose
 * @see OscSupWdtKeepAlive
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscSupWdtInit();

/*********************************************************************//*!
 * @brief Close and stop an initialized watchdog.
 * 
 * This function will close the device file. It does not work when
 * CONFIG_WATCHDOG_NOWAYOUT has been configured.
 * 
 * Host: No functionality.
 * 
 * @see OscSupWdtInit
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscSupWdtClose();

/*********************************************************************//*!
 * @brief Reset the watchdog counter tracking the keep alive period.
 * 
 * @see OscSupWdtInit
 * 
 * Host: No functionality.
 *//*********************************************************************/
inline void OscSupWdtKeepAlive();

/*---------------------------- Cycles ----------------------------------*/

/*********************************************************************//*!
 * @brief Get the current cycle count to measure time.
 * 
 * Target: Reads out the current cycle count from the blackfin hardware
 * register and returns the lower 32 bit.
 * 
 * Host: Get the cycle count from the ANSI C function clock(). The time
 * resolution is worse with this method, but it is compatible among a
 * wide selection of platforms.
 * /todo Low priority: Use whole 64 bit cycles counter.
 * @see OscSupCycToMicroSecs
 * 
 * @return The current cycle count (wraps).
 *//*********************************************************************/
inline uint32 OscSupCycGet();

/*********************************************************************//*!
 * @brief Convert a cycle count to microseconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * /todo Low priority: Use whole 64 bit cycles counter.
 * @see OscSupCycGet
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
inline uint32 OscSupCycToMicroSecs(uint32 cycles);

/*------------------------------ SRAM ----------------------------------*/

/*********************************************************************//*!
 * @brief Query the length of the available L1 SRAM in Block A
 * 
 * @see OscSupSramL1A
 * 
 * @return Length
 *//*********************************************************************/
inline uint32 OscSupSramL1ALen();

/*********************************************************************//*!
 * @brief Get the pointer to the available L1 SRAM in block A
 * 
 * Host: This is implemented by a temporary memory area which is not
 * really faster than any other.
 * @see OscSupSramL1ALen
 * 
 * @return Pointer to L1A.
 *//*********************************************************************/
inline void* OscSupSramL1A();

/*********************************************************************//*!
 * @brief Query the length of the available L1 SRAM in Block B
 * 
 * @see OscSupSramL1B
 * 
 * @return Length
 *//*********************************************************************/
inline uint32 OscSupSramL1BLen();

/*********************************************************************//*!
 * @brief Get the pointer to the available L1 SRAM in block B
 * 
 * Host: This is implemented by a temporary memory area which is not
 * really faster than any other.
 * @see OscSupSramL1BLen
 * 
 * @return Pointer to L1B.
 *//*********************************************************************/
inline void* OscSupSramL1B();

/*********************************************************************//*!
 * @brief Query the length of the available L1 SRAM in the Scratchpad
 * 
 * @see OscSupSramScratch
 * 
 * @return Length
 *//*********************************************************************/
inline uint32 OscSupSramScratchLen();

/*********************************************************************//*!
 * @brief Get the pointer to the available L1 SRAM in the Scratchpad
 * 
 * Host: This is implemented by a temporary memory area which is not
 * really faster than any other.
 * @see OscSupSramScratchLen
 * 
 * @return Pointer to the Scratchpad.
 *//*********************************************************************/
inline void* OscSupSramScratch();


/*------------------------------ Cache ---------------------------------*/

/*! @brief the length of a cache line of the Blackfin Prozessor. */
#define CACHE_LINE_LEN 32

#ifdef OSC_TARGET
/*********************************************************************//*!
 * @brief Prefetch the cacheline containing an address.
 * 
 * Host: This does nothing.
 * 
 * @param address Address to be prefetched.
 *//*********************************************************************/
#define PREFETCH(address) \
    asm volatile ("prefetch [%0];\n\t" : : "a" (address))

/*********************************************************************//*!
 * @brief Prefetch the cacheline following the specified address.
 * 
 * Host: This does nothing.
 * 
 * @param address Address before the cache line to be prefetched.
 *//*********************************************************************/
#define PREFETCH_NEXT(address) \
     PREFETCH(((uint32)address) + CACHE_LINE_LEN)

/*********************************************************************//*!
 * @brief Flush the cacheline containing an address.
 * 
 * Host: This does nothing.
 * 
 * @param address Address to be flushed.
 *//*********************************************************************/
#define FLUSH(address) \
    asm volatile ("flush [%0];\n\t" : : "a" (address))

/*********************************************************************//*!
 * @brief Flush and invalidate the cacheline containing an address.
 * 
 * Host: This does nothing.
 * 
 * @param address Address to be flushed and invalidated.
 *//*********************************************************************/
#define FLUSHINV(address) \
    asm volatile ("flushinv [%0];\n\t" : : "a" (address))

/*********************************************************************//*!
 * @brief Flush the cacheline containing an address in instruction cache.
 * 
 * Host: This does nothing.
 * 
 * @param address Address to be flushed.
 *//*********************************************************************/
#define IFLUSH(address) \
    asm volatile ("iflush [%0];\n\t" : : "a" (address))

/*********************************************************************//*!
 * @brief Flush and invalidate a whole memory region.
 * 
 * This macro assumes the start address to be 32 byte aligned. If it
 * is not, add an additional 32 byte to the size.
 * 
 * Host: This does nothing.
 * 
 * @param address Start address of memory region to be flushed and 
 * invalidated.
 * @param size Length of the memory region.
 *//*********************************************************************/
#define FLUSHINV_REGION(address,size) \
        __asm__ __volatile__(   \
        "[--SP]=%0;"            \
        "%0+=31;"               \
        "%0>>=5;"               \
        "p0=%0;"                \
        "p1=%1;"                \
        "lsetup (1f,1f) LC1=P0\n" \
        "1:\n"                  \
        "flushinv [P1++]\n"     \
        "2:\n"                  \
        "%0=[SP++]\n"           \
        :                       \
        : "d" (size), "a" (address) \
        : "LC1", "LT1", "LB1", "P0", "P1" )

/*********************************************************************//*!
 * @brief Flush a whole memory region.
 * 
 * This macro assumes the start address to be 32 byte aligned. If it
 * is not, add an additional 32 byte to the size.
 * 
 * Host: This does nothing.
 * 
 * @param address Start address of memory region to be flushed.
 * @param size Length of the memory region.
 *//*********************************************************************/
#define FLUSH_REGION(address,size) \
        __asm__ __volatile__(   \
        "[--SP]=%0;"            \
        "%0+=31;"               \
        "%0>>=5;"               \
        "p0=%0;"                \
        "p1=%1;"                \
        "lsetup (1f,1f) LC1=P0\n" \
        "1:\n"                  \
        "flush [P1++]\n"        \
        "2:\n"                  \
        "%0=[SP++]\n"           \
        :                       \
        : "d" (size), "a" (address) \
        : "LC1", "LT1", "LB1", "P0", "P1" )

#endif /* OSC_TARGET */

#ifdef OSC_HOST
#define PREFETCH(address)                   {}
#define PREFETCH_NEXT(address)              {}
#define FLUSH(address)                      {}
#define FLUSHINV(address)                   {}
#define IFLUSH(address)                     {}
#define FLUSHINV_REGION(address, size)      {}
#define FLUSH_REGION(address, size)         {}
#endif /* OSC_HOST */

/*------------------------ Instruction Flow ----------------------------*/
#ifdef OSC_TARGET
/*! @brief Wait until all previous actions have been completed
 * (system-wide) 
 * 
 * Host: This does nothing
 */
#define SSYNC asm("ssync;\n")
/*! @brief Wait until all previous actions have been completed
 * (core-wide) 
 * 
 * Host: This does nothing
 */
#define CSYNC asm("csync;\n")
#endif /* OSC_TARGET */

#ifdef OSC_HOST
#define SSYNC {}
#define CSYNC {}
#endif /* OSC_HOST */

/*------------------------ Memory copy ----------------------------*/
#ifdef OSC_TARGET
/*********************************************************************//*!
 * @brief Copy a memory region with the CPU.
 * 
 * Copies the data of length count*4 bytes from src to dst. Thus
 * only works with lengths that are a multiple of 4 bytes.
 * 
 * @param pDst Destination address of copy operation.
 * @param pSrc Source address of copy operation.
 * @param count Number of 4-byte words to copy.
 *//*********************************************************************/
#define memcpy_inline(pDst,pSrc,count)      \
        asm(                                \
        "I0=%1;"                            \
        "P0=%0;"                            \
        "R0=[I0++];"                        \
        "LSETUP (1f,1f) LC1=%2;\n"          \
        "1:\n"                              \
        "mnop || [P0++]=R0 || R0=[I0++];"   \
        "[P0++]=R0;"                        \
        :                                   \
        :"a" (pDst), "a" (pSrc), "a" (count-1) \
        : "LC1","LT1","LB1", "I0","P0", "R0")
#endif /* OSC_TARGET */

#ifdef OSC_HOST
#define memcpy_inline(pDst,pSrc,count)      \
{                                           \
    int i;                                  \
    uint32 *pS = (uint32*)pSrc;             \
    uint32 *pD = (uint32*)pDst;             \
    for(i=0; i < count; i++)                \
    {                                       \
        *pD++ = *pS++;                      \
    }                                       \
}
#endif /* OSC_HOST */

/*------------------------------ LED ---------------------------------*/
/*********************************************************************//*!
 * @brief Initialize the on-board LED of the Industrial OSC platform.
 * 
 * Target only. Industrial OSC only. 
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscSupLedInit();

/*********************************************************************//*!
 * @brief Set the state of the LED.
 * 
 * Target only. Industrial OSC only. 
 * 
 * @param val   '1': on
 *              '0': off
 *              'T': toggle
 *//*********************************************************************/
inline void OscSupLedWrite(char val);

/*********************************************************************//*!
 * @brief Close the driver of the LED.
 * 
 * Target only. Industrial OSC only. 
 *//*********************************************************************/
void OscSupLedClose();

#endif /* SUP_PUB_H_ */

