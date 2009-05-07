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
 * @brief API definition for Memory DMA module
 * 
 */
#ifndef DMA_PUB_H_
#define DMA_PUB_H_

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscDmaErrors {
	EDMA_NO_MORE_CHAINS_AVAILABLE = OSC_DMA_ERROR_OFFSET
};

/*! @brief The number of bits the DMA transfers with one operation. */
enum EnDmaWdSize
{
	/*! @brief DMA channel option: Word size  8 bits */
	DMA_WDSIZE_8 = 0x00000000,
	/*! @brief DMA channel option: Word size 16  bits */
	DMA_WDSIZE_16 = 0x00000004,
	/*! @brief DMA channel option: Word size 32 bits */
	DMA_WDSIZE_32 = 0x00000008
};

/*=========================== API functions ============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscDmaCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscDmaDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Allocate a new, re-usable DMA chain handle.
 * 
 * @param phChainHandle Pointer to where the DMA chain handle should be
 * returned.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscDmaAllocChain(void **phChainHandle);

/*********************************************************************//*!
 * @brief Reset a DMA chain (delete all moves pending).
 * 
 * @param hChainHandle Handle to the DMA chain to be reset.
 *//*********************************************************************/
void OscDmaResetChain(void *hChainHandle);

/*********************************************************************//*!
 * @brief Add a 2D memory move to a DMA chain.
 * 
 * This schedules the move of a 2D memory block in the DMA chain. If
 * the DMA chain already contains moves, it will be appended. There is
 * a limited number of moves supported in a chain. The transfer will not
 * start until OscDmaStart() has been called.
 * All moves in a chain must have the same word size!
 * @see OscDmaStart
 * 
 * @param hChainHandle Handle to the DMA chain to add this move to.
 * @param pDstAddr The starting address at which writing will begin.
 * @param enDstWdSize The word size transferred per DMA cycle on the
 * writing end.
 * @param dstXCount The number of words in the X direction of the
 * destination memory block.
 * @param dstXModify The offset the write address is modified with
 * after each line.
 * @param dstYCount The number of words in the Y direction of the
 * destination memory block.
 * @param dstYModify The offset the write address is modified with
 * after each word.
 * @param pSrcAddr The starting address at which reading will begin.
 * @param enSrcWdSize The word size transferred per DMA cycle on the
 * writing end.
 * @param srcXCount The number of words in the X direction of the
 * source memory block.
 * @param srcXModify The offset the read address is modified with
 * after each word.
 * @param srcYCount The number of words in the Y direction of the
 * source memory block.
 * @param srcYModify The offset the read address is modified with
 * after each line.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscDmaAdd2DMove(void *hChainHandle,
		const void *pDstAddr,
		const enum EnDmaWdSize enDstWdSize,
		const uint16 dstXCount, const int32 dstXModify,
		const uint16 dstYCount, const int32 dstYModify,
		const void *pSrcAddr,
		const enum EnDmaWdSize enSrcWdSize,
		const uint16 srcXCount, const int32 srcXModify,
		const uint16 srcYCount, const int32 srcYModify);

/*********************************************************************//*!
 * @brief Add a 1D memory move to a DMA chain.
 * 
 * This schedules the move of a 1D memory block in the DMA chain. If
 * the DMA chain already contains moves, it will be appended. There is
 * a limited number of moves supported in a chain. The transfer will not
 * start until OscDmaStart() has been called.
 * All moves in a chain must have the same word size!
 * @see OscDmaStart
 * 
 * @param hChainHandle Handle to the DMA chain to add this move to.
 * @param pDstAddr The starting address at which writing will begin.
 * @param enDstWdSize The word size transferred per DMA cycle on the
 * writing end.
 * @param dstCount The number of words in the
 * destination memory block.
 * @param dstModify The offset the write address is modified with
 * after each word.
 * @param pSrcAddr The starting address at which reading will begin.
 * @param enSrcWdSize The word size transferred per DMA cycle on the
 * writing end.
 * @param srcCount The number of words in the the
 * source memory block.
 * @param srcModify The offset the read address is modified with
 * after each word.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscDmaAdd1DMove(void *hChainHandle,
		const void *pDstAddr,
		const enum EnDmaWdSize enDstWdSize,
		const uint16 dstCount, const int32 dstModify,
		const void *pSrcAddr,
		const enum EnDmaWdSize enSrcWdSize,
		const uint16 srcCount, const int32 srcModify);

/*********************************************************************//*!
 * @brief Add a a synchronization point to the DMA chain.
 * 
 * Synchronization points allow waiting for the DMA until all data has
 * really been written, as opposed to just querying the current
 * destination address, which is inexact because of the internal FIFOs.
 * A synchronization point is usually added to the end of a DMA chain
 * to be able to find out when it is finished. Behaviour is undefined
 * if no sync point is added to the end of a DMA chain.
 * @see OscDmaSync
 * 
 * @param hChainHandle Handle to the DMA chain.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDmaAddSyncPoint(void *hChainHandle);

/*********************************************************************//*!
 * @brief Start the moves associated with a previously prepared DMA chain.
 * 
 * Starts the actual transfer. If a synchronization point has been added,
 * one can wait for the transfer to be finished by calling OscDmaSync.
 * 
 * Host: DMA transfer is emulated in software and happens in this function
 * call.
 * 
 * @see OscDmaAddSyncPoint
 * @see OscDmaSync
 * 
 * @param hChainHandle Handle to the DMA chain.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
void OscDmaStart(void *hChainHandle);

/*********************************************************************//*!
 * @brief Synchronize to the end of a DMA move chain.
 * 
 * Waits until the DMA has finished a previously started move chain. This
 * only works if a synchronization point has been added to the end of
 * the chain with OscDmaAddSyncPoint. To prevent total lockups due to
 * a DMA malfunction, an improvised timeout was added. The polling loop
 * is only cycled a predefined number of times after which the function
 * returns with -ETIMEOUT (> 8 seconds).
 * All moves in a chain must have the same word size or the DMA will
 * lock up on the target!
 * 
 * Host: Nothing to wait on, just check whether it has been successful.
 * 
 * @see OscDmaAddSyncPoint
 * 
 * @param hChainHandle Handle to the DMA chain.
 * @return SUCCESS or -ETIMEOUT.
 *//*********************************************************************/
OSC_ERR OscDmaSync(void *hChainHandle);

/*********************************************************************//*!
 * @brief Copy a memory area but do not wait for completion.
 * 
 * Only lengths that are multiples of 4 bytes are supported.
 * 
 * @param hChainHandle Handle to the DMA chain.
 * @param pDstAddr Destination address of copy operation.
 * @param pSrcAddr Source address of copy operation.
 * @param len Length of the data to be copied. Must be multiple of 4.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDmaMemCpy(void *hChainHandle,
		void *pDstAddr,
		void *pSrcAddr,
		uint32 len);

/*********************************************************************//*!
 * @brief Copy a memory area and wait for completion.
 * 
 * Only lengths that are multiples of 4 bytes are supported. Do not
 * forget invalidating the target memory area.
 * 
 * @param hChainHandle Handle to the DMA chain.
 * @param pDstAddr Destination address of copy operation.
 * @param pSrcAddr Source address of copy operation.
 * @param len Length of the data to be copied. Must be multiple of 4.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDmaMemCpySync(void *hChainHandle,
		void *pDstAddr,
		void *pSrcAddr,
		uint32 len);

#endif /*DMA_PUB_H_*/
