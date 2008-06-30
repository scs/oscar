/*! @file srd_target.c
 * @brief Stimuli reader module implementation for target.
 * 
 ************************************************************************/

#include "oscar_types_target.h"

#include "srd_pub.h"
#include "srd_priv.h"
#include "oscar_intern.h"

struct OSC_SRD srd;		/*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY srd_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscSrdCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->srd.useCnt != 0)
    {
        pFw->srd.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }

    /* Load the module srd_deps of this module. */
    err = OSCLoadDependencies(pFw, 
            srd_deps, 
            sizeof(srd_deps)/sizeof(struct OSC_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load srd_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
 	memset(&srd, 0, sizeof(struct OSC_SRD));
	
    /* Increment the use count */
    pFw->srd.hHandle = (void*)&srd;
    pFw->srd.useCnt++;  
    
    return SUCCESS;
}

void OscSrdDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
        
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->srd.useCnt--;
    if(pFw->srd.useCnt > 0)
    {
        return;
    }
    
    OSCUnloadDependencies(pFw, 
            srd_deps, 
            sizeof(srd_deps)/sizeof(struct OSC_DEPENDENCY));
    
	memset(&srd, 0, sizeof(struct OSC_SRD));
}

