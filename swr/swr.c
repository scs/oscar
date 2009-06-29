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

/*! @file
 * @brief Stimuli writer module implementation.
 */

#include <string.h>
#include <stdio.h>

#include "oscar.h"

OSC_ERR OscSwrCreate();
OSC_ERR OscSwrDestroy();

// FIXME: Why does this module exist for the non-simulating target?

#if defined(OSC_HOST) || defined(OSC_SIM)
/*! @brief Limited number of writer instances */
#define MAX_NR_WRITER               10
/*! @brief Limited number of signal instances per writer */
#define MAX_NR_SIGNAL_PER_WRITER    20

/*! @brief Limited string length for string type value */
#define MAX_LENGTH_STRING_VALUE    200

/*!@brief Union for signal value (function argument)*/
union uOSC_SWR_VALUE
{
	int32 nValue;    /*!< @brief Integer coded value */
	float fValue;    /*!< @brief Float coded value */
	char strValue[MAX_LENGTH_STRING_VALUE]; /*!< @brief String coded value */
};

/*!@brief Signal object struct */
struct OSC_SWR_SIGNAL
{
	char* strName;  /*!< @brief Signal name for stimuli file report*/
	enum EnOscSwrSignalType type; /*!< @brief Signal value type*/
	union uOSC_SWR_VALUE value; /*!< @brief Signal value */
	char strFormat[20]; /*!< @brief logging format string */
};

/*!@brief Writer object struct */
struct OSC_SWR_WRITER
{
	FILE* pFile;              /*!< @brief Handle to writer file */
	char strFile[64];
	uint16 nrOfSignals;       /*!< @brief Number of managed signals */
	bool bDescriptorPrinted;  /*!< @brief File descripter line already printed */
	bool bReportTime;
	bool bReportCyclic;
	/*! @brief Signal instance array*/
	struct OSC_SWR_SIGNAL sig[ MAX_NR_SIGNAL_PER_WRITER];
};

void OscSwrCycleCallback( void);
void OscSwrReport( const void* pWriter);
#endif /* OSC_HOST or OSC_SIM*/

/*!@brief Stimuli writer module object struct */
struct OSC_SWR
{
	uint16 nrOfWriters;     /*!< @brief Number of managed writers */
	/*! @brief Writer instance array */
	#if defined(OSC_HOST) || defined(OSC_SIM)
	/*! @brief Array of managed writers */
	struct OSC_SWR_WRITER wr[ MAX_NR_WRITER];
	#endif
};

struct OSC_SWR swr; /*!< Module singelton instance */

