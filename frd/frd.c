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
 * @brief Filename reader module implementation.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "oscar.h"

/*! @brief The maximum number of readers that can be registered. */
#define MAX_NR_READERS 8

/*! @brief The maximum length of a file name prefix. */
#define MAX_PREFIX_LEN 1024
/*! @brief The maximum length of a file name postfix. */
#define MAX_SUFFIX_LEN 16
/*! @brief The maximum length of a path name of the file name list
 * reader. */
#define MAX_PATH_LEN 1024

/*! @brief Reader object struct for a sequential reader*/
struct OSC_FRD_SEQUENCE_READER
{
	/*! @brief The prefix of the filenames. */
	char strPrefix[MAX_PREFIX_LEN];
	/*! @brief To how many digits the sequence number is expanded. */
	int seqNrDigits;
	/*! @brief The suffix of the filenames. */
	char strSuffix[MAX_SUFFIX_LEN];
};

/*! @brief Reader object struct for a filelist reader*/
struct OSC_FRD_FILELIST_READER
{
	/* @brief The file name of the file list. */
	char strFileList[1024];
	/* @brief The open file handle to the file list. */
	FILE * pFList;
	/* @brief The current file name. */
	char curFileName[1024];
};

/*! @brief Reader object struct for a constant reader*/
struct OSC_FRD_CONSTANT_READER
{
	/* @brief The constant file name to return. */
	char strFN[1024];
};

/* @brief Enumeration of the different supported reader types. */
enum EnFilenameReaderType
{
	FRD_READER_TYPE_SEQUENCE,
	FRD_READER_TYPE_LIST,
	FRD_READER_TYPE_CONSTANT
};

/* @brief A reader can only be of one type, thus placing all reader
 * types in a union. */
struct OSC_FRD_READER
{
	/*! @brief The actual type of this reader. */
	enum EnFilenameReaderType enType;
	/*! @brief The actual data of the reader, depending on its type. */
	union OSC_FRD_READER_DATA
	{
		/*! @brief Sequence file name reader. */
		struct OSC_FRD_SEQUENCE_READER seq;
		/*! @brief List file name reader. */
		struct OSC_FRD_FILELIST_READER list;
		/*! @brief Constant file name reader. */
		struct OSC_FRD_CONSTANT_READER constant;
	} reader;
};

/*!@brief Filename reader module object struct */
struct OSC_FRD
{
	uint16 nrOfReaders;     /*!< @brief Number of managed readers */
	/*! @brief Reader object array */
	struct OSC_FRD_READER rd[MAX_NR_READERS];
};

struct OSC_FRD frd; /*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY frd_deps[] = {
		{"log", OscLogCreate, OscLogDestroy},
		{"sim", OscSimCreate, OscSimDestroy}
};

static void OscFrdSimCycleCallback();

OSC_ERR OscFrdCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->frd.useCnt != 0)
	{
		pFw->frd.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}

	/* Load the module frd_deps of this module. */
	err = OscLoadDependencies(pFw,
			frd_deps,
			sizeof(frd_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load frd_deps! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	memset(&frd, 0, sizeof(struct OSC_FRD));

	/* Register a callback to be invoked every simulation cycle
	 * to be able to fetch the next file name from the list. */
	err = OscSimRegisterCycleCallback(OscFrdSimCycleCallback);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to register cycle callback (%d)\n",
				__func__, err);
		return err;
	}
	
	/* Increment the use count */
	pFw->frd.hHandle = (void*)&frd;
	pFw->frd.useCnt++;
	
	return SUCCESS;
}

void OscFrdDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	struct OSC_FRD_READER *pReader;
	uint32 rd;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->frd.useCnt--;
	if(pFw->frd.useCnt > 0)
	{
		return;
	}
	
	for(rd = 0; rd < frd.nrOfReaders; rd++)
	{
		pReader = &frd.rd[rd];
		
		switch(pReader->enType)
		{
		case FRD_READER_TYPE_LIST:
			fclose(pReader->reader.list.pFList);
			break;
		case FRD_READER_TYPE_SEQUENCE:
		case FRD_READER_TYPE_CONSTANT:
			/* No operation necessary reader. */
			break;
		default:
			OscLog(ERROR,
					"%s: Unsupported reader type configured (%d)!\n",
					__func__, pReader->enType);
		}
	}
	OscUnloadDependencies(pFw,
			frd_deps,
			sizeof(frd_deps)/sizeof(struct OSC_DEPENDENCY));
	
	
	memset(&frd, 0, sizeof(struct OSC_FRD));
}

/*********************************************************************//*!
 * @brief Read the next file name from the file list.
 *//*********************************************************************/
static void OscFrdListFetchNextFileName(
		struct OSC_FRD_FILELIST_READER *pReader)
{
	int assigned;
	
	if(unlikely(pReader->pFList == NULL))
	{
		OscLog(ERROR, "%s: No file list open!\n", __func__);
		return;
	}
	
	/* Get the next file name from the list. */
	assigned = fscanf(pReader->pFList, "%[^\t\n]\n",
			pReader->curFileName);
	if(unlikely(assigned != 1))
	{
		if(assigned == EOF)
		{
			OscLog(WARN, "%s: Parse error: End of File! (%s)\n",
					__func__, pReader->strFileList);
		} else {
			OscLog(ERROR, "%s: Parse error: Unable to parse next "
					"file name! (%s)\n", __func__,
					pReader->strFileList);
		}
		pReader->curFileName[0] = '\0';
	} else { /* succeeded */
		/* Sanity checks */
		if(unlikely(pReader->curFileName[0] == '\0'))
		{
			OscLog(WARN, "%s: Next file name is empty string! "
					"Probably not intended.\n", __func__);
		}
	}
}

/*********************************************************************//*!
 * @brief The callback being invoked every simulation cycle.
 * 
 * Execute periodic tasks depending on the reader type.
 *//*********************************************************************/
static void OscFrdSimCycleCallback()
{
	int rd;
	struct OSC_FRD_READER *pReader;
	
	/* Go through all readers and execute the necessary actions
	 * depending on their type. */
	for(rd = 0; rd < frd.nrOfReaders; rd++)
	{
		pReader = &frd.rd[rd];
		
		switch(pReader->enType)
		{
		case FRD_READER_TYPE_LIST:
			OscFrdListFetchNextFileName(&pReader->reader.list);
			break;
		case FRD_READER_TYPE_SEQUENCE:
		case FRD_READER_TYPE_CONSTANT:
			/* No operation necessary. */
			break;
		default:
			OscLog(ERROR,
					"%s: Unsupported reader type configured (%d)!\n",
					__func__, pReader->enType);
		}
	}
}

/* ------------------------ Sequential reader -----------------------------*/
/*********************************************************************//*!
 * @brief Parses in the parameters of a sequential file name reader.
 * 
 * @param pConfigFile Open handle to the frd config file advanced to the
 * position where the options of the sequence reader are located.
 * @param pReader Reader object structure to complete with the information
 * in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscFrdParseSequentialReader(FILE *pConfigF,
		struct OSC_FRD_SEQUENCE_READER * pReader)
{
	int                         assigned;
	
	assigned = fscanf(pConfigF, "FILENAME_PREFIX = %s\n",
			pReader->strPrefix);
	if(unlikely(assigned != 1))
	{
		return -EFRD_PARSING_FAILURE;
	}
	
	assigned = fscanf(pConfigF, "FILENAME_SEQ_NR_DIGITS = %d\n",
			&pReader->seqNrDigits);
	if(unlikely(assigned != 1))
	{
		return -EFRD_PARSING_FAILURE;
	}
	
	assigned = fscanf(pConfigF, "FILENAME_SUFFIX = %s\n",
			pReader->strSuffix);
	if(unlikely(assigned != 1))
	{
		return -EFRD_PARSING_FAILURE;
	}
	
	/* Sanity checks. */
	if(unlikely((pReader->seqNrDigits > 1000) ||
			(pReader->seqNrDigits < 1) ||
			(pReader->strPrefix[0] == '\0') ||
			(pReader->strSuffix[0] == '\0')))
	{
		return -EFRD_INVALID_VALUES_CONFIGURED;
	}
	
	return SUCCESS;
}

