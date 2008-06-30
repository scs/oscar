/*! @file sim_host.c
 * @brief Simulation module implementation for host
 * 
 * @author Markus Berner
 */

#include "framework_intern.h"

#include "sim_pub.h"
#include "sim_priv.h"

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

void LCVSimInitialize(void)
{
    uint16 i;
    
    for( i=0; i< sim.numCycleCallback; i++)
    {
        (*sim.aryCycleCallback[ i])();
    }    
}

void LCVSimStep()
{
    uint16 i;
    
    /* pre advance simulation time */
    
    /* advance simulation time */
    sim.curTimeStep++;  
    
    /* post advance operations */
    for( i=0; i< sim.numCycleCallback; i++)
    {
        (*sim.aryCycleCallback[ i])();
    }
}

uint32 LCVSimGetCurTimeStep()
{
    return sim.curTimeStep;
}


LCV_ERR LCVSimRegisterCycleCallback( void (*pCallback)(void))
{
    uint16 id;
    
    id = sim.numCycleCallback;
    if( MAX_NUMBER_CALLBACK_FUNCTION == id)
    {
        printf("Error: Maximum number of callback functions exhausted");
        return -ENUM_CALLBACK_EXHAUSTED;
    } 
    sim.aryCycleCallback[ id] = pCallback;
    sim.numCycleCallback++;
    
    return SUCCESS;
}

