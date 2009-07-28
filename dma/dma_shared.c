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
 * @brief Memory DMA module implementation shared by target and host.
 */

#include "dma.h"

OSC_ERR OscDmaCreate();

/*! @brief The module singelton instance. */
struct OSC_DMA dma;

/*! @brief Data field filled with only ones. */
static uint32 allOnes = 0xffffffff;

struct OscModule OscModule_dma = {
	.name = "dma",
	.create = OscDmaCreate,
	.dependencies = {
		&OscModule_log,
		&OscModule_sup,
		NULL // To end the flexible array.
	}
};

OSC_ERR OscDmaCreate()
{
	dma = (struct OSC_DMA) { };
	
	return SUCCESS;
}

inline void OscDmaResetChain(void *hChainHandle)
{
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;

	if(unlikely(hChainHandle == NULL))
	{
	    OscLog(ERROR, "%s: Invalid parameter!\n");
	    return;
	}
	pChain->nMoves = 0;
}

OSC_ERR OscDmaAllocChain(void **phChainHandle)
{
	struct DMA_CHAIN ** pChain;
	
	if(unlikely(phChainHandle == NULL))
	{
		return -EINVALID_PARAMETER;
	}

	if(unlikely(dma.nChainsAllocated == MAX_NR_DMA_CHAINS))
	{
		OscLog(WARN, "%s: All DMA chains already allocated!\n",
				__func__);
		*phChainHandle = NULL;
		return -EDMA_NO_MORE_CHAINS_AVAILABLE;
	}
	
	pChain = (struct DMA_CHAIN**)phChainHandle;
	*pChain = &dma.dmaChains[dma.nChainsAllocated];
	dma.nChainsAllocated++;
	
	/* Reset the chain, just to be sure. */
	OscDmaResetChain(*pChain);
	
	return SUCCESS;
}

OSC_ERR OscDmaAdd2DMove(void *hChainHandle,
		const void *pDstAddr,
		const enum EnDmaWdSize enDstWdSize,
		const uint16 dstXCount, const int32 dstXModify,
		const uint16 dstYCount, const int32 dstYModify,
		const void *pSrcAddr,
		const enum EnDmaWdSize enSrcWdSize,
		const uint16 srcXCount, const int32 srcXModify,
		const uint16 srcYCount, const int32 srcYModify)
{
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;
	struct DMA_DESC  *pSrcDesc = &pChain->arySrcDesc[pChain->nMoves];
	struct DMA_DESC  *pDstDesc = &pChain->aryDstDesc[pChain->nMoves];
	
	if(unlikely(pChain->nMoves == MAX_MOVES_PER_CHAIN))
	{
		return -EINVALID_PARAMETER;
	}
	pChain->nMoves++;
	
	/************* Configure destination channel descriptor *************/
	/* Enable DMA, Use desc array mode, Size of next descriptor, Write */
	pDstDesc->config = NDSIZE | FLOW_ARRAY | DMAEN | WNR;
	/* Use the supplied word size (8/16/32 bit) */
	pDstDesc->config |= enDstWdSize;
	
	pDstDesc->startAddrLow = (uint16)((uint32)pDstAddr & 0x0000FFFF);
	pDstDesc->startAddrHigh = (uint16)((uint32)pDstAddr >> 16);
	
	pDstDesc->xCount = dstXCount;
	pDstDesc->xModify = dstXModify;
	pDstDesc->yCount = dstYCount;
	pDstDesc->yModify = dstYModify;
	if(dstYCount > 1)
	{
		pDstDesc->config |= DMA2D;
	}
	
	/************* Configure source channel descriptor *************/
	/* Enable DMA, Use desc array mode, Size of next descriptor */
	pSrcDesc->config = NDSIZE | FLOW_ARRAY | DMAEN;
	/* Use the supplied word size (8/16/32 bit) */
	pSrcDesc->config |= enSrcWdSize;
	
	pSrcDesc->startAddrLow = (uint16)((uint32)pSrcAddr & 0x0000FFFF);
	pSrcDesc->startAddrHigh = (uint16)((uint32)pSrcAddr >> 16);
	
	pSrcDesc->xCount = srcXCount;
	pSrcDesc->xModify = srcXModify;
	pSrcDesc->yCount = srcYCount;
	pSrcDesc->yModify = srcYModify;
	if(srcYCount > 1)
	{
		pSrcDesc->config |= DMA2D;
	}
	
	/* Flush the descriptors, otherwise they are only updated in cache
	 * and the DMA can not read them. Add a cache line length to the
	 * size because they're not 32 Byte aligned. */
	FLUSH_REGION(pSrcDesc, sizeof(struct DMA_DESC) + CACHE_LINE_LEN);
	FLUSH_REGION(pDstDesc, sizeof(struct DMA_DESC) + CACHE_LINE_LEN);
	return SUCCESS;
}

