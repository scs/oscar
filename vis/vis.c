/*! @file bmp.c
 * @brief Vision module implementation for target and host
 * 
 * @author Markus Berner
 */

#include "vis_pub.h"
#include "../dma/dma_pub.h"
#include "vis_priv.h"
#include "framework_intern.h"
#include <stdio.h>
#include <string.h>

/*! @brief The module singelton instance. */
struct LCV_VIS vis;     

/*! @brief The dependencies of this module. */
struct LCV_DEPENDENCY vis_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy},
        {"dma", LCVDmaCreate, LCVDmaDestroy}
};


LCV_ERR LCVVisCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;
    
    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->vis.useCnt != 0)
    {
        pFw->vis.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module dependencies of this module. */
    err = LCVLoadDependencies(pFw, 
            vis_deps, 
            sizeof(vis_deps)/sizeof(struct LCV_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load dependencies! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&vis, 0, sizeof(struct LCV_VIS));
    
    /* Increment the use count */
    pFw->vis.hHandle = (void*)&vis;
    pFw->vis.useCnt++;    
    
    return SUCCESS;
}

void LCVVisDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
        
    pFw = (struct LCV_FRAMEWORK *)hFw;
    
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->vis.useCnt--;
    if(pFw->vis.useCnt > 0)
    {
        return;
    }
    
    LCVUnloadDependencies(pFw, 
            vis_deps, 
            sizeof(vis_deps)/sizeof(struct LCV_DEPENDENCY));
    
    memset(&vis, 0, sizeof(struct LCV_VIS));
}

