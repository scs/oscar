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

/*! @file sup_pub.h
 * @brief API definition for support module.
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
 * the functions are implemented for compatibility but they do not exert
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
void OscSupWdtKeepAlive();

/*---------------------------- Cycles ----------------------------------*/

/*********************************************************************//*!
 * @brief Get the current 32 bit cycle count to measure time.
 * 
 * Target: Reads out the current cycle count from the blackfin hardware
 * register and returns the lower 32 bit.
 * 
 * Host: Get the cycle count from the ANSI C function clock(). The time
 * resolution is worse with this method, but it is compatible among a
 * wide selection of platforms.
 * @see OscSupCycGet64
 * @see OscSupCycToMicroSecs
 * @see OscSupCycToMilliSecs
 * @see OscSupCycToSecs
 * 
 * @return The current cycle count (wraps).
 *//*********************************************************************/
uint32 OscSupCycGet();

/*********************************************************************//*!
 * @brief Get the current 64 bit cycle count to measure time.
 * 
 * Target: Reads out the current cycle count from the blackfin hardware
 * register.
 * 
 * Host: Get the cycle count from the ANSI C function clock(). The time
 * resolution is worse with this method, but it is compatible among a
 * wide selection of platforms.
 * @see OscSupCycGet
 * @see OscSupCycToMicroSecs64
 * @see OscSupCycToMilliSecs64
 * @see OscSupCycToSecs64
 *
 * @return The current cycle count (wraps).
 *//*********************************************************************/
long long OscSupCycGet64();

/*********************************************************************//*!
 * @brief Convert a 32-bit cycle count to microseconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * @see OscSupCycGet
 * @see OscSupCycToMilliSecs
 * @see OscSupCycToSecs
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
uint32 OscSupCycToMicroSecs(uint32 cycles);

/*********************************************************************//*!
 * @brief Convert a 32-bit cycle count to milliseconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * @see OscSupCycGet
 * @see OscSupCycToMicroSecs
 * @see OscSupCycToSecs
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
uint32 OscSupCycToMilliSecs(uint32 cycles);

/*********************************************************************//*!
 * @brief Convert a 32-bit cycle count to seconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * @see OscSupCycGet
 * @see OscSupCycToMicroSecs
 * @see OscSupCycToMilliSecs
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
uint32 OscSupCycToSecs(uint32 cycles);

/*********************************************************************//*!
 * @brief Convert a 64-bit cycle count to microseconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * @see OscSupCycGet64
 * @see OscSupCycToMilliSecs64
 * @see OscSupCycToSecs64
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
long long OscSupCycToMicroSecs64(long long cycles);

/*********************************************************************//*!
 * @brief Convert a 64-bit cycle count to milliseconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * @see OscSupCycGet64
 * @see OscSupCycToMicroSecs64
 * @see OscSupCycToSecs64
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
long long OscSupCycToMilliSecs64(long long cycles);

/*********************************************************************//*!
 * @brief Convert a 64-bit cycle count to seconds.
 * 
 * Use to get a real time measure of time passed by supplying the
 * difference between to cycle counts.
 * @see OscSupCycGet64
 * @see OscSupCycToMicroSecs64
 * @see OscSupCycToMilliSecs64
 * 
 * @param cycles Cycle count to be converted.
 * @return Microseconds corresponding to supplied cycle count.
 *//*********************************************************************/
long long OscSupCycToSecs64(long long cycles);

/*------------------------------ SRAM ----------------------------------*/

/*********************************************************************//*!
 * @brief Allocate a chunk of L1 Data SRAM from block A
 * 
 * Memory is allocated continuously and the call fails if no continuous
 * chunk of the specified length can be found. Redirects to sram_alloc
 * for target and malloc for host.
 * 
 * @see OscSupSramFree
 * 
 * @param Number of bytes to allocate.
 * @return Pointer to allocated memory area or NULL on failure.
 *//*********************************************************************/
void* OscSupSramAllocL1DataA(unsigned int size);

/*********************************************************************//*!
 * @brief Allocate a chunk of L1 Data SRAM from block B
 * 
 * Memory is allocated continuously and the call fails if no continuous
 * chunk of the specified length can be found. Redirects to sram_alloc
 * for target and malloc for host.
 * 
 * @see OscSupSramFree
 * 
 * @param Number of bytes to allocate.
 * @return Pointer to allocated memory area or NULL on failure.
 *//*********************************************************************/
void* OscSupSramAllocL1DataB(unsigned int size);

/*********************************************************************//*!
 * @brief Allocate a chunk of L1 Data SRAM from block A or B
 * 
 * Memory is allocated continuously and the call fails if no continuous
 * chunk of the specified length can be found. Redirects to sram_alloc
 * for target and malloc for host.
 * 
 * @see OscSupSramFree
 * 
 * @param Number of bytes to allocate.
 * @return Pointer to allocated memory area or NULL on failure.
 *//*********************************************************************/
void* OscSupSramAllocL1Data(unsigned int size);

/*********************************************************************//*!
 * @brief Allocate a chunk of L1 Instruction SRAM
 * 
 * Memory is allocated continuously and the call fails if no continuous
 * chunk of the specified length can be found. Redirects to sram_alloc
 * for target and malloc for host.
 * 
 * @see OscSupSramFree
 * 
 * @param Number of bytes to allocate.
 * @return Pointer to allocated memory area or NULL on failure.
 *//*********************************************************************/
void* OscSupSramAllocL1Instr(unsigned int size);

/*********************************************************************//*!
 * @brief Allocate a chunk of Scratchpad SRAM
 * 
 * Memory is allocated continuously and the call fails if no continuous
 * chunk of the specified length can be found. Redirects to sram_alloc
 * for target and malloc for host.
 * 
 * @warning Currently not supported by the HEAD build of uclinux; a feature
 * request has been submitted though and hopefully it is being incorporated
 * soon.
 * 
 * @see OscSupSramFree
 * 
 * @param Number of bytes to allocate.
 * @return Pointer to allocated memory area or NULL on failure.
 *//*********************************************************************/
void* OscSupSramAllocScratch(unsigned int size);

/*********************************************************************//*!
 * @brief Free a previously allocated chunk of any SRAM memory
 * 
 * Redirects to sram_free for target and free for host.
 * 
 * @param Pointer to memory area to be freed.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
int OscSupSramFree(void *pAddr);

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


#endif /* SUP_PUB_H_ */

