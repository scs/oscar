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
 * @brief Stimuli reader module implementation for host.
 */

#include "srd.h"

struct OSC_SRD srd; /*!< Module singelton instance */

struct OscModule OscModule_srd = {
	.create = OscSrdCreate,
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

OSC_ERR OscSrdCreate()
{
	OSC_ERR err;
	
	sup = (struct OSC_SRD) { };

	OscSimRegisterCycleCallback( &OscSrdCycleCallback);
	
	return SUCCESS;
}

OSC_ERR OscSrdCreateReader(
			char* strFile,
			void (*pUpdateCallback)(void),
			void** ppReader)
{
	FILE* pFile;
	uint16 id;
		
	/* Open reader file */
	pFile = fopen( strFile, "r");
	if( pFile == 0)
	{
		printf("Error: Unable open to reader file %s.\n",
				strFile);
		return -EUNABLE_TO_OPEN_FILE;
	}
	if( MAX_NR_READER == srd.nrOfReaders)
	{
		printf("Error: Maximum number of readers exhausted");
		return -ENR_OF_INSTANCES_EXHAUSTED;
	}
	id = srd.nrOfReaders;
	srd.nrOfReaders ++;
	srd.rd[ id].pFile = pFile;
	srd.rd[ id].strFile = strFile;
	srd.rd[id].pUpdateCallback = pUpdateCallback;
			
	*ppReader = &srd.rd[ id];
	
	return SUCCESS;
}

OSC_ERR OscSrdRegisterSignal(
			void* pReader,
			char* strSignal,
			void** ppSignal)
{
	struct OSC_SRD_READER* pRd;
	uint16 id;
		
	pRd = pReader;
	if( MAX_NR_SIGNAL_PER_READER == pRd->nrOfSignals)
	{
		printf("Error: Maximum number of signals exhausted");
		return -ENR_OF_INSTANCES_EXHAUSTED;
	}
	id = pRd->nrOfSignals;
	pRd->nrOfSignals++;
	
	pRd->sig[ id].strName = strSignal;
	*ppSignal = &pRd->sig[ id];
		
	return SUCCESS;
}

void OscSrdCycleCallback( void)
{
	uint16 rdId;
	
	ReadAllDescriptor();

	for( rdId = 0; rdId < srd.nrOfReaders; rdId++)
	{
		GetNext( rdId);
	}
	
	return;
}

OSC_ERR OscSrdGetUpdateSignal(
			void* pSignal,
			bool* pbValue)
{
	struct OSC_SRD_SIGNAL* pSig;

	pSig = pSignal;
	*pbValue = pSig->bActiveValue;
	return SUCCESS;
}

OSC_ERR ReadAllDescriptor( void)
{
	char strRead[50];
	int32 ret;
	uint16 rdId;
	uint16 sigId;
	
	for( rdId = 0; rdId < srd.nrOfReaders; rdId++)
	{

		/* read descriptor line if not already parsed */
		if( FALSE ==srd.rd[ rdId].bDescriptorParsed)
		{
			/* parse line begin:  !  \t  time */
			ret = fscanf(srd.rd[ rdId].pFile, "%*1[!]\t%*s");
			if( ret < 0)
			{
				OscLog(ERROR, "%s: Missing descriptor begin (! \t Time)\n", srd.rd[ rdId].strFile);
				return EFILE_PARSING_ERROR;
			}
			for( sigId= 0; sigId<srd.rd[ rdId].nrOfSignals; sigId++)
			{
				/* parse element: \t signalname */
				ret = fscanf(srd.rd[ rdId].pFile, "\t%s", strRead);
				OscLog(DEBUG, "Parsing element: %s\n", strRead);
				if( ret <= 0)
				{
					OscLog(ERROR, "%s: Missing descriptor element or name does not match.\n",
							srd.rd[ rdId].strFile);
					return EFILE_PARSING_ERROR;
				}
				if( strcmp( strRead, srd.rd[ rdId].sig[ sigId].strName) != 0)
				{
					OscLog(ERROR, "%s: Wrong descriptor element order.\n",
							srd.rd[ rdId].strFile);
					return EFILE_PARSING_ERROR;
				}
				
			}
			/* parse descriptor line end: \n  */
			ret = fscanf(srd.rd[ rdId].pFile, "\n");
			if( ret != 0)
			{
				OscLog(ERROR, "%s: Missing end of descriptor line.\n", srd.rd[ rdId].strFile);
				return EFILE_PARSING_ERROR;
			}
		}
		
		srd.rd[ rdId].bDescriptorParsed = TRUE;
	}
	return SUCCESS;
}

OSC_ERR GetNext( uint16 rdId)
{
	uint16 sigId;
	
	uint32 currTime;
	OSC_ERR readLine;
	
	currTime = OscSimGetCurTimeStep();
	readLine = SUCCESS;
	
	/* compare system time with the time stamp last time read (readtime)
	 *   sys.time > readtime:   read new line and store to read value/time
	 *                          elaluate again
	 *   sys.time = readtime:   copy read value/time to current value/time
	 *                          callback
	 *   sys.time < readtime:   do nothing
	 */
	
	while( (readLine == SUCCESS) & (currTime >= srd.rd[ rdId].readTime) )
	{
		if( currTime == srd.rd[ rdId].readTime)
		{
			for( sigId= 0; sigId<srd.rd[ rdId].nrOfSignals; sigId++)
			{
				srd.rd[ rdId].sig[ sigId].bActiveValue = srd.rd[ rdId].sig[ sigId].bReadValue;
				(*srd.rd[ rdId].pUpdateCallback)();
			}
		}
		
		readLine = ReadLine( rdId);
	}
	
	return SUCCESS;
}
	
	
OSC_ERR ReadLine( uint16 rdId)
{
	int32 ret;
	uint16 sigId;
	
	uint32 time;
	uint32 value;
		
	ret = fscanf(srd.rd[ rdId].pFile, "%*1[@]");
	if( ret == EOF)
	{
		return EOF;
	}
	if( ret != 0)
	{
		OscLog(ERROR, "%s: Missing line opening (@ \t)\n", srd.rd[ rdId].strFile);
		return EFILE_PARSING_ERROR;
	}
	
	ret = fscanf(srd.rd[ rdId].pFile, "\t%lu", &time);
	OscLog(DEBUG, "Parsing time: %d\n", time);
	if( ret <= 0)
	{
		OscLog(ERROR, "%s: Missing time value  (unsigned decimal number)\n", srd.rd[ rdId].strFile);
		return EFILE_PARSING_ERROR;
	}
	srd.rd[ rdId].readTime = time;
	
	for( sigId= 0; sigId<srd.rd[ rdId].nrOfSignals; sigId++)
	{
		ret = fscanf(srd.rd[ rdId].pFile, "\t%lu", &value);
		OscLog(DEBUG, "Parsing value: %d\n", value);
		if( ret <= 0)
		{
			OscLog(ERROR, "%s: Missing signal value (unsigned decimal number).\n",
					srd.rd[ rdId].strFile);
			return EFILE_PARSING_ERROR;
		}
		srd.rd[ rdId].sig[ sigId].bReadValue = value;
			
	}
	ret = fscanf(srd.rd[ rdId].pFile, "\n");
	if( ret != 0)
	{
		OscLog(ERROR, "%s: Missing end of line.\n", srd.rd[ rdId].strFile);
		return EFILE_PARSING_ERROR;
	}
	
	return SUCCESS;
}

