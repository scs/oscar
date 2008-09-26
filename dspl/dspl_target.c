/*! @file dspl_target.c
 * @brief Blackfin DSP runtime library implementation for host.
 * 
 * All functions of the DSP runtime library used on the target must
 * be implemented for the host. The implementation for the target on the
 * other hand must not be done in the framework, since it already exists
 * in optimized form in the library.
 * 
 */

#include "oscar_types_target.h"

#include "dspl_pub.h"
#include "dspl_priv.h"
#include "oscar_intern.h"

/*! @brief The module singelton instance.  */
struct OSC_DSPL osc_dspl;


OSC_ERR OscDsplCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->dspl.useCnt != 0)
	{
		pFw->dspl.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
		
	memset(&osc_dspl, 0, sizeof(struct OSC_DSPL));
		
	/* Increment the use count */
	pFw->dspl.hHandle = (void*)&osc_dspl;
	pFw->dspl.useCnt++;

	return SUCCESS;
}

void OscDsplDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->dspl.useCnt--;
	if(pFw->dspl.useCnt > 0)
	{
		return;
	}
	
	memset(&osc_dspl, 0, sizeof(struct OSC_DSPL));
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
