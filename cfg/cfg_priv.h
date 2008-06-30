/*! @file cfg_priv.h
 * @brief Private configuration file module definitions with object structure
 * 
 */
#ifndef CFG_PRIV_H_
#define CFG_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef OSC_HOST
    #include <oscar_types_host.h>
#else
    #include <oscar_types_target.h>
#endif /* OSC_HOST */

#include <log/log_pub.h>

/*! @brief Macro defining the maximal number of open configuration files */
#define CONFIG_FILE_MAX_NUM 3
/*! @brief Macro defining the maximal file name string size */
#define CONFIG_FILE_NAME_MAX_SIZE CONFIG_VAL_MAX_SIZE
/*! @brief Macro defining the section suffix string */
#define CONFIG_FILE_SECTION_SUFFIX "\n"
/*! @brief Macro defining the tag suffix string */
#define CONFIG_FILE_TAG_SUFFIX ": "
/*! @brief Macro defining the tag suffix string */
#define CONFIG_FILE_LABEL_PREFIX "\n"
/*! @brief Macro defining the escape characters for the string scanning */
#define CONFIG_FILE_ESCAPE_CHARS "%1024[^\n]"


/*! @brief Structure containing the file content */
struct CFG_FILE_CONTENT {
	char *data; /* +1 to add string termination \0 */
	unsigned int dataSize; /* allocated memory of data array */
	char fileName[CONFIG_FILE_NAME_MAX_SIZE];
};

/*! @brief Config File Content handels. */
struct OSC_CFG { 
    uint16 nrOfContents;		/*!< @brief Number of managed contents */
	struct CFG_FILE_CONTENT contents[CONFIG_FILE_MAX_NUM];
};



/*======================= Private methods ==============================*/

/*********************************************************************//*!
 * @brief Helper function to write the file content
 * 
 * @param hFileContent Handle to the File content.
 * @param all True if the whole buffer shall be written.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgFlushContentHelper(const CFG_FILE_CONTENT_HANDLE hFileContent, bool all);

/*********************************************************************//*!
 * @brief Helper function to find the beginning of the value string in the file
 * 
 * @param contentIndex Index to the File content.
 * @param pKey The name of the section and tag.
 * @param pPStrVal Return pointer to the value in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetValPtr(
		const unsigned int 	contentIndex, 
		const struct CFG_KEY *pKey,
		char **pPStrVal);

/*********************************************************************//*!
 * @brief Finds subString in String
 * 
 * @param subString Pointer to substring.
 * @param subStringLen Substring length.
 * @param string Pointer to string.
 * @return Pointer to char in string after subString, NULL if no subString is found
 *//*********************************************************************/
char* OscCfgIsSubStr(
		const char *subString, 
		const size_t subStringLen, 
		const char *string);

/*********************************************************************//*!
 * @brief Finds label at the beginning of the line of a text. 
 * 
 * @param label Pointer to label. If NULL, beginning of text is returned 
 * @param labelSuffix Pointer to label suffix (e.g. ": " for tags).
 * @param text Pointer to text.
 * @return Pointer to char in test after label suffix, NULL if label not found
 *//*********************************************************************/
char* OscCfgFindNewlineLabel(
		const char* label, 
		const char* labelSuffix, 
		char* text);

/*********************************************************************//*!
 * @brief Replaces oldStr with newStr in text
 * 
 * @param contentIndex Index to content structure.
 * @param oldStr Pointer to the old string, which will be replaced.
 * @param newStr Pointer to new string.
 * @param text Pointer to text.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgReplaceStr(
		const unsigned int 	contentIndex, 
		const char *oldStr, 
		const char *newStr, 
		char* text);


/*********************************************************************//*!
 * @brief Appends label to the file content
 * 
 * @param text Pointer to text.
 * @param maxTextLen maxFileSize + 1.
 * @param label Pointer to label. If NULL, nothing is appended
 * @param labelPrefix Pointer to labelPrefix.
 * @param labelSuffix Pointer to labelSuffix.
 * @return pointer to char after labelSuffix
 *//*********************************************************************/
char* OscCfgAppendLabel(
		char* text, 
		const unsigned int maxTextLen, 
		const char* label, 
		const char* labelPrefix,
		const char* labelSuffix);


/*********************************************************************//*!
 * @brief Helper to find the first occurence of an invalid character
 * 
 * @param str Character array to search in.
 * @param strSize Array length of str.
 * @return index of first invalid char.
 *//*********************************************************************/
unsigned int OscCfgFindInvalidChar(const char *str, const unsigned int strSize);

#endif /*CFG_PRIV_H_*/
