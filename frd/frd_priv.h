/*! @file frd_priv.h
 * @brief Private filename reader module definition
 * 
 * @author Markus Berner
 ************************************************************************/
#ifndef FRD_PRIV_H_
#define FRD_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef LCV_HOST
#include <framework_types_host.h>
#else
#include <framework_types_target.h>
#endif /* LCV_HOST */

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
struct LCV_FRD_SEQUENCE_READER
{
    /*! @brief The prefix of the filenames. */
    char strPrefix[MAX_PREFIX_LEN];    
    /*! @brief To how many digits the sequence number is expanded. */
    int seqNrDigits;     
    /*! @brief The suffix of the filenames. */
    char strSuffix[MAX_SUFFIX_LEN];
};

/*! @brief Reader object struct for a filelist reader*/ 
struct LCV_FRD_FILELIST_READER
{
	/* @brief The file name of the file list. */
	char strFileList[1024];
	/* @brief The open file handle to the file list. */
	FILE * pFList;
	/* @brief The current file name. */
	char curFileName[1024];
};

/*! @brief Reader object struct for a constant reader*/ 
struct LCV_FRD_CONSTANT_READER
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
struct LCV_FRD_READER
{
	/*! @brief The actual type of this reader. */
	enum EnFilenameReaderType enType;
	/*! @brief The actual data of the reader, depending on its type. */
	union LCV_FRD_READER_DATA
	{
    	/*! @brief Sequence file name reader. */
    	struct LCV_FRD_SEQUENCE_READER seq;
    	/*! @brief List file name reader. */
    	struct LCV_FRD_FILELIST_READER list;
    	/*! @brief Constant file name reader. */
    	struct LCV_FRD_CONSTANT_READER constant;
	} reader;
};

/*!@brief Filename reader module object struct */
struct LCV_FRD
{
    uint16 nrOfReaders;		/*!< @brief Number of managed readers */
	/*! @brief Reader object array */
    struct LCV_FRD_READER rd[MAX_NR_READERS];
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
static LCV_ERR LCVFrdParseSequentialReader(FILE *pConfigFile, 
		struct LCV_FRD_SEQUENCE_READER * pReader);

/*********************************************************************//*!
 * @brief Parses in the parameters of a file name list reader.
 * 
 * @param pConfigF Open handle to the frd config file advanced to the
 * position where the options of the sequence reader are located.
 * @param pReader Reader object structure to complete with the information
 * in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static LCV_ERR LCVFrdParseListReader(FILE *pConfigF, 
		struct LCV_FRD_FILELIST_READER * pReader);

/*********************************************************************//*!
 * @brief Parses in the parameters of a file name list reader.
 * 
 * @param pConfigF Open handle to the frd config file advanced to the
 * position where the options of the constant reader are located.
 * @param pReader Reader object structure to complete with the information
 * in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static LCV_ERR LCVFrdParseConstantReader(FILE *pConfigF, 
		struct LCV_FRD_CONSTANT_READER * pReader);
		
/*********************************************************************//*!
 * @brief Get the current file name of a list reader.
 * 
 * @param pReader Handle to the file list reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static inline void LCVFrdListGetCurrentFileName(
		const struct LCV_FRD_FILELIST_READER *pReader, 
        char strCurName[]);

/*********************************************************************//*!
 * @brief Get the current file name of a sequence reader.
 * 
 * @param pReader Handle to the file sequence reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/
static void LCVFrdSeqGetCurrentFileName(
		const struct LCV_FRD_SEQUENCE_READER *pReader, 
        char strCurName[]);

/*********************************************************************//*!
 * @brief Get the current file name of a constant reader.
 * 
 * @param pReader Handle to the file sequence reader.
 * @param strCurName Current file name is written to this string.
 *//*********************************************************************/        
static void LCVFrdConstGetCurrentFileName(
		const struct LCV_FRD_CONSTANT_READER *pReader, 
        char strCurName[]);

/*********************************************************************//*!
 * @brief The callback being invoked every simulation cycle.
 * 
 * Execute periodic tasks depending on the reader type.
 *//*********************************************************************/
static void LCVFrdSimCycleCallback();

/*********************************************************************//*!
 * @brief Read the next file name from the file list.
 *//*********************************************************************/
static void LCVFrdListFetchNextFileName();

#endif /* FRD_PRIV_H_ */