OSC_ERR OscFrdCreateSequenceReader(void **phReaderHandle,
		char strPrefix[],
		int seqNrDigits,
		char strSuffix[])
{
	struct OSC_FRD_READER           *pReader;
	struct OSC_FRD_SEQUENCE_READER  *pSeqReader;
	
	/* Input validation. */
	if(unlikely(phReaderHandle == NULL ||
			strPrefix == NULL ||
			strSuffix == NULL ||
			seqNrDigits < 1 || seqNrDigits > 10))
	{
		OscLog(ERROR,
				"%s(0x%x, 0x%x, %d, 0x%x): "
				"Invalid parameter!\n", __func__,
				(uint32)phReaderHandle, strPrefix,
				seqNrDigits, strSuffix);
		return -EINVALID_PARAMETER;
	}

	if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
	{
		OscLog(ERROR, "%s: Maximum number of readers reached!\n",
				__func__);
		return -EFRD_MAX_NR_READERS_REACHED;
	}

	/* Create the file list reader. */
	pReader = &frd.rd[frd.nrOfReaders];
	frd.nrOfReaders++;

	pReader->enType = FRD_READER_TYPE_SEQUENCE;
	pSeqReader = &pReader->reader.seq;
	strcpy(pSeqReader->strPrefix, strPrefix);
	strcpy(pSeqReader->strSuffix, strSuffix);
	pSeqReader->seqNrDigits = seqNrDigits;
	
	*phReaderHandle = (void*)pReader;

	return SUCCESS;
}

/*********************************************************************//*!
 * @brief Get the current file name of a sequence reader.
 * 
 * @param pReader Handle to the file sequence reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static void OscFrdSeqGetCurrentFileName(
		const struct OSC_FRD_SEQUENCE_READER *pReader,
		char strCurName[])
{
	char        strFormat[16];
	char        strSeq[16];
	uint32      curSeqNr;
	
	/* Get the current time step from the simulation module. */
	curSeqNr = OscSimGetCurTimeStep();
	
	/* Customize the format string then use it to stringify the
	 * sequence number afterwards. ("%<x>u", where <x> is the number
	 * of digits of the sequence number) */
	sprintf(strFormat, "%%0%uu", pReader->seqNrDigits);
	sprintf(strSeq, strFormat, curSeqNr);

	/* File name is prefix +  number + suffix */
	strcpy(strCurName, pReader->strPrefix);
	strcat(strCurName, strSeq);
	strcat(strCurName, pReader->strSuffix);
}

/* ---------------------------- List reader -------------------------------*/
/*********************************************************************//*!
 * @brief Parses in the parameters of a file name list reader.
 * 
 * @param pConfigF Open handle to the frd config file advanced to the
 * position where the options of the sequence reader are located.
 * @param pReader Reader object structure to complete with the information
 * in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscFrdParseListReader(FILE *pConfigF,
		struct OSC_FRD_FILELIST_READER * pReader)
{
	char                        strTemp[1024];
	int                         assigned;
	
	assigned = fscanf(pConfigF, "FILENAME_LIST = %s\n",
			strTemp);
	if(unlikely(assigned != 1))
	{
		return -EFRD_PARSING_FAILURE;
	}
	
	/* Open the file list. */
	pReader->pFList = fopen(strTemp, "r");
	if(pReader->pFList == NULL)
	{
		OscLog(ERROR, "%s: Unable to open file list (%s)! Errno: %s\n",
				__func__, strTemp, strerror(errno));
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	strcpy(pReader->strFileList, strTemp);
	
	return SUCCESS;
}

