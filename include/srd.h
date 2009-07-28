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
#ifndef OSCAR_INCLUDE_SRD_H_
#define OSCAR_INCLUDE_SRD_H_

/*! @file
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
 */

extern struct OscModule OscModule_srd;

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

#endif // #ifndef OSCAR_INCLUDE_SRD_H_