inline OSC_ERR OscDmaAdd1DMove(void *hChainHandle,
		const void *pDstAddr,
		const enum EnDmaWdSize enDstWdSize,
		const uint16 dstCount, const int32 dstModify,
		const void *pSrcAddr,
		const enum EnDmaWdSize enSrcWdSize,
		const uint16 srcCount, const int32 srcModify)
{
	return OscDmaAdd2DMove(hChainHandle,
			pDstAddr,
			enDstWdSize,
			dstCount, dstModify,
			1, 4,
			pSrcAddr,
			enSrcWdSize,
			srcCount, srcModify,
			1, 4);
}

OSC_ERR OscDmaAddSyncPoint(void *hChainHandle)
{
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;
	struct DMA_DESC  *pSrcDesc = &pChain->arySrcDesc[pChain->nMoves];
	struct DMA_DESC  *pDstDesc = &pChain->aryDstDesc[pChain->nMoves];
	uint32 allOnesAddr = (uint32)&allOnes;
	uint32 syncFlagAddr = (uint32)&pChain->syncFlag;
	
	if(unlikely(pChain->nMoves > MAX_MOVES_PER_CHAIN))
	{
		return -EINVALID_PARAMETER;
	}
	pChain->nMoves++;


	/************* Configure destination channel descriptor *************/
	/* Enable DMA,  Write */
	pDstDesc->config = DMAEN | WNR;
	/* Set the word size to the word size of the previous descriptor. */
	if(likely(pChain->nMoves > 1))
	{
		pDstDesc->config |=
			(pChain->aryDstDesc[pChain->nMoves - 2].config & WDSIZE_MASK);
	} else {
		pDstDesc->config |= WDSIZE_32;
	}

	/* Write to the stop Flag, so we can poll it until the DMA is
	 * finished. */
	pDstDesc->startAddrLow = (uint16)(syncFlagAddr & 0x0000FFFF);
	pDstDesc->startAddrHigh = (uint16)(syncFlagAddr >> 16);

	pDstDesc->xCount = 1;
	pDstDesc->xModify = 4;
	pDstDesc->yCount = 1;
	pDstDesc->yModify = 4;

	/************* Configure source channel descriptor *************/
	/* Enable DMA */
	pSrcDesc->config = DMAEN;
	/* Set the word size to the word size of the previous descriptor. */
	if(likely(pChain->nMoves > 1))
	{
		pSrcDesc->config |=
			(pChain->arySrcDesc[pChain->nMoves - 2].config & WDSIZE_MASK);
	} else {
		pSrcDesc->config |= WDSIZE_32;
	}

	/* Read some non-NULL value. */
	pSrcDesc->startAddrLow = (uint16)(allOnesAddr & 0x0000FFFF);
	pSrcDesc->startAddrHigh = (uint16)(allOnesAddr >> 16);

	pSrcDesc->xCount = 1;
	pSrcDesc->xModify = 4;
	pSrcDesc->yCount = 1;
	pSrcDesc->yModify = 4;

	/* Flush the descriptors, otherwise they are only updated in cache
	 * and the DMA can not read them. */
	FLUSH_REGION(pSrcDesc, sizeof(struct DMA_DESC) + CACHE_LINE_LEN);
	FLUSH_REGION(pDstDesc, sizeof(struct DMA_DESC) + CACHE_LINE_LEN);
	return SUCCESS;
}

OSC_ERR OscDmaMemCpy(void *hChainHandle,
		void *pDstAddr,
		void *pSrcAddr,
		uint32 len)
{
	OSC_ERR err;
	/* Add the move to the chain (effectively a 1D move with 32 bit word
	 * length. */
	err = OscDmaAdd2DMove(hChainHandle,
			pDstAddr,
			DMA_WDSIZE_32,
			(len >> 2), 4,
			1, 4,
			pSrcAddr,
			DMA_WDSIZE_32,
			(len >> 2), 4,
			1, 4);
	
	if(unlikely(err != SUCCESS))
	{
		OscLog(ERROR, "%s: Unable to add move!\n", __func__);
		return err;
	}
	
	/* Start the transfer. */
	OscDmaStart(hChainHandle);
	
	return SUCCESS;
}

OSC_ERR OscDmaMemCpySync(void *hChainHandle,
		void *pDstAddr,
		void *pSrcAddr,
		uint32 len)
{
	OSC_ERR err;
	/* Add the move to the chain (effectively a 1D move with 32 bit word
	 * length xModify = 4 and xCount = len/4). */
	err = OscDmaAdd2DMove(hChainHandle,
			pDstAddr,
			DMA_WDSIZE_32,
			(len >> 2), 4,
			1, 4,
			pSrcAddr,
			DMA_WDSIZE_32,
			(len >> 2), 4,
			1, 4);
	
	if(unlikely(err != SUCCESS))
	{
		OscLog(ERROR, "%s: Unable to add move!\n", __func__);
		return err;
	}
	
	/* Add a sync point so we can wait for it later. */
	err = OscDmaAddSyncPoint(hChainHandle);
	if(unlikely(err != SUCCESS))
	{
		OscLog(ERROR, "%s: Unable to add synchronizaion point!\n",
				__func__);
		return err;
	}

	/* Start the transfer. */
	OscDmaStart(hChainHandle);
	
	/* Wait for the transfer to be finished. */
	err = OscDmaSync(hChainHandle);
	
	return err;
}

