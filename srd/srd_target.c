/*! @file srd_target.c
 * @brief Stimuli reader module implementation for target.
 * 
 * @author Samuel Zahnd
 ************************************************************************/

#include "framework_types_target.h"

#include "srd_pub.h"
#include "srd_priv.h"
#include "framework_intern.h"

struct LCV_SRD srd;		/*!< Module singelton instance */

/*! The dependencies of this module. */
struct LCV_DEPENDENCY srd_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy}
};

LCV_ERR LCVSrdCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->srd.useCnt != 0)
    {
        pFw->srd.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }

    /* Load the module srd_deps of this module. */
    err = LCVLoadDependencies(pFw, 
            srd_deps, 
            sizeof(srd_deps)/sizeof(struct LCV_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load srd_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
 	memset(&srd, 0, sizeof(struct LCV_SRD));
	
    /* Increment the use count */
    pFw->srd.hHandle = (void*)&srd;
    pFw->srd.useCnt++;  
    
    return SUCCESS;
}

void LCVSrdDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
        
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->srd.useCnt--;
    if(pFw->srd.useCnt > 0)
    {
        return;
    }
    
    LCVUnloadDependencies(pFw, 
            srd_deps, 
            sizeof(srd_deps)/sizeof(struct LCV_DEPENDENCY));
    
	memset(&srd, 0, sizeof(struct LCV_SRD));
}

