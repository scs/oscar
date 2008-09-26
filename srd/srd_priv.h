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

/*! @file srd_priv.h
 * @brief Private stimuli reader module definition
 * 
	************************************************************************/
#ifndef SRD_PRIV_H_
#define SRD_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef OSC_HOST
#include <oscar_types_host.h>
#else
#include <oscar_types_target.h>
#endif /* OSC_HOST */

#define MAX_NR_READER   10
#define MAX_NR_SIGNAL_PER_READER    20

/*!@brief Reader signal object struct */
struct OSC_SRD_SIGNAL
{
	char* strName;          /*!< @brief Signal name string */
	bool bReadValue;        /*!< @brief Read value from last parsed stimuli line */
	bool bActiveValue;      /*!< @brief Active signal value */
};

/*!@brief Reader object struct */
struct OSC_SRD_READER
{
	FILE* pFile;                    /*!< @brief Handle to reader file */
	char* strFile;                  /*!< @brief Reader file name*/
	bool bDescriptorParsed;         /*!< @brief Stimuli descriptor line already parsed */
	uint32 readTime;                /*!< @brief Timestamp of recent file readline */
	void (*pUpdateCallback)(void);  /*!< @brief Callback to inform about update */
	uint16 nrOfSignals;             /*!< @brief Number of managed signal instances */
	/*! Signal object array*/
	struct OSC_SRD_SIGNAL sig[MAX_NR_SIGNAL_PER_READER];
};

/*!@brief Stimuli reader module object struct */
struct OSC_SRD
{
	uint16 nrOfReaders;     /*!< @brief Number of managed readers */
	/*! @brief Reader object array */
#if defined(OSC_HOST) || defined(OSC_SIM)
	struct OSC_SRD_READER rd[MAX_NR_READER];
#endif
};

#if defined(OSC_HOST) || defined(OSC_SIM)

/*********************************************************************//*!
 * @brief Cycle Callback for Sim module registration (host only)
 * 
 *//*********************************************************************/
static void OscSrdCycleCallback( void);

/*********************************************************************//*!
 * @brief Read descriptor line in all stimuli files (host only)
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR ReadAllDescriptor( void);

/*********************************************************************//*!
 * @brief Get next values states (host only)
 * 
 * @param rdId Reader array index
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR GetNext( uint16 rdId);

/*********************************************************************//*!
 * @brief Read next line from stimuli file (host only)
 * 
 * @param rdId Reader array index
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR ReadLine( uint16 rdId);
#endif /*OSC_HOST or OSC_SIM*/

#endif /* SRD_PRIV_H_ */
