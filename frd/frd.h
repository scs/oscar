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

/*! @file frd.h
 * @brief Private filename reader module definition
 */

#ifndef FRD_PRIV_H_
#define FRD_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <oscar.h>

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

/*********************************************************************//*!
 * @brief Parses in the parameters of a sequential file name reader.
 * 
 * @param pConfigFile Open handle to the frd config file advanced to the
 * position where the options of the sequence reader are located.
 * @param pReader Reader object structure to complete with the information
 * in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscFrdParseSequentialReader(FILE *pConfigFile,
		struct OSC_FRD_SEQUENCE_READER * pReader);

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
		struct OSC_FRD_FILELIST_READER * pReader);

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
		struct OSC_FRD_CONSTANT_READER * pReader);
		
/*********************************************************************//*!
 * @brief Get the current file name of a list reader.
 * 
 * @param pReader Handle to the file list reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static inline void OscFrdListGetCurrentFileName(
		const struct OSC_FRD_FILELIST_READER *pReader,
		char strCurName[]);

/*********************************************************************//*!
 * @brief Get the current file name of a sequence reader.
 * 
 * @param pReader Handle to the file sequence reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static void OscFrdSeqGetCurrentFileName(
		const struct OSC_FRD_SEQUENCE_READER *pReader,
		char strCurName[]);

/*********************************************************************//*!
 * @brief Get the current file name of a constant reader.
 * 
 * @param pReader Handle to the file sequence reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static void OscFrdConstGetCurrentFileName(
		const struct OSC_FRD_CONSTANT_READER *pReader,
		char strCurName[]);

/*********************************************************************//*!
 * @brief The callback being invoked every simulation cycle.
 * 
 * Execute periodic tasks depending on the reader type.
 *//*********************************************************************/
static void OscFrdSimCycleCallback();

/*********************************************************************//*!
 * @brief Read the next file name from the file list.
 *//*********************************************************************/
static void OscFrdListFetchNextFileName();

#endif /* FRD_PRIV_H_ */
