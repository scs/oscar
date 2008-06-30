/*! @file swr_target.c
 * @brief Stimuli writer module implementation for target.
 * 
 ************************************************************************/

#include "oscar_types_target.h"

#include "swr_pub.h"
#include "swr_priv.h"
#include "oscar_intern.h"

struct OSC_SWR swr;		/*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY swr_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscSwrCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;
    
    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->swr.useCnt != 0)
    {
        pFw->swr.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module swr_deps of this module. */
    err = OSCLoadDependencies(pFw, 
            swr_deps, 
            sizeof(swr_deps)/sizeof(struct OSC_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load swr_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
 	memset(&swr, 0, sizeof(struct OSC_SWR));
	
    /* Increment the use count */
    pFw->swr.hHandle = (void*)&swr;
    pFw->swr.useCnt++;  
    
    return SUCCESS;
}

void OscSwrDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
        
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->swr.useCnt--;
    if(pFw->swr.useCnt > 0)
    {
        return;
    }
    
    OSCUnloadDependencies(pFw, 
            swr_deps, 
            sizeof(swr_deps)/sizeof(struct OSC_DEPENDENCY));
    
	memset(&swr, 0, sizeof(struct OSC_SWR));
}

