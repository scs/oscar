/*! @file dma_target.c
 * @brief Memory DMA module implementation for target
 * 
 * @author Markus Berner
 */

#include "framework_types_target.h"

#include "dma_pub.h"
#include "dma_priv.h"
#include "framework_intern.h"

#include "sup/sup_pub.h"
#include "log/log_pub.h"

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
#define LCVDmaRaiseException(srcDesc, dstDesc, config) \
   asm volatile  ("excpt 0xd;\n\t" : :          \
    "q0" (srcDesc),                             \
    "q1" (dstDesc),                             \
    "q2" (config))

void LCVDmaStart(void *hChainHandle)
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
    LCVDmaRaiseException(&pChain->arySrcDesc[0], 
            &pChain->aryDstDesc[0],
            config);  
}

LCV_ERR LCVDmaSync(void *hChainHandle)
{
    struct DMA_CHAIN *pChain = (struct DMA_CHAIN*)hChainHandle;
   
#ifdef DMA_TIMEOUT_WORKAROUND
    uint32 startCyc = LCVSupCycGet();
    
    while(LCVSupCycToMicroSecs(LCVSupCycGet() - startCyc) < DMA_TIMEOUT)
    {
        FLUSHINV(&pChain->syncFlag);
        if(pChain->syncFlag != 0)
            return SUCCESS;
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
    }
    return -ETIMEOUT;
#endif /* DMA_TIMEOUT_WORKAROUND */
}