struct OscModule OscModule_swr = {
	.name = "swr",
#if defined(OSC_HOST) || defined(OSC_SIM)
	.create = OscSwrCreate,
	.destroy = OscSwrDestroy,
#endif /* defined(OSC_HOST) || defined(OSC_SIM) */
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

#if defined(OSC_HOST) || defined(OSC_SIM)
OSC_ERR OscSwrCreate()
{
	OSC_ERR err;
	
	OscSimRegisterCycleCallback( &OscSwrCycleCallback);
	
	return SUCCESS;
}

OSC_ERR OscSwrDestroy()
{
	uint16 wrId;

	/* close all files */
	for( wrId = 0; wrId<swr.nrOfWriters; wrId++)
	{
		fflush(swr.wr[ wrId].pFile);
		fclose( swr.wr[ wrId].pFile);
		OscLog(INFO, "Close %s\n", &swr.wr[ wrId].strFile);
	}
	
	return SUCCESS;
}
#endif

#if defined(OSC_HOST) || defined(OSC_SIM)
OSC_ERR OscSwrCreateWriter(
		void** ppWriter,
		const char* strFile,
		const bool bReportTime,
		const bool bReportCyclic )
{
	FILE* pFile;
	uint16 id;

	/* Open writer file */
	pFile = fopen( strFile, "w");
	if( pFile == 0)
	{
		printf("Error: Unable open to writer file %s.\n",
				strFile);
		return -EUNABLE_TO_OPEN_FILE;
	}
	if( MAX_NR_WRITER == swr.nrOfWriters)
	{
		printf("Error: Maximum number of writers exhausted");
		return -ENR_OF_INSTANCES_EXHAUSTED;
	}
	id = swr.nrOfWriters;
	swr.nrOfWriters ++;
	swr.wr[id].pFile = pFile;
	swr.wr[id].bReportTime = bReportTime;
	swr.wr[id].bReportCyclic = bReportCyclic;
	
	strcpy(swr.wr[id].strFile, strFile);
	OscLog(INFO, "Open %s\n", strFile);
	
	*ppWriter = &swr.wr[ id];

	return SUCCESS;
}

OSC_ERR OscSwrRegisterSignal(
		void** ppSignal,
		const void* pWriter,
		const char* strSignal,
		const enum EnOscSwrSignalType type,
		const void* pDefaultValue,
		const char* strFormat )
{
	struct OSC_SWR_WRITER* pWr;
	struct OSC_SWR_SIGNAL* pSig;
	uint16 id;

	pWr = (void*) pWriter;
	if( MAX_NR_SIGNAL_PER_WRITER == pWr->nrOfSignals)
	{
		printf("Error: Maximum number of signals exhausted");
		return -ENR_OF_INSTANCES_EXHAUSTED;
	}
	id = pWr->nrOfSignals;
	pWr->nrOfSignals++;
	pSig = &pWr->sig[ id];
	
	pWr->sig[ id].strName = (char*)strSignal;
	pWr->sig[ id].type = type;
	
	if( pDefaultValue)
	{
		switch( pWr->sig[ id].type)
		{
		case SWR_INTEGER:
			pSig->value.nValue = *(int32*)pDefaultValue;
			break;
		case SWR_FLOAT:
			pSig->value.fValue = *(float*)pDefaultValue;
			break;
		case SWR_STRING:
			strcpy( pSig->value.strValue, (char*)pDefaultValue);
		}
	}
	
	strcpy( pWr->sig[ id].strFormat, strFormat);
	
	*ppSignal = pSig;
	return SUCCESS;
}

OSC_ERR OscSwrUpdateSignal(
		const void* pSignal,
		const void* pValue)
{
	struct OSC_SWR_SIGNAL* pSig;
	pSig = (void*) pSignal;
	
	switch( pSig->type)
	{
	case SWR_INTEGER:
		pSig->value.nValue = *(int32*)pValue;
		break;
	case SWR_FLOAT:
		pSig->value.fValue = *(float*)pValue;
		break;
	case SWR_STRING:
		strcpy( pSig->value.strValue, (char*)pValue);
	}
	return SUCCESS;
}

OSC_ERR OscSwrManualReport( const void* pWriter)
{
	OscSwrReport( pWriter);
	return SUCCESS;
}

void OscSwrReport( const void* pWriter)
{
	uint16 sigId;
	struct OSC_SWR_WRITER* pWr;
	struct OSC_SWR_SIGNAL* pSig;
	
	pWr = (void*) pWriter;
	
	/* print descriptor line */
	if( FALSE == pWr->bDescriptorPrinted)
	{
		if(pWr->bReportTime)
		{
			fprintf( pWr->pFile, "!\tTime\t");
		}
		for( sigId = 0; sigId < pWr->nrOfSignals; sigId++)
		{
			fprintf( pWr->pFile, "%s\t",
					pWr->sig[ sigId].strName);
		}
		fprintf( pWr->pFile, "\n");
		pWr->bDescriptorPrinted = TRUE;

	}
	
	/* print instruction line */
	if(pWr->bReportTime)
	{
		fprintf( pWr->pFile, "@\t%lu\t", OscSimGetCurTimeStep() );
	}
	for( sigId = 0; sigId < pWr->nrOfSignals; sigId++)
	{
		pSig = &pWr->sig[ sigId];
		switch( pSig->type)
		{
		case SWR_INTEGER:
			fprintf( pWr->pFile, pSig->strFormat,
					pSig->value.nValue);
			break;
		case SWR_FLOAT:
			fprintf( pWr->pFile, pSig->strFormat,
					pSig->value.fValue);
			break;
		case SWR_STRING:
			fprintf( pWr->pFile, pSig->strFormat,
					pSig->value.strValue);
		}
		fprintf( pWr->pFile, "\t");
	}
	fprintf( pWr->pFile, "\n");
}

void OscSwrCycleCallback( void)
{
	uint16 wrId;

	for( wrId = 0; wrId<swr.nrOfWriters; wrId++)
	{
		/* report all writers (all signals) if cyclic reporting
		 * is selected */
		if( swr.wr[ wrId].bReportCyclic)
		{
			OscSwrReport( &swr.wr[ wrId]);
		}
	}

	return;
}
#endif /* defined(OSC_HOST) || defined(OSC_SIM) */