OSC_ERR OscFrdCreateFileListReader(void **phReaderHandle,
		const char strFileList[])
{
	struct OSC_FRD_READER           *pReader;
	struct OSC_FRD_FILELIST_READER  *pFLReader;
	/* Input validation. */
	if(unlikely(phReaderHandle == NULL || strFileList == NULL ||
			strFileList[0] == '\0'))
	{
		OscLog(ERROR, "%s(0x%x, %s): Invalid parameter!\n",
				__func__, (uint32)phReaderHandle, strFileList);
		return -EINVALID_PARAMETER;
	}
	
	if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
	{
		OscLog(ERROR, "%s: Maximum number of readers reached!\n",
				__func__);
		return -EFRD_MAX_NR_READERS_REACHED;
	}
	
	/* Create the file list reader. */
	pReader = &frd.rd[frd.nrOfReaders];
	pFLReader = &pReader->reader.list;
	/* Test if the file list really exists as soon as possible. */
	pFLReader->pFList = fopen(strFileList, "r");
	if(pFLReader->pFList == NULL)
	{
		OscLog(ERROR, "%s: Unable to open file list! (%s)\n",
				__func__, strFileList);
		return -EUNABLE_TO_OPEN_FILE;
	}
		
	frd.nrOfReaders++;
	
	pReader->enType = FRD_READER_TYPE_LIST;
	strcpy(pFLReader->strFileList, strFileList);
	*phReaderHandle = (void*)pReader;
	
	return SUCCESS;
}

/*********************************************************************//*!
 * @brief Get the current file name of a list reader.
 * 
 * @param pReader Handle to the file list reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static inline void OscFrdListGetCurrentFileName(
		const struct OSC_FRD_FILELIST_READER *pReader,
		char strCurName[])
{
	strcpy(strCurName, pReader->curFileName);
}

/* -------------------------- Constant reader ------------------------------*/
/*********************************************************************//*!
 * @brief Parses in the parameters of a file name list reader.
 * 
 * @param pConfigF Open handle to the frd config file advanced to the
 * position where the options of the constant reader are located.
 * @param pReader Reader object structure to complete with the information
 * in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscFrdParseConstantReader(FILE *pConfigF,
		struct OSC_FRD_CONSTANT_READER * pReader)
{
	int                         assigned;
		
	/* The only parameter is the constant file name. */
	assigned = fscanf(pConfigF, "FILENAME = %s\n",
			pReader->strFN);
	if(unlikely(assigned != 1))
	{
		return -EFRD_PARSING_FAILURE;
	}
	
	return SUCCESS;
}

OSC_ERR OscFrdCreateConstantReader(void **phReaderHandle, char strFN[])
{
	struct OSC_FRD_READER           *pReader;
	struct OSC_FRD_CONSTANT_READER  *pConstReader;
	
	/* Input validation */
	if(unlikely((phReaderHandle == NULL) ||
				(strFN == NULL) ||
				(*strFN == '\0')))
	{
		OscLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
					__func__, phReaderHandle, strFN);
		return -EINVALID_PARAMETER;
	}
	
	if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
	{
		OscLog(ERROR, "%s: Maximum number of readers reached!\n",
				__func__);
		return -EFRD_MAX_NR_READERS_REACHED;
	}

	/* Create the file list reader. */
	pReader = &frd.rd[frd.nrOfReaders];
	frd.nrOfReaders++;
	
	pReader->enType = FRD_READER_TYPE_CONSTANT;
	pConstReader = &pReader->reader.constant;
	strcpy(pConstReader->strFN, strFN);
	
	*phReaderHandle = (void*)pReader;
	
	return SUCCESS;
}

/*********************************************************************//*!
 * @brief Get the current file name of a constant reader.
 * 
 * @param pReader Handle to the file sequence reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static void OscFrdConstGetCurrentFileName(
		const struct OSC_FRD_CONSTANT_READER *pReader,
		char strCurName[])
{
	/* Simply copy the constant file name. */
	strcpy(strCurName, pReader->strFN);
}

