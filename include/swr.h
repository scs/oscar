/*	Oscar, a hardware abstraction framework for the LeanXcam and IndXcam.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

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
 */

#ifndef SWR_PUB_H_
#define SWR_PUB_H_

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscSwrErrors
{
	ESWR_PARSING_FAILURE = OSC_SWR_ERROR_OFFSET
};

/*! Signal type */
enum EnOscSwrSignalType
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
OSC_ERR OscSwrCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscSwrDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Create Stimuli Writer (host only)
 * 
 * @param ppWriter       O: handle to writer instance
 * @param strFile        I: output file name
 * @param bReportTime    I: TRUE: report time step information
 * @param bReportCyclic  I: TRUE: autonomous cyclic reporting
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSwrCreateWriter(
		void** ppWriter,
		const char* strFile,
		const bool bReportTime,
		const bool bReportCyclic );

/*********************************************************************//*!
 * @brief Register a signal to writer (host only)
 * 
 * @param ppSignal      O: handle to signal instance
 * @param pWriter       I: handle to writer
 * @param strSignal     I: signal name
 * @param type          I: value type: Interger, Float, String
 * @param pDefaultValue I: default output value
 *                         only used for cyclic reporting
 * @param strFormat     I: log format as fprintf format instruction string
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSwrRegisterSignal(
		void** ppSignal,
		const void* pWriter,
		const char* strSignal,
		const enum EnOscSwrSignalType type,
		const void* pDefaultValue,
		const char* strFormat );

/*********************************************************************//*!
 * @brief Update Signal value (host only)
 * 
 * @param pSignal   I: handle to signal writer
 * @param pValue    I: source pointer of value to set
 *                     Depending on the signal type this will be
 *                     interpreted as in32, float or string.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSwrUpdateSignal(
		const void* pSignal,
		const void* pValue);

/*********************************************************************//*!
 * @brief Manual report (host only)
 * 
 * @param pWriter   I: handle to writer
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscSwrManualReport( const void* pWriter);


#endif /*SWR_PUB_H_*/
