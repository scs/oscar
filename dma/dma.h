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
 * @brief Private memory DMA module definition
 * 
 */
#ifndef DMA_PRIV_H_
#define DMA_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "oscar.h"

/*! @brief The maximum number of transfers in a DMA chain. */
#define MAX_MOVES_PER_CHAIN 4
/*! @brief The maximum number of DMA chains that can be allocated. */
#define MAX_NR_DMA_CHAINS 2
/*! @brief The number of loop cycles before OscDmaSync gives up and
 * returns. */
#define DMA_SYNC_TIMEOUT_LOOPS 0xFFFFFFFF

/*! @brief Prevent an infinite stall on waiting for the DMA if it does
 * finish (likely caused by Blackfin anomaly. */
#define DMA_TIMEOUT_WORKAROUND
/*! @brief Milliseconds to wait for the DMA to finish. */
#define DMA_TIMEOUT 20000
/*! @brief The frequency (in CPU cycles) with which the DMA is polled
 * when waiting for it to finish. */ 
#define DMA_WAIT_POLL_FREQ_CYCLES 200

/* Configuration word masks */
/*! @brief DMA channel option: Enable. */
#define DMAEN           0x00000001
/*! @brief DMA channel option: Read or write. */
#define WNR             0x00000002
/*! @brief DMA channel option: Word size  8 bits. */
#define WDSIZE_8        0x00000000
/*! @brief DMA channel option: Word size 16  bits. */
#define WDSIZE_16       0x00000004
/*! @brief DMA channel option: Word size 32 bits. */
#define WDSIZE_32       0x00000008
/*! @brief Bitmask for the wordsize DMA channel option.*/
#define WDSIZE_MASK     0x0000000C
/*! @brief DMA channel option: 2D mode. */
#define DMA2D           0x00000010
/*! @brief DMA channel option: Synchronized execution. This
 * is needed to switch word sizes. */
#define SYNC            0x00000020
/*! @brief DMA channel option: Data interrupt select. */
#define DI_SEL          0x00000040
/*! @brief DMA channel option: Data interrupt enable. */
#define DI_EN           0x00000080
/*! @brief DMA channel option: Size of next descriptor (always 7B). */
#define NDSIZE          0x00000700
/*! @brief DMA channel option: Set flow mode to descriptor array. */
#define FLOW_ARRAY      0x00004000


struct DMA_DESC
{
#ifdef OSC_TARGET
	/*! @brief The low part of the start address for the transfer on the target */
	uint16 startAddrLow;
	/*! @brief The high part of the start address for the transfer on the target */
	uint16 startAddrHigh;
#endif /* OSC_TARGET */
#ifdef OSC_HOST
  /*! @brief The start address for the transfer on the host.
    Needed for 64bit compatibility. */
  uintptr_t startAddr;
#endif /* OSC_HOST */
	/*! @brief DMA channel config */
	uint16 config;
	/*! @brief Number of words along X axis. */
	uint16 xCount;
	/*! @brief Address offset along X axis. */
	int16 xModify;
	/*! @brief Number of words along Y axis. */
	uint16 yCount;
	/*! @brief Address offset along Y axis. */
	int16 yModify;
};

struct DMA_CHAIN
{
	/*! @brief Data field used for waiting until the DMA transfers have
	 *  been completed.
	 * 
	 * This is set to zero at the beginning of a transfer and is
	 * overwritten with ones with the last operation in the DMA chain. */
	uint32 syncFlag;
	/*! @brief The number of data moves currently scheduled in this
	 * chain. */
	uint32 nMoves;
	/*! The source channel descriptors of this chain. */
	struct DMA_DESC arySrcDesc[MAX_MOVES_PER_CHAIN + 1];
	/*! The destination channel descriptors of this chain. */
	struct DMA_DESC aryDstDesc[MAX_MOVES_PER_CHAIN + 1];

};

/*! @brief The object struct of the camera module */
struct OSC_DMA {
	/*! @brief The DMA chains that can be allocated. */
	struct DMA_CHAIN        dmaChains[MAX_NR_DMA_CHAINS];
	/*! @brief The current number of allocated DMA chains. */
	uint16                  nChainsAllocated;
};

/*************************** Private methods ***************************/
#ifdef OSC_HOST

/*********************************************************************//*!
 * @brief Extract the transfer word size from a DMA config word.
 * 
 * @param dmaConfigMask The DMA configuration word.
 * @return Number of bytes transferred in one DMA cycle or 0 if invalid.
 *//*********************************************************************/
uint8 OscDmaExtractWdSize(const uint16 dmaConfigMask);

/*********************************************************************//*!
 * @brief Emulate the copy functionality of a DMA channel.
 * 
 * A DMA transfer is a combined effort of two DMA channels; one to read
 * and one to write. These two are connected over a FIFO in the read
 * hardware, on the host we just use a temporary buffer. This method
 * now either copies the data from the source to the temporary buffer
 * or from the temporary buffer to the destination, depending on the
 * contents of the DMA descriptor.
 * 
 * @param pDesc The DMA descriptor pointer.
 * @param pTemp Pointer to the temporary buffer.
 * @param wdSize Word size used for transfers.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDmaChanCopy(const struct DMA_DESC *pDesc,
		void *pTemp,
		const uint8 wdSize);

#endif /* OSC_HOST */

#endif /*DMA_PRIV_H_*/
