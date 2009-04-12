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

/*! @file dma_target.c
 * @brief Memory DMA module implementation for target
 * 
 */

#include "oscar_types_target.h"

#include "include/dma.h"
#include "dma.h"
#include "oscar_intern.h"

#include "include/sup.h"
#include "include/log.h"

/*! @brief Raise exception, which will configure and start the DMA.
 * 
 * Target only:
 * The exception handler is part of the mdma kernel module.
 * 
 * @param srcDesc The DMA descriptor of the source channel.
 * @param dstDesc The DMA descriptor of the destination channel.
 * @param config The DMA config words for source and destination, with
 * the source word in the lower 16 bit and the destination word in the
 * higher 16 bit.*/
#define OscDmaRaiseException(srcDesc, dstDesc, config) \
	asm volatile  ("excpt 0xd;\n\t" : :          \
	"q0" (srcDesc),                             \
	"q1" (dstDesc),                             \
	"q2" (config))

void OscDmaStart(void *hChainHandle)
{
	uint32 config;
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;

	/* Set the stop flag to zero */
	pChain->syncFlag = 0;
	FLUSHINV(&pChain->syncFlag);
	
	/* Calculate the config word as a merge of source and destination
	 * config. */
	config = ((uint32)(pChain->aryDstDesc[0].config) << 16) |
		((uint32)(pChain->arySrcDesc[0].config));
	
	/* Raise the exception which will start the DMA in the exception
	 * handler. */
	OscDmaRaiseException(&pChain->arySrcDesc[0],
			&pChain->aryDstDesc[0],
			config);
}

OSC_ERR OscDmaSync(void *hChainHandle)
{
	struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;
	uint32 waitCyc;
	
#ifdef DMA_TIMEOUT_WORKAROUND
	uint32 startCyc = OscSupCycGet();	
	while(OscSupCycToMicroSecs(OscSupCycGet() - startCyc) < DMA_TIMEOUT)
	{
		FLUSHINV(&pChain->syncFlag);
		if(pChain->syncFlag != 0)
			return SUCCESS;
		waitCyc = OscSupCycGet();
		while(OscSupCycGet() - waitCyc < DMA_WAIT_POLL_FREQ_CYCLES)
		{
			asm("nop;");
			asm("nop;");
			asm("nop;");
			asm("nop;");
			asm("nop;");
			asm("nop;");
		}
	}
	printf("DMA timeout!\n");
	return -ETIMEOUT;
#else
	uint32 timeout = DMA_SYNC_TIMEOUT_LOOPS;
	while(timeout--)
	{
		FLUSHINV(&pChain->syncFlag);
		if(pChain->syncFlag != 0)
			return SUCCESS;
		waitCyc = OscSupCycGet();
		while(OscSupCycToMicroSecs(OscSupCycGet() - waitCyc) < DMA_WAIT_POLL_FREQ_CYCLES)
		{
			asm("nop;");
			asm("nop;");
			asm("nop;");
			asm("nop;");
			asm("nop;");
			asm("nop;");
		}
	}
	return -ETIMEOUT;
#endif /* DMA_TIMEOUT_WORKAROUND */
}
