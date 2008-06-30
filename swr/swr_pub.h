/*! @file swr_pub.h
 * @brief API definition for stimuli writer module
 *
 * Swr allows to log signal values to a stimuli report file 
 * One writer instance is required per stimuli file while a writer
 * may contain multiple signal instances.
 *
 * The stimuli file follows the following syntax. A example
 * is give first:
 * 
 *   !    Time   ExampleA     ExampleB
 *        
 *   @    0      1            0
 * 
 *   @    20     1            1
 *
 *   ! tab Time   tab  {SignalNameA}  tab  {SignalNameB} (descriptor line)
 * 
 *   @ tab {time} tab  {val}          tab  {val}         (instruction line) 
 *
 * @author Samuel Zahnd
 ************************************************************************/
#ifndef SWR_PUB_H_
#define SWR_PUB_H_

#include "framework_error.h"

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvSwrErrors
{
    ESWR_PARSING_FAILURE = LCV_SWR_ERROR_OFFSET
};

/*! Signal type */
enum EnLcvSwrSignalType
{
    SWR_INTEGER,
    SWR_FLOAT,
    SWR_STRING
};

/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Constructor
 *
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVSwrCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 *
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVSwrDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Create Stimuli Writer (host only)
 *
 * @param ppWriter       O: handle to writer instance
 * @param strFile	     I: output file name
 * @param bReportTime    I: TRUE: report time step information
 * @param bReportCyclic  I: TRUE: autonomous cyclic reporting  
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVSwrCreateWriter( 
        void** ppWriter,        
        const char* strFile, 
        const bool bReportTime,
        const bool bReportCyclic );

/*********************************************************************//*!
 * @brief Register a signal to writer (host only)
 *
 * @param ppSignal      O: handle to signal instance
 * @param pWriter	    I: handle to writer
 * @param strSignal	    I: signal name
 * @param type          I: value type: Interger, Float, String  
 * @param pDefaultValue I: default output value 
 *                         only used for cyclic reporting 
 * @param strFormat     I: log format as fprintf format instruction string  
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVSwrRegisterSignal( 
        void** ppSignal,        
        const void* pWriter, 
        const char* strSignal,
        const enum EnLcvSwrSignalType type,        
        const void* pDefaultValue,
        const char* strFormat );

/*********************************************************************//*!
 * @brief Update Signal value (host only)
 *
 * @param pSignal	I: handle to signal writer
 * @param pValue	I: source pointer of value to set
 *                     Depending on the signal type this will be 
 *                     interpreted as in32, float or string.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVSwrUpdateSignal( 
        const void* pSignal,
        const void* pValue);

/*********************************************************************//*!
 * @brief Manual report (host only)
 *
 * @param pWriter   I: handle to writer
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVSwrManualReport( const void* pWriter);


#endif /*SWR_PUB_H_*/
