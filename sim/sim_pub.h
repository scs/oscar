/*! @file sim_pub.h
 * @brief API definition for simulation module
 * 
 * Simulation is only used for host. Target implementation provides 
 * stump function definition.
 * 
 * @author Markus Berner
 */
#ifndef SIM_PUB_H_
#define SIM_PUB_H_

#include "framework_error.h"
#ifdef LCV_HOST
    #include "framework_types_host.h"
    #include "framework_host.h"
#else
    #include "framework_types_target.h"
    #include "framework_target.h"
#endif /* LCV_HOST */

/*! @brief Module-specific error codes.
 * 
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvSimErrors {
    ENUM_CALLBACK_EXHAUSTED = LCV_SIM_ERROR_OFFSET
};

/*======================== API functions ===============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVSimCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVSimDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Initialize simulation
 * 
 * After creation of all required modules the application has to init
 * the simulation module. Time variable is set to 0. Callbacks to
 * stimuli reader and writer are issued to applie default signal values. 
 *//*********************************************************************/
void LCVSimInitialize(void);

/*********************************************************************//*!
 * @brief Increment the simulation time step. 
 * 
 * The application should call this function after every 'frame' of 
 * the simulation. This automatically adjusts the current test image
 * file name fo the next cycle. Callback for stimuli reader and
 * writer is issued after timer increment. This prepares the the input
 * signals for the next cycle and writes current output signals (to 
 * next cycle!) 
 * Target: Stump since simulation is only done on host.
 *//*********************************************************************/
void LCVSimStep();

/*********************************************************************//*!
 * @brief Get the current simulation time step.
 * 
 * Target: Stump since simulation is only done on host.
 * 
 * @return The current simulation time step.
 *//*********************************************************************/
uint32 LCVSimGetCurTimeStep();

/*********************************************************************//*!
 * @brief Register a callback function to be called every new timestep.
 * 
 * Target: Stump since simulation is only done on host.
 * 
 * @param pCallback Pointer to the function to be called.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
LCV_ERR LCVSimRegisterCycleCallback( void (*pCallback)(void));

#endif /*SIM_PUB_H_*/
