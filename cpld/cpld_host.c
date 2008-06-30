/*! @file cpld_host.c
 * @brief Cpld module implementation for host.
 * 
 ************************************************************************/

#include "oscar_types_host.h"

#include "cpld_pub.h"
#include "cpld_priv.h"
#include "oscar_intern.h"

struct OSC_CPLD cpld; /*!< The cpld module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY cpld_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscCpldCreate(void *hFw)
{
#ifdef TARGET_TYPE_INDXCAM
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;
    
    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->cpld.useCnt != 0)
    {
        pFw->cpld.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module cpld_deps of this module. */
    err = OSCLoadDependencies(pFw, 
            cpld_deps, 
            sizeof(cpld_deps)/sizeof(struct OSC_DEPENDENCY));
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load cpld_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&cpld, 0, sizeof(struct OSC_CPLD));

    /* Increment the use count */
    pFw->cpld.hHandle = (void*)&cpld;
    pFw->cpld.useCnt++;
    
    return SUCCESS;
#else
    OscLog(ERROR, "%s: No CPLD available on this hardware platform!\n",
                __func__);
    return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

void OscCpldDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;

    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->cpld.useCnt--;
    if(pFw->cpld.useCnt > 0)
    {
        return;
    }
    
    OSCUnloadDependencies(pFw, 
            cpld_deps, 
            sizeof(cpld_deps)/sizeof(struct OSC_DEPENDENCY));
    
    memset(&cpld, 0, sizeof(struct OSC_CPLD));
}

OSC_ERR OscCpldRset( 
        const uint16 regId, 
        const uint8 val)
{
#ifdef TARGET_TYPE_INDXCAM
    cpld.reg[ regId] = val;
    return SUCCESS;
#else
    return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldFset( 
        uint16 regId, 
        uint8 field, 
        uint8 val)
{
#ifdef TARGET_TYPE_INDXCAM
    uint8 current;
    current = cpld.reg[ regId];
    if( val)
    {
        current = current | field;
    }
    else
    {
        current = current & (0xff ^ field);
    }
    cpld.reg[ regId] = current;    
    return SUCCESS;
#else
    return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldRget( 
        const uint16 regId,
        uint8* val)
{
#ifdef TARGET_TYPE_INDXCAM
    *val = cpld.reg[ regId];    
    return SUCCESS;
#else
    return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}

OSC_ERR OscCpldFget( 
        const uint16 regId, 
        const uint8 field, 
        uint8* val)
{
#ifdef TARGET_TYPE_INDXCAM
    uint8 current = cpld.reg[ regId];
    if( current & field)
    {
        *val = 1;
    }
    else
    {
        *val = 0;
    }    
    return SUCCESS;
#else
    return -ENO_SUCH_DEVICE;
#endif /* TARGET_TYPE_INDXCAM */
}