OSC_ERR OscFrdCreateReader(void ** phReaderHandle,
		const char strReaderConfigFile[])
{
	FILE                        *pConfigF;
	struct OSC_FRD_READER           *pReader;
	int                         assigned;
	OSC_ERR                     err;
	char                        strTemp[1024];
	
	if(unlikely((strReaderConfigFile == NULL) ||
			(strReaderConfigFile[0] == '\0') ||
			(phReaderHandle == NULL)))
	{
		OscLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
				__func__, phReaderHandle, strReaderConfigFile);
		return -EINVALID_PARAMETER;
	}
	
	if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
	{
		OscLog(ERROR, "%s: Maximum number of readers reached!\n",
				__func__);
		return -EFRD_MAX_NR_READERS_REACHED;
	}
	
	pConfigF = fopen(strReaderConfigFile, "r");
	if(unlikely(pConfigF == NULL))
	{
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	pReader = &frd.rd[frd.nrOfReaders];
	frd.nrOfReaders++;
	
	/* Read in the configuration from the config file. */
	
	/* First, check the reader type. */
	assigned = fscanf(pConfigF, "READER_TYPE = %s\n",
			strTemp);
	if(unlikely(assigned != 1))
	{
		err = -EFRD_PARSING_FAILURE;
		goto exit_fail;
	}
	
	if(strcmp(strTemp, "FRD_FILELIST_READER") == 0)
	{
		/* File List Reader. */
		err = OscFrdParseListReader(pConfigF,
				&pReader->reader.list);
		
		if(err != SUCCESS)
		{
			OscLog(ERROR, "%s: Error parsing list reader config"
					"(%d)!\n", __func__, err);
			goto exit_fail;
		}
		pReader->enType = FRD_READER_TYPE_LIST;
	}
	else if(strcmp(strTemp, "FRD_SEQUENCE_READER") == 0)
	{
		/* Sequential Reader. */
		err = OscFrdParseSequentialReader(pConfigF,
				&pReader->reader.seq);
		if(err != SUCCESS)
		{
			OscLog(ERROR, "%s: Error parsing sequential reader config"
					"(%d)!\n", __func__, err);
			goto exit_fail;
		}
		pReader->enType = FRD_READER_TYPE_SEQUENCE;
	} else if(strcmp(strTemp, "FRD_CONSTANT_READER") == 0)
	{
		/* Constant Reader. */
		err = OscFrdParseConstantReader(pConfigF,
				&pReader->reader.constant);
		if(err != SUCCESS)
		{
			OscLog(ERROR, "%s: Error parsing constant reader config"
					"(%d)!\n", __func__, err);
			goto exit_fail;
		}
		pReader->enType = FRD_READER_TYPE_CONSTANT;
	}
				
	/* Success. Return the handle*/
	*phReaderHandle = (void*)pReader;
	err = SUCCESS;
	
exit_fail:
	fclose(pConfigF);
	return err;
}

OSC_ERR OscFrdGetCurrentFileName(const void *hReaderHandle,
		char strCurName[])
{
	struct OSC_FRD_READER *pReader;
	
	/* Input validation. */
	if(unlikely((hReaderHandle == NULL) || (strCurName == NULL)))
	{
		OscLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
				__func__, hReaderHandle, strCurName);
		return -EINVALID_PARAMETER;
	}
	pReader = (struct OSC_FRD_READER*)hReaderHandle;

	switch(pReader->enType)
	{
	case FRD_READER_TYPE_LIST:
		OscFrdListGetCurrentFileName(&pReader->reader.list, strCurName);
		break;
	case FRD_READER_TYPE_SEQUENCE:
		OscFrdSeqGetCurrentFileName(&pReader->reader.seq, strCurName);
		break;
	case FRD_READER_TYPE_CONSTANT:
		OscFrdConstGetCurrentFileName(&pReader->reader.constant, strCurName);
		break;
	default:
		OscLog(ERROR, "%s: Unsupported reader type configured (%d)!\n",
				__func__, pReader->enType);
		return -EFRD_INVALID_VALUES_CONFIGURED;
	}
	
	return SUCCESS;
}
