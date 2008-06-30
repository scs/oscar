/*! @file rtl_target.c
 * @brief Blackfin DSP runtime library implementation for host. 
 * 
 * All functions of the DSP runtime library used on the target must
 * be implemented for the host. The implementation for the target on the
 * other hand must not be done in the framework, since it already exists
 * in optimized form in the library.  
 * 
 * @author Samuel Zahnd, Markus Berner
 */

#include "framework_types_target.h"

#include "rtl_pub.h"
#include "rtl_priv.h"
#include "framework_intern.h"

/*! @brief The module singelton instance.  */
struct LCV_RTL lcv_rtl;		


LCV_ERR LCVRtlCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->rtl.useCnt != 0)
    {
        pFw->rtl.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }    
        
	memset(&lcv_rtl, 0, sizeof(struct LCV_RTL));
		
    /* Increment the use count */
    pFw->rtl.hHandle = (void*)&lcv_rtl;
    pFw->rtl.useCnt++;    

	return SUCCESS;
}

void LCVRtlDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
        
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->rtl.useCnt--;
    if(pFw->rtl.useCnt > 0)
    {
        return;
    }
	
	memset(&lcv_rtl, 0, sizeof(struct LCV_RTL));
}


inline float LCVRtlFr16ToFloat(fract16 n)
{
    return (((float)n)/FR16_SCALE);
}

inline fract16 LCVRtlFloatToFr16(float n)
{
    float ret = (n*FR16_SCALE);
    if(ret > FR16_SAT)
        ret = FR16_SAT;
    return (fract16)ret;
}
