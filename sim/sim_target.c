/*! @file sim_target.c
 * @brief Simulation module implementation for host 
 * 
 * @author Markus Berner
 */

#include "framework_types_target.h"

#include "sim_pub.h"
#include "sim_priv.h"
#include "framework_intern.h"

/*! @brief The module singelton instance. */
struct LCV_SIM_OBJ sim;     

LCV_ERR LCVSimCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->sim.useCnt != 0)
    {
        pFw->sim.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
     
    memset(&sim, 0, sizeof(struct LCV_SIM_OBJ));
        
    /* Increment the use count */
    pFw->sim.hHandle = (void*)&sim;
    pFw->sim.useCnt++; 
    
    return SUCCESS;
}

void LCVSimDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
            
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->sim.useCnt--;
    if(pFw->sim.useCnt > 0)
    {
        return;
    }
    
    memset(&sim, 0, sizeof(struct LCV_SIM_OBJ));
}

/*********************************************************************//*!
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
void LCVSimInitialize(void)
{   
}

void LCVSimStep()
{
}

uint32 LCVSimGetCurTimeStep()
{
    return SUCCESS;
}


/*********************************************************************//*!
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
LCV_ERR LCVSimRegisterCycleCallback( void (*pCallback)(void))
{
    return SUCCESS;
}
