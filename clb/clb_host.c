/*! @file clb_host.c
 * @brief Calibration module implementation for host
 *  
 * No calibration is applied to host pictures.
 * 
 * @author Samuel Zahnd
 */
#include "framework_intern.h"
#include "clb_pub.h"
#include "clb_priv.h"
#include <stdlib.h>

/*! @brief The dependencies of this module. */
struct LCV_DEPENDENCY clb_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy}
};

struct LCV_CLB clb; /*!< @brief The clbera module singelton instance */


LCV_ERR LCVClbCreate(void *hFw)
{
    struct LCV_FRAMEWORK    *pFw;
    LCV_ERR                 err;
    
    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->clb.useCnt != 0)
    {
        pFw->clb.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }  

    /* Load the module clb_deps of this module. */
    err = LCVLoadDependencies(pFw, 
            clb_deps, 
            sizeof(clb_deps)/sizeof(struct LCV_DEPENDENCY));
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load clb_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
        
    memset(&clb, 0, sizeof(struct LCV_CLB));    

    /* Increment the use count */
    pFw->clb.hHandle = (void*)&clb;
    pFw->clb.useCnt++; 
    
    return SUCCESS;
}

void LCVClbDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
            
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->clb.useCnt--;
    if(pFw->clb.useCnt > 0)
    {
        return;
    }
        
    LCVUnloadDependencies(pFw, 
            clb_deps, 
            sizeof(clb_deps)/sizeof(struct LCV_DEPENDENCY));
    
    memset(&clb, 0, sizeof(struct LCV_CLB));
}

LCV_ERR LCVClbSetupCalibrate(
        enum EnLcvClbCalibrateSlope calibSlope,
        bool bHotpixel)
{
    return SUCCESS;
}

LCV_ERR LCVClbApplyCorrection( uint8 *pImg,
        const uint16 lowX, const uint16 lowY,
        const uint16 width, const uint16 height)
{
    return SUCCESS;
}

