/*! @file frd_pub.h
 * @brief API definition for filename reader module
 * 
 * Frd allows reads a configuration file containing the syntax rules
 * for the files to be generated. These have to be of the form 
 * 'prefix + seq_nr + suffix', e.g. 'test/img001.bmp'.
 * 
 * The config file must be specified during creation of a reader. It
 * has the following form (for above example):
 * 
 * FileNamePrefix = test/img
 * 
 * FileNameSeqNrDigits = 3
 * 
 * FileNameSuffix = .bmp
 * 
 * @author Markus Berner
 ************************************************************************/
#ifndef FRD_PUB_H_
#define FRD_PUB_H_

#include "framework_error.h"

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvFrdErrors
{
    EFRD_PARSING_FAILURE = LCV_FRD_ERROR_OFFSET,
    EFRD_MAX_NR_READERS_REACHED,
    EFRD_INVALID_VALUES_CONFIGURED
};

/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVFrdCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVFrdDestroy(void *hFw);


/*********************************************************************//*!
 * @brief Create a filename reader.
 *
 * This will parse the supplied config file for the parameters to 
 * assemble the desired file names depending on the time step. It is also
 * possible to create a reader without going through a config file.
 * 
 * The config file for a sequence reader looks like this:
 * READER_TYPE = FRD_SEQUENCE_READER
 * FILENAME_PREFIX = <PREFIX>
 * FILENAME_SEQ_NR_DIGITS = <# DIGITS>
 * FILENAME_SUFFIX = <SUFFIX>
 * 
 * The config file for a file-list reader looks like this:
 * READER_TYPE = FRD_FILELIST_READER
 * FILENAME_LIST = <Path to file-list>
 * 
 * The config file for a constant reader looks like this:
 * READER_TYPE = FRD_CONSTANT_READER
 * FILENAME = <FILE-NAME>
 * 
 * @see LCVFrdCreateFileListReader
 * @see LCVFrdCreateSequenceReader
 * 
 * @param phReaderHandle The handle to the reader is returned over
 * this pointer.
 * @param strReaderConfigFile The file name of the config file where
 * the reader parameters are stored.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVFrdCreateReader(void ** phReaderHandle, 
        const char strReaderConfigFile[]);

/*********************************************************************//*!
 * @brief Create a file-list filename reader directly.
 *
 * Create a filename reader for file-lists directly without parsing a
 * config file.
 * @see LCVFrdCreateReader
 * @see LCVFrdCreateSequenceReader
 * @see LCVFrdCreateConstantReader
 * 
 * @param phReaderHandle The handle to the reader is returned over
 * this pointer.
 * @param strFileList The name of the file list this reader should
 * be associated with.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVFrdCreateFileListReader(void **phReaderHandle,
        const char strFileList[]);

/*********************************************************************//*!
 * @brief Create a sequence filename reader directly.
 *
 * Create a filename reader for sequential file names directly without 
 * parsing a config file.
 * @see LCVFrdCreateReader
 * @see LCVFrdCreateFileListReader
 * @see LCVFrdCreateConstantReader
 * 
 * @param phReaderHandle The handle to the reader is returned over
 * this pointer.
 * @param strPrefix The prefix of the sequential file names. This also
 * includes the path information.
 * @param seqNrDigits The number of digits used for the sequence number.
 * @param strSuffix The suffix appended to the file name. This includes
 * the file type extension.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVFrdCreateSequenceReader(void **phReaderHandle,
        char strPrefix[], 
        int seqNrDigits,
        char strSuffix[]);

/*********************************************************************//*!
 * @brief Create a constant filename reader directly.
 *
 * Create a filename reader which always reads the same file name.
 * Good for simple testing.
 * @see LCVFrdCreateReader
 * @see LCVFrdCreateFileListReader
 * @see LCVFrdCreateSequenceReader
 * 
 * @param phReaderHandle The handle to the reader is returned over
 * this pointer.
 * @param strFN The constant file name to read.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVFrdCreateConstantReader(void **phReaderHandle, char strFN[]);

/*********************************************************************//*!
 * @brief Returns the file name corresponding to the current time step.
 *
 * @param hReaderHandle Handle to the reader.
 * @param strCurName The current file name is written into this string.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVFrdGetCurrentFileName(const void *hReaderHandle, 
        char strCurName[]);


#endif /*FRD_PUB_H_*/
