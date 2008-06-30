/*! @file sim_target.c
 * @brief Simulation module implementation for host 
 * 
 */

#include "oscar_types_target.h"

#include "sim_pub.h"
#include "sim_priv.h"
#include "oscar_intern.h"

/*! @brief The module singelton instance. */
struct OSC_SIM_OBJ sim;     

OSC_ERR OscSimCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->sim.useCnt != 0)
    {
        pFw->sim.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
     
    memset(&sim, 0, sizeof(struct OSC_SIM_OBJ));
        
    /* Increment the use count */
    pFw->sim.hHandle = (void*)&sim;
    pFw->sim.useCnt++; 
    
    return SUCCESS;
}

void OscSimDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
            
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->sim.useCnt--;
    if(pFw->sim.useCnt > 0)
    {
        return;
    }
    
    memset(&sim, 0, sizeof(struct OSC_SIM_OBJ));
}

/*********************************************************************//*!
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
void OscSimInitialize(void)
{   
}

void OscSimStep()
{
}

uint32 OscSimGetCurTimeStep()
{
    return SUCCESS;
}


/*********************************************************************//*!
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
OSC_ERR OscSimRegisterCycleCallback( void (*pCallback)(void))
{
    return SUCCESS;
}
