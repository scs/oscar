/*! @file srd_pub.h
 * @brief API definition for stimuli reader module
 * 
 * Srd allows to read signal values from a stimuli file.
 * One reader instance is required per stimuli file while a reader
 * may contain multiple signal instances.
 * 
 * The stimuli file must fullfill following syntax rules. A example
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
 * The order of signal names has to match with the order of signal
 * registration to the reader.
 * 
	************************************************************************/
#ifndef SRD_PUB_H_
#define SRD_PUB_H_

#include "oscar_error.h"

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscSrdErrors
{
	ESRD_PARSING_FAILURE = OSC_SRD_ERROR_OFFSET
};

/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSrdCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscSrdDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Create Stimuli Reader (host only)
 * 
 * @param strFile           I: output file name
 * @param pUpdateCallback   I: callback fxn to notify a value change
 * @param ppReader          O: handle to reader instance
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSrdCreateReader( char* strFile,
		void (*pUpdateCallback)(void),
		void** ppReader);

/*********************************************************************//*!
 * @brief Register a signal to reader (host only)
 * 
 * @param pReader   I: handle to reader
 * @param strSignal I: signal name
 * @param ppSignal  O: handle to signal instance
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSrdRegisterSignal( void* pReader, char* strSignal, void** ppSignal);

/*********************************************************************//*!
 * @brief GetUpdateSignal (host only)
 * 
 * @param pSignal   I: handle to signal
 * @param pbValue   O: return active signal value
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSrdGetUpdateSignal( void* pSignal, bool* pbValue);



#endif /*SRD_PUB_H_*/
