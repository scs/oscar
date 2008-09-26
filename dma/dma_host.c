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

/*! @file dma_host.c
 * @brief Memory DMA module implementation for host
 * 
 */

#include "oscar_types_host.h"

#include "dma_pub.h"
#include "dma_priv.h"
#include "oscar_intern.h"

#include "sup/sup_pub.h"
#include "log/log_pub.h"


uint8 OscDmaExtractWdSize(const uint16 dmaConfigMask)
{
	/* Determine source descriptor word size. */
	if(dmaConfigMask & WDSIZE_16)
	{
		return 2;
	}
	else if(dmaConfigMask & WDSIZE_32)
	{
		return 4;
	}
	else
	{
		/* 8 bit word size. */
		return 1;
	}
}

OSC_ERR OscDmaChanCopy(const struct DMA_DESC *pDesc,
		void *pTemp,
		const uint8 wdSize)
{
	uint32 x, y;
	
	/* Have pointers for all possible word sizes. */
	uint8 *pRd8 = pTemp;
	uint8 *pWr8 = pTemp;
	
	uint16 *pRd16 = pTemp;
	uint16 *pWr16 = pTemp;
	
	uint32 *pRd32 = pTemp;
	uint32 *pWr32 = pTemp;
	
	int16 rdModX = 1;
	int16 rdModY = 1;
	int16 wrModX = 1;
	int16 wrModY = 1;
	void * pAddr;
	
	
	pAddr = (void*)(((uint32)pDesc->startAddrLow) |
			(((uint32)pDesc->startAddrHigh) << 16));
	
	if(pDesc->config & WNR)
	{
		/* Write channel. */
		wrModX = pDesc->xModify/wdSize;
		wrModY = pDesc->yModify/wdSize;
		pWr8 = (uint8*)pAddr;
		pWr16 = (uint16*)pAddr;
		pWr32 = (uint32*)pAddr;
	}
	else
	{
		/* Read channel. */
		rdModX = pDesc->xModify/wdSize;
		rdModY = pDesc->yModify/wdSize;
		pRd8 = (uint8*)pAddr;
		pRd16 = (uint16*)pAddr;
		pRd32 = (uint32*)pAddr;
	}
			
	/* The actual copy loop. Not very fast but it works just like the
	 * blackfin DMA for maximum compability. */
	switch(wdSize)
	{
	case 1:
		/* 8 bit transfers. */
		for(y = 0; y < pDesc->yCount; y++)
		{
			for(x = 0; x < pDesc->xCount; x++)
			{
				*pWr8 = *pRd8;
				pWr8 += wrModX;
				pRd8 += rdModX;
			}
			pWr8 -= wrModX;
			pRd8 -= rdModX;
			
			pWr8 += wrModY;
			pRd8 += rdModY;
		}
		break;
	case 2:
		/* 16 bit transfers. */
		for(y = 0; y < pDesc->yCount; y++)
		{
			for(x = 0; x < pDesc->xCount; x++)
			{
				*pWr16 = *pRd16;
				pWr16 += wrModX;
				pRd16 += rdModX;
			}
			pWr16 -= wrModX;
			pRd16 -= rdModX;
						
			pWr16 += wrModY;
			pRd16 += rdModY;
		}
		break;
	case 4:
		/* 32 bit transfers. */
		for(y = 0; y < pDesc->yCount; y++)
		{
			for(x = 0; x < pDesc->xCount; x++)
			{
				*pWr32 = *pRd32;
				pWr32 += wrModX;
				pRd32 += rdModX;
			}
			pWr32 -= wrModX;
			pRd32 -= rdModX;
			
			pWr32 += wrModY;
			pRd32 += rdModY;
		}
		break;
	default:
		OscLog(ERROR, "%s: Invalid word size: %d\n", __func__, wdSize);
		return -EINVALID_PARAMETER;
	}
	return SUCCESS;
}

void OscDmaStart(void *hChainHandle)
{
	OSC_ERR err;
	uint32 srcDataSize;
	uint8  srcWdSize, dstWdSize;
	uint16 move;
	void* *pTempBuffer;
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;
	struct DMA_DESC  *pSrcDesc;
	struct DMA_DESC  *pDstDesc;
	
	/* Execute the DMA chain one move after the other. */
	for(move = 0; move < pChain->nMoves; move++)
	{
		pSrcDesc = &pChain->arySrcDesc[move];
		pDstDesc = &pChain->aryDstDesc[move];
		
		/* Find the word sizes. */
		srcWdSize = OscDmaExtractWdSize(pSrcDesc->config);
		dstWdSize = OscDmaExtractWdSize(pDstDesc->config);
		if(srcWdSize == 0 || dstWdSize == 0)
		{
			OscLog(ERROR, "%s: Invalid word sizes configured!\n",
					__func__);
			return;
		}
		
		/* Find the data size of the source channel. */
		srcDataSize = srcWdSize * pSrcDesc->xCount;
		if(pSrcDesc->config & DMA2D)
		{
			srcDataSize *= pSrcDesc->yCount;
		}
		
		/* Allocate a temporary buffer with enough space for all
		 * data read, acting as FIFO. */
		pTempBuffer = (void*)malloc(srcDataSize);
		if(pTempBuffer == NULL)
		{
			OscLog(ERROR, "%s: Memory allocation failed!\n", __func__);
			return;
		}
		
		/* The actual copy action. The reason we copy all data twice is
		 * to be able to support all the 2D transfer options of the
		 * Blackfin DMA. The DMA has an internal FIFO which acts as bridge
		 * between the source and destination channels. On the host we
		 * emulate this FIFO with the temporary buffer. */
		
		/* Copy all the source data to the temporary buffer. */
		err = OscDmaChanCopy(pSrcDesc, pTempBuffer, srcWdSize);
		if(err != SUCCESS)
		{
			OscLog(ERROR, "%s: Source channel operation failed! (%d)\n",
					__func__, err);
			free(pTempBuffer);
			return;
		}
		/* Copy all the data from the temporary buffer to the destination.
		 * */
		OscDmaChanCopy(pDstDesc, pTempBuffer, dstWdSize);
		if(err != SUCCESS)
		{
			OscLog(ERROR, "%s: Dest channel operation failed! (%d)\n",
						__func__, err);
			
			free(pTempBuffer);
			return;
		}
		free(pTempBuffer);
	}
}

OSC_ERR OscDmaSync(void *hChainHandle)
{
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;
	
	if(pChain == NULL)
	{
		return -EINVALID_PARAMETER;
	}
	/* Since the "DMA" transfers happen instantaneously on the host
	 * we just need to check whether it succeeded. */
	if(pChain->syncFlag != 0)
	{
		return SUCCESS;
	}

	return -ETIMEOUT;
}

