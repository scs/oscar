/*! @file rtl_target.c
 * @brief Blackfin DSP runtime library implementation for host. 
 * 
 * All functions of the DSP runtime library used on the target must
 * be implemented for the host. The implementation for the target on the
 * other hand must not be done in the framework, since it already exists
 * in optimized form in the library.  
 * 
 */

#include "oscar_types_target.h"

#include "rtl_pub.h"
#include "rtl_priv.h"
#include "oscar_intern.h"

/*! @brief The module singelton instance.  */
struct OSC_RTL osc_rtl;		


OSC_ERR OscDsplCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->rtl.useCnt != 0)
    {
        pFw->rtl.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }    
        
	memset(&osc_rtl, 0, sizeof(struct OSC_RTL));
		
    /* Increment the use count */
    pFw->rtl.hHandle = (void*)&osc_rtl;
    pFw->rtl.useCnt++;    

	return SUCCESS;
}

void OscDsplDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
        
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->rtl.useCnt--;
    if(pFw->rtl.useCnt > 0)
    {
        return;
    }
	
	memset(&osc_rtl, 0, sizeof(struct OSC_RTL));
}


inline float OscDsplFr16ToFloat(fract16 n)
{
    return (((float)n)/FR16_SCALE);
}

inline fract16 OscDsplFloatToFr16(float n)
{
    float ret = (n*FR16_SCALE);
    if(ret > FR16_SAT)
        ret = FR16_SAT;
    return (fract16)ret;
}
