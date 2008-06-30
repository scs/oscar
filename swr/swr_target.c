/*! @file swr_target.c
 * @brief Stimuli writer module implementation for target.
 * 
 * @author Samuel Zahnd
 ************************************************************************/

#include "framework_types_target.h"

#include "swr_pub.h"
#include "swr_priv.h"
#include "framework_intern.h"

struct LCV_SWR swr;		/*!< Module singelton instance */

/*! The dependencies of this module. */
struct LCV_DEPENDENCY swr_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy}
};

LCV_ERR LCVSwrCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;
    
    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->swr.useCnt != 0)
    {
        pFw->swr.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module swr_deps of this module. */
    err = LCVLoadDependencies(pFw, 
            swr_deps, 
            sizeof(swr_deps)/sizeof(struct LCV_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load swr_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
 	memset(&swr, 0, sizeof(struct LCV_SWR));
	
    /* Increment the use count */
    pFw->swr.hHandle = (void*)&swr;
    pFw->swr.useCnt++;  
    
    return SUCCESS;
}

void LCVSwrDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
        
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->swr.useCnt--;
    if(pFw->swr.useCnt > 0)
    {
        return;
    }
    
    LCVUnloadDependencies(pFw, 
            swr_deps, 
            sizeof(swr_deps)/sizeof(struct LCV_DEPENDENCY));
    
	memset(&swr, 0, sizeof(struct LCV_SWR));
}

