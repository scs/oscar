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
 * @brief Configuration file module implementation for target and host
 * 
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "oscar.h"

struct OscModule OscModule_cfg = {
	.name = "cfg",
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

/*! @brief Macro defining the maximal number of open configuration files */
#define CONFIG_FILE_MAX_NUM 5
/*! @brief Macro defining the maximal file name string size */
#define CONFIG_FILE_NAME_MAX_SIZE CONFIG_VAL_MAX_SIZE
/*! @brief Macro defining the section suffix string */
#define CONFIG_FILE_SECTION_SUFFIX "\n"
/*! @brief Macro defining the tag suffix string */
#define CONFIG_FILE_TAG_SUFFIX ":"
/*! @brief Macro defining the tag suffix string */
#define CONFIG_FILE_LABEL_PREFIX "\n"
/*! @brief Macro defining the escape characters for the string scanning */
#define CONFIG_FILE_ESCAPE_CHARS "%1023[^\n]"

/*! @brief Structure containing the file content */
struct CFG_FILE_CONTENT {
	char *data; /* +1 to add string termination \0 */
	unsigned int dataSize; /* allocated memory of data array */
	char fileName[CONFIG_FILE_NAME_MAX_SIZE];
};

/*! @brief Config File Content handels. */
struct OSC_CFG {
	uint16 nrOfContents;        /*!< @brief Number of managed contents */
	struct CFG_FILE_CONTENT contents[CONFIG_FILE_MAX_NUM];
};

/*======================= Private methods ==============================*/

/*********************************************************************//*!
 * @brief Helper function to find the beginning of the value string in the file
 * 
 * @param contentIndex Index to the File content.
 * @param pKey The name of the section and tag.
 * @param pPStrVal Return pointer to the value in the file.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OscFunctionDeclare( static OscCfgGetValPtr,
		const unsigned int  contentIndex,
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
static char* OscCfgIsSubStr(
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
static char* OscCfgFindNewlineLabel(
		const char* label,
		const char* labelSuffix,
		char* text);

/*********************************************************************//*!
 * @brief Finds start of next section.
 * 
 * @param pSectionStart Start of section that shall be analysed.
 * @return Pointer to start of next section label. NULL if error
 *//*********************************************************************/
static char* OscCfgFindNextSectionStart(
		char* pSectionStart);


/*********************************************************************//*!
 * @brief Replaces oldStr with newStr in text
 * 
 * @param contentIndex Index to content structure.
 * @param oldStr Pointer to the old string, which will be replaced.
 * @param newStr Pointer to new string.
 * @param text Pointer to text.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OscFunctionDeclare( static OscCfgReplaceStr,
		const unsigned int  contentIndex,
		const char *oldStr,
		const char *newStr,
		char* text);


/*********************************************************************//*!
 * @brief Inserts Text to the file content
 * 
 * @param contentIndex Index to content structure.
 * @param insertPosition Pointer to file text position, where text shall be inserted
 * @param insertText Pointer to text string that shall be inserted. If NULL, nothing is inserted
 * @return pointer to char after insertedText
 *//*********************************************************************/
static char* OscCfgInsertText(
		const unsigned int  contentIndex,
		char* insertPosition,
		const char* insertText);

/*!
	@brief Get the value of a U-Boot environment variable.
	@param key The name of the variable.
	@param value Is set to point to the value in a static buffer.
*/
OscFunctionDeclare(static getUBootEnv, char * key, char ** value)

/*!
	@brief Parse an integer from a string.
	@param str String containing the integer.
	@param res Is set to the number parsed from the string.
*/
OscFunctionDeclare(static parseInteger, char * str, int * res)

/*!
	@brief Get the version of the running uClinux version.
	@param res Is set to point to the value in a static buffer.
	@param the other variables to matching ints parsed from the string
*/
OscFunctionDeclare(static getUClinuxVersion, char ** res, int* major, int* minor, int* patch_level, int* rc)


/*! @brief The module singelton instance. */
struct OSC_CFG cfg;

/*======================= Public methods ==============================*/

OscFunction( OscCfgRegisterFile,
		CFG_FILE_CONTENT_HANDLE *pFileContentHandle,
		const char *strFileName,
		const unsigned int maxFileSize)

	FILE    *pCfgFile;
	size_t  fileSize;
	unsigned int    actIndex;
	
	/* find an unused file index */
	if(cfg.nrOfContents==0) {
		for(uint16 i=0; i<CONFIG_FILE_MAX_NUM; ++i) 
			cfg.contents[i].data=NULL;
		actIndex=0;
	} else {
		actIndex=0;
		while(actIndex<CONFIG_FILE_MAX_NUM && cfg.contents[actIndex].data)
			++actIndex;
	}
	
	/* check preconditions */
	OscAssert_e(pFileContentHandle && strFileName, -ECFG_ERROR);
	OscAssert_e(strFileName[0] != '\0', -ECFG_INVALID_FUNC_PARAMETER);
	OscAssert_em(cfg.nrOfContents < CONFIG_FILE_MAX_NUM && actIndex < CONFIG_FILE_MAX_NUM, ECFG_NO_HANDLES,  "too many handles open (%d=%d) !\n", cfg.nrOfContents, CONFIG_FILE_MAX_NUM);

	/* copy file name and open file */
	pCfgFile = fopen(strFileName, "r");
	OscAssert_em(pCfgFile, -ECFG_UNABLE_TO_OPEN_FILE, "Unable to open config file %s!\n", strFileName);

	/* save data in content manager */
	cfg.contents[actIndex].data = malloc(maxFileSize + 1);
	OscAssert_em(cfg.contents[actIndex].data, -ECFG_ERROR, "could not allocate memory!\n");

	fileSize = fread(cfg.contents[actIndex].data, sizeof(char), maxFileSize + 1, pCfgFile);
	if (fileSize == maxFileSize + 1 || !feof(pCfgFile) || ferror(pCfgFile))
	{
		OscLog(ERROR, "%s: config file %s too long, or unable to open it! (ferror/feof: %i, %i)\n",
				__func__, strFileName, ferror(pCfgFile), feof(pCfgFile));
		fclose(pCfgFile);
		free(cfg.contents[actIndex].data);
		cfg.contents[actIndex].data=NULL;
		OscFail_e(-ECFG_UNABLE_TO_OPEN_FILE);
	}
	fclose(pCfgFile);
	cfg.nrOfContents++;
	
	/* append string termination */
	cfg.contents[actIndex].data[fileSize] = '\0';
	OscLog(DEBUG, "%s: string length set to %d\n",
			__func__, fileSize);

	cfg.contents[actIndex].dataSize = maxFileSize + 1;
	*pFileContentHandle = actIndex+1; /* return content handle */
	strcpy(cfg.contents[actIndex].fileName, strFileName); /* store file name */
	
OscFunctionEnd()

OscFunction( OscCfgUnregisterFile, CFG_FILE_CONTENT_HANDLE pFileContentHandle)
	
	OscAssert_e(pFileContentHandle>0 && pFileContentHandle<=CONFIG_FILE_MAX_NUM
			&& cfg.contents[pFileContentHandle-1].data, -EINVALID_PARAMETER);
	
	free(cfg.contents[pFileContentHandle-1].data);
	cfg.contents[pFileContentHandle-1].data=NULL;
	
	--cfg.nrOfContents;
	
OscFunctionEnd()

OscFunction( OscCfgDeleteAll, void)
	/* do nothing */
OscFunctionEnd()

OscFunction( OscCfgFlushContent, const CFG_FILE_CONTENT_HANDLE hFileContent)

	FILE        *pCfgFile;
	char        *strFileName;
	size_t      fileSize;
	unsigned int strSize, index;
	
	/* check preconditions */
	OscAssert_e(hFileContent && hFileContent <= CONFIG_FILE_MAX_NUM, -ECFG_INVALID_FUNC_PARAMETER)
	index = hFileContent-1;
	strSize = strlen(cfg.contents[index].data); /* string size without \0 */
	OscAssert_em(strSize < cfg.contents[index].dataSize, -ECFG_ERROR, "invalid content size!\n");
	
	/* open file */
	strFileName = cfg.contents[index].fileName;
	pCfgFile = fopen(strFileName, "w+");
	OscAssert_em(pCfgFile, -ECFG_UNABLE_TO_OPEN_FILE, "Unable to open config file %s!\n", strFileName);

	fileSize = fwrite(cfg.contents[index].data, sizeof(char), strSize, pCfgFile);   /* write string */

	fclose(pCfgFile);
	OscAssert_em(fileSize == strSize, -ECFG_UNABLE_TO_WRITE_FILE, "could not write data!\n");
	
OscFunctionEnd()

OscFunction( OscCfgFlushContentAll, const CFG_FILE_CONTENT_HANDLE hFileContent)

	/* Does same as OscFlushContent - old EEPROM flushing depreciated */
	return OscCfgFlushContent(hFileContent);

OscFunctionEnd()
		
OscFunction( OscCfgGetStr,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		struct CFG_VAL_STR *pVal)

	char *pStrVal = NULL;
	int  stdErr;
	OSC_ERR err;
	pVal->str[0] = '\0'; /* default */
	
	/* check preconditions */
	OscAssert_em(pKey && pVal && hFileContent && hFileContent <= CONFIG_FILE_MAX_NUM, -ECFG_INVALID_FUNC_PARAMETER, "Invalid parameter.(%d, 0x%x, 0x%x)\n", hFileContent, pKey, pVal);

	/* find value pointer */
	err = OscCfgGetValPtr(hFileContent-1, pKey, &pStrVal);
	OscAssert_e (err == SUCCESS, err)
	/* function may return null pointer */
	OscAssert_e(pStrVal, -ECFG_INVALID_KEY);
	
	/* scan value at beginning of file */
	{
		char ignoreStr[80]; /* copy white spaces to this ignored string */
		int result = sscanf(pStrVal, "%80[ \t]", ignoreStr);
		if (result == 1) {
			pStrVal = &pStrVal[strlen(ignoreStr)]; /* move pointer forward */
		}
	}
	stdErr = sscanf(pStrVal, CONFIG_FILE_ESCAPE_CHARS, pVal->str);

	if (stdErr == 0 || stdErr == EOF) { 
		// Empty string, return '\0' string...
		pVal->str[0] = '\0';
	}

	OscAssert_em(strlen(pVal->str) < 1024, -ECFG_INVALID_VAL, "value too long (%i)! (TAG=%s)\n", strlen(pVal->str), pKey->strTag);
	OscLog(DEBUG, "Read Tag '%s': Value '%s'\n", pKey->strTag, pVal->str);

OscFunctionEnd()
	
OscFunction( OscCfgGetStrRange,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		struct CFG_VAL_STR *pVal,
		const uint32 len,
		const char* pDefault)

	OSC_ERR err;
	err = OscCfgGetStr( hFileContent, pKey, pVal);
	
    if( (err == SUCCESS) && (strlen(pVal->str) > len) && (len != -1))
    {
    	OscFail_e(-ECFG_INVALID_RANGE);
	}  
	if( err != SUCCESS && pDefault != NULL)
	{
    	strcpy( pVal->str, pDefault);
    	err = ECFG_USED_DEFAULT;
	}
	return err;

OscFunctionEnd()
	
OscFunction( OscCfgSetStr,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const char *strNewVal)

	char *pStrSecStart = NULL;  /* points to beginning of section content */
	char *pStrVal = NULL;
	struct CFG_VAL_STR oldVal;
	int  stdErr;
	unsigned int index;
	OSC_ERR err;
	
	/* check preconditions */
	OscAssert_em(pKey && hFileContent && hFileContent <= CONFIG_FILE_MAX_NUM, -ECFG_INVALID_FUNC_PARAMETER, "Invalid parameter.(%d, 0x%x, 0x%x)\n", hFileContent, pKey, strNewVal);
	index = hFileContent - 1;

	/* find value pointer */
	err = OscCfgGetValPtr(index, pKey, &pStrVal);
	OscAssert_e(err == SUCCESS, err);

	if (pStrVal == NULL) /* if section or tag not found */
	{ /* value inserted */
		/* find section */
		pStrSecStart = OscCfgFindNewlineLabel(pKey->strSection, CONFIG_FILE_SECTION_SUFFIX, cfg.contents[index].data);
		if(pStrSecStart == NULL)
		{
			/* append section label to end of file and get pointer to content string termination */
			pStrSecStart = OscCfgInsertText(index, &cfg.contents[index].data[strlen(cfg.contents[index].data)], CONFIG_FILE_LABEL_PREFIX);
			pStrSecStart = OscCfgInsertText(index, pStrSecStart/*append*/, pKey->strSection);
			pStrSecStart = OscCfgInsertText(index, pStrSecStart/*append*/, CONFIG_FILE_SECTION_SUFFIX);
			/* postfix '\n' added with tag*/
			OscAssert_em(pStrSecStart, -ECFG_ERROR, "Unable to write Section '%s'\n", pKey->strSection);
		}
		/* insert tag label at beginning of section and get pointer to content string termination */
		pStrVal = OscCfgInsertText(index, pStrSecStart, pKey->strTag);
		pStrVal = OscCfgInsertText(index, pStrVal/*append*/, CONFIG_FILE_TAG_SUFFIX);
		pStrVal = OscCfgInsertText(index, pStrVal/*append*/, strNewVal);
		pStrVal = OscCfgInsertText(index, pStrVal/*append*/, "\n");
	}
	else { /* value replaced */
		/* scan value after tag */
		stdErr = sscanf(pStrVal, CONFIG_FILE_ESCAPE_CHARS, oldVal.str);
		if ((stdErr == EOF) || (stdErr == 0))
		{
			oldVal.str[0] = '\0'; /* set string termination */
		}
	
		/* insert the new string into config file */
		err = OscCfgReplaceStr(index, oldVal.str, strNewVal, pStrVal);
		OscAssert_em(err == SUCCESS, err, "Unable to write Tag '%s': Value '%s'\n", pKey->strTag, strNewVal);
	}
	OscLog(DEBUG, "Wrote Tag '%s': Value '%s'\n", pKey->strTag, strNewVal);

OscFunctionEnd()
	
OscFunction( OscCfgSetBool,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const bool val)

	if(val)
	{
		return OscCfgSetStr( hFileContent, pKey, "TRUE");
	}
	else
	{
		return OscCfgSetStr( hFileContent, pKey, "FALSE");
	}		
OscFunctionEnd()
	
OscFunction( OscCfgSetInt,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const int val)

	char strVal[ CONFIG_VAL_MAX_SIZE];
	
	sprintf( strVal, "%d", val);
	return OscCfgSetStr( hFileContent, pKey, strVal);
	
OscFunctionEnd()
	
OscFunction( OscCfgGetInt,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal)

	OSC_ERR err;
	int32 tmpVal;
	err = OscCfgGetInt32(hFileContent, pKey, &tmpVal);
	if (err == SUCCESS)
	{
			*iVal = (int16)tmpVal;
	}
	return err;
	
OscFunctionEnd()
	
OscFunction( OscCfgGetUInt8,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint8 *iVal)

	OSC_ERR err;
	uint32 tmpVal;
	err = OscCfgGetUInt32(hFileContent, pKey, &tmpVal);
	if (err == SUCCESS)
	{
			*iVal = (uint8)tmpVal;
	}
	return err;

OscFunctionEnd()

OscFunction( OscCfgGetIntRange,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal,
		const int16 min,
		const int16 max,
		const int16 def)

	OSC_ERR err;
	int32 tmpVal;
	err = OscCfgGetInt32Range(hFileContent, pKey, &tmpVal, min, max, (int32)def);
	*iVal = (int16)tmpVal;
	return err;

OscFunctionEnd()
	
OscFunction( OscCfgGetInt8Range,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int8 *iVal,
		const int8 min,
		const int8 max,
		const int8 def)

	OSC_ERR err;
	int32 tmpVal;
	err = OscCfgGetInt32Range(hFileContent, pKey, &tmpVal, min, max, (int32)def);
	*iVal = (int8)tmpVal;
	return err;

OscFunctionEnd()
	
OscFunction( OscCfgGetUInt8Range,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint8 *iVal,
		const uint8 min,
		const uint8 max,
		const uint8 def)

	OSC_ERR err;
	uint32 tmpVal;
	err = OscCfgGetUInt32Range(hFileContent, pKey, &tmpVal, min, max, (uint32)def);
	*iVal = (uint8)tmpVal;
	return err;
	
OscFunctionEnd()
	
OscFunction( OscCfgGetInt16Range,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal,
		const int16 min,
		const int16 max,
		const int16 def)

	OSC_ERR err;
	int32 tmpVal;
	err = OscCfgGetInt32Range(hFileContent, pKey, &tmpVal, min, max, (int32)def);
	*iVal = (int16)tmpVal;
	return err;
	
OscFunctionEnd()
			
OscFunction( OscCfgGetUInt16Range,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint16 *iVal,
		const uint16 min,
		const uint16 max,
		const uint16 def)

	OSC_ERR err;
	uint32 tmpVal;
	err = OscCfgGetUInt32Range(hFileContent, pKey, &tmpVal, (uint32)min, (uint32)max, (uint32)def);
	*iVal = (uint16)tmpVal;
	return err;

OscFunctionEnd()

OscFunction( OscCfgGetInt32,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int32 *iVal)

	struct CFG_VAL_STR val;
	OSC_ERR err;
	/* check preconditions */
	OscAssert_em(pKey && iVal, -ECFG_INVALID_FUNC_PARAMETER, "Invalid parameter.(%d, 0x%x, 0x%x)\n", hFileContent, pKey, iVal);

	err = OscCfgGetStr(hFileContent, pKey, &val);
	if (err == SUCCESS)
	{
		*iVal = (int32)atoi(val.str);
	}
	return err;

OscFunctionEnd()
	
OscFunction( OscCfgGetUInt32,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint32 *iVal)

	struct CFG_VAL_STR val;
	OSC_ERR err;
	/* check preconditions */
	OscAssert_em(pKey && iVal, -ECFG_INVALID_FUNC_PARAMETER, "Invalid parameter.(%d, 0x%x, 0x%x)\n", hFileContent, pKey, iVal);

	err = OscCfgGetStr(hFileContent, pKey, &val);
	if (err == SUCCESS)
	{
		*iVal = (uint32)atoi(val.str);
	}
	return err;

OscFunctionEnd()
	
OscFunction( OscCfgGetInt32Range,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int32 *iVal,
		const int32 min,
		const int32 max,
		const int32 def)

	OSC_ERR err;
	err = OscCfgGetInt32(hFileContent, pKey, iVal);
	if ((max > min) && (err == SUCCESS))
	{
		/* check range */
		OscAssert_em(*iVal >= min, -ECFG_INVALID_VAL, "Value too small (%s: %d)!\n", pKey->strTag, *iVal);
		OscAssert_em((*iVal <= max) || (max==-1), -ECFG_INVALID_VAL, "Value too big (%s: %d)!\n", pKey->strTag, *iVal);
	}
	if( err != SUCCESS)
	{
		*iVal = def;
		err = ECFG_USED_DEFAULT;
	}	
	return err;

OscFunctionEnd()
	
OscFunction( OscCfgGetUInt32Range,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint32 *iVal,
		const uint32 min,
		const uint32 max,
		const uint32 def)

	OSC_ERR err;
	err = OscCfgGetUInt32(hFileContent, pKey, iVal);
	if ((max > min) && (err == SUCCESS))
	{
		/* check range */
		OscAssert_em(*iVal >= min, -ECFG_INVALID_VAL, "Value too small (%s: %d)!\n", pKey->strTag, *iVal);
		OscAssert_em((*iVal <= max) || (max==-1), -ECFG_INVALID_VAL, "Value too big (%s: %d)!\n", pKey->strTag, *iVal);
	}
	if( err != SUCCESS)
	{
		*iVal = def;
    	err = ECFG_USED_DEFAULT;		
	}
	return err;
	
OscFunctionEnd()
	
OscFunction( OscCfgGetFloatRange,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		float *iVal,
		const float min,
		const float max,
		const float def)

	struct CFG_VAL_STR val;
	float valF;
	int ret;
	OSC_ERR err;

	if( !(max!=max) && !(min!=min)) /* NAN test */
	{
		OscAssert_es( max > min, -ECFG_INVALID_VAL);
	}
	err = OscCfgGetStr( hFileContent, pKey, &val);
	if( err != SUCCESS)
	{
		*iVal = def;
		return ECFG_USED_DEFAULT;
	}
	ret = sscanf(val.str, "%f", &valF);
	if( ret == EOF)
	{
		*iVal = def;
		return ECFG_USED_DEFAULT;
	}
	if( ((min!=NAN) && (valF < min)) || ((max!=NAN) && (valF > max)) )
	{
		OscLog(WARN, "%s: Value out of range (%s: %f)!\n",
				__func__, pKey->strTag, valF);
		*iVal = def;
		return ECFG_USED_DEFAULT;
	}

	*iVal = valF;
OscFunctionEnd()

OscFunction( OscCfgGetBool,
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		bool *iVal,
		const bool def)

	OSC_ERR err;
	struct CFG_VAL_STR val;
	
	err = OscCfgGetStr( hFileContent, pKey, &val);
	if( err != SUCCESS)
	{
		strcpy( val.str, "0"); // if default
	}
	
	if( (0 == strcasecmp( val.str, "TRUE"))  || 
		(0 == strcmp( val.str, "1")) )
	{	
		*iVal = TRUE;
	}
	else
	{
		if( (0 == strcasecmp( val.str, "FALSE")) || 
			(0 == strcmp( val.str, "0")) )
		{	
			*iVal = FALSE;
		}		
		else
		{
			OscFail_e(-ECFG_INVALID_VAL);
		}
	}  
	if( err != SUCCESS)
	{
		*iVal = def;
    	err = ECFG_USED_DEFAULT;		
	}
	return err;		
	
OscFunctionEnd()
	
#ifdef TARGET_TYPE_MESA_SR4K
#error OscCfgGetSystemInfo needs to be updated to work on the Mesa SwissRanger.
#endif

OscFunction(OscCfgGetSystemInfo, struct OscSystemInfo ** ppInfo)
	OscFunction(staticStore, char * str, char ** staticStr)
		static char buffer[1024];
		static char * pNext = buffer;
		static int remaining = sizeof buffer;
		int len = strlen(str) + 1;
		
		OscAssert_m(len < remaining, "No buffer space left.");
		
		strncpy(pNext, str, remaining);
		
		*staticStr = pNext;
		pNext += len;
		remaining -= len;
	OscFunctionEnd()
	
#if defined(TARGET_TYPE_LEANXCAM) || (TARGET_TYPE_INDXCAM)
	OscFunction(hasBayernPattern, struct OscSystemInfo * pInfo, bool * res)
		if (pInfo->hardware.board.boardType == OscSystemInfo_boardType_leanXcam) {
			if (strcmp(pInfo->hardware.board.assembly, "A") == 0 || strcmp(pInfo->hardware.board.assembly, "B") == 0) {
				*res = true;
			} else if (strcmp(pInfo->hardware.board.assembly, "C") == 0) {
				*res = false;
			} else {
				OscFail();
			}
		} else if (pInfo->hardware.board.boardType == OscSystemInfo_boardType_indXcam) {
			*res = false;
		} else {
			OscFail();
		}
	OscFunctionEnd()
#endif /* TARGET_TYPE_LEANXCAM || TARGET_TYPE_INDXCAM*/
	
	static struct OscSystemInfo info = { };
	static bool inited = false;
	
#if defined(OSC_HOST)
	memset(&info, 0, sizeof(struct OscSystemInfo));

	info.hardware.board.boardType = OscSystemInfo_boardType_host;

	info.software.Oscar.major = OSC_VERSION_MAJOR;
  info.software.Oscar.minor = OSC_VERSION_MINOR;
  info.software.Oscar.patch = OSC_VERSION_PATCH;
  info.software.Oscar.rc = OSC_VERSION_RC;
  char * version;
  OscCall(OscGetVersionString, &version);
  OscCall(staticStore, version, &info.software.Oscar.version);

  inited = true;
  *ppInfo = &info;
	return SUCCESS;
#endif

	if (!inited) {
		char * envVar;
		char envVar2[80];
		
		OscCall(getUBootEnv, "hwrev", &envVar)
		if (OscLastStatus() == ECFG_UBOOT_ENV_NOT_FOUND) {
			OscCall(getUBootEnv, "HWREV", &envVar); // Fallback to the ALL_CAPS_VARIANT.
			if (OscLastStatus() == ECFG_UBOOT_ENV_NOT_FOUND) {
#ifdef TARGET_TYPE_LEANXCAM
				envVar = "LX_1.1_B";
#endif
#ifdef TARGET_TYPE_INDXCAM
				envVar = "IX_1.1_A";
#endif
#ifdef TARGET_TYPE_LEANXRADIO
				envVar = "LEANXRADIO_1.0_A";
#endif
			}
		}
		
		// FIXME: why do I have to copy the string out of getUBootEnv's static buffer?
		strcpy(envVar2, envVar);
		envVar = envVar2;
		
		OscCall(staticStore, envVar, &info.hardware.board.revision);
		
		{
			char * part2, * part3, * part4;
			
			part2 = strchr(envVar, '_');
			OscAssert_m(part2 != NULL, "Invalid format for hwrev: %s", info.hardware.board.revision);
			*part2 = '\0';
			part2 += 1;
			
			part3 = strchr(part2, '.');
			OscAssert_m(part3 != NULL, "Invalid format for hwrev: %s", info.hardware.board.revision);
			*part3 = 0;
			part3 += 1;
			
			part4 = strchr(part3, '_');
			OscAssert_m(part4 != NULL, "Invalid format for hwrev: %s", info.hardware.board.revision);
			*part4 = 0;
			part4 += 1;
			
			if (strcmp(envVar, "LX") == 0)
				info.hardware.board.boardType = OscSystemInfo_boardType_leanXcam;
			else if (strcmp(envVar, "IX") == 0)
				info.hardware.board.boardType = OscSystemInfo_boardType_indXcam;
			else if (strcmp(envVar, "LEANXRADIO") == 0)
				info.hardware.board.boardType = OscSystemInfo_boardType_leanXradio;
			else
				OscAssert_m(part2 != NULL, "Invalid format for hwrev: %s", info.hardware.board.revision);
			
			OscCall(parseInteger, part2, &info.hardware.board.major);
			OscCall(parseInteger, part3, &info.hardware.board.minor);
			
			OscCall(staticStore, part4, &info.hardware.board.assembly);
		}

#if defined(TARGET_TYPE_LEANXCAM) || (TARGET_TYPE_INDXCAM)
		info.hardware.imageSensor.imageWidth = OSC_CAM_MAX_IMAGE_WIDTH;
		info.hardware.imageSensor.imageWidth = OSC_CAM_MAX_IMAGE_HEIGHT;
		OscCall(hasBayernPattern, &info, &info.hardware.imageSensor.hasBayernPattern);
#endif /* TARGET_TYPE_LEANXCAM || TARGET_TYPE_INDXCAM*/
		
		{
			char * version;
			
			info.software.Oscar.major = OSC_VERSION_MAJOR;
			info.software.Oscar.minor = OSC_VERSION_MINOR;
			info.software.Oscar.patch = OSC_VERSION_PATCH;
			info.software.Oscar.rc = OSC_VERSION_RC;
			
			OscCall(OscGetVersionString, &version);
			OscCall(staticStore, version, &info.software.Oscar.version);
			
			OscCall(getUClinuxVersion,
					&version,
					&info.software.uClinux.major,
					&info.software.uClinux.minor,
					&info.software.uClinux.patch,
					&info.software.uClinux.rc);
			OscCall(staticStore, version, &info.software.uClinux.version);
		}
		
		inited = true;
	}
	
	*ppInfo = &info;
OscFunctionEnd()

/*======================= Private methods ==============================*/

OscFunction( static OscCfgGetValPtr,
		const unsigned int  contentIndex,
		const struct CFG_KEY *pKey,
		char **pPStrVal)

	char *pStrSecStart;  /* points to beginning of section content */
	char *pStrNextSecStart;
	
	/* check preconditions */
	OscAssert_em(pPStrVal && pKey && pKey->strTag,-ECFG_INVALID_FUNC_PARAMETER, "Invalid parameter.(%d, 0x%x)\n", contentIndex, pKey->strTag);
	
	/* find section */
	pStrSecStart = OscCfgFindNewlineLabel(pKey->strSection, CONFIG_FILE_SECTION_SUFFIX, cfg.contents[contentIndex].data);
	if(pStrSecStart == NULL)
	{
		*pPStrVal = NULL;
		return SUCCESS;
	}

	/* find tag */
	*pPStrVal = OscCfgFindNewlineLabel(pKey->strTag, CONFIG_FILE_TAG_SUFFIX, pStrSecStart);
	
	pStrNextSecStart = OscCfgFindNextSectionStart(pStrSecStart);
	OscAssert_e(pStrNextSecStart, -ECFG_ERROR);
	if (*pPStrVal > pStrNextSecStart) {
		/* tag found is in next section */
		*pPStrVal = NULL;
	}
	
OscFunctionEnd()
	
static char* OscCfgIsSubStr(
		const char *subString,
		const size_t subStringLen,
		const char *string)
{
	unsigned int i = 0;
	/* check preconditions */
	if (subString==NULL || string==NULL)
	{
		OscLog(ERROR, "%s(0x%x, %d, 0x%x): Invalid parameter.\n",
				__func__, subString, subStringLen, string);
		return NULL;
	}
	for (i=0; i<subStringLen; i++)
	{
		if ((subString[i] != string[i]) ||
			(subString[i] == 0) ||
			(string[i] == 0))
		{
			return NULL;
		}
	}
	return (char*)&string[i];
}

static char* OscCfgFindNewlineLabel(
		const char* label,
		const char* labelSuffix,
		char* text)
{
	const char newLine = '\n';
	char* textStr, *tmpStr;
	unsigned int offset;
	size_t labelLen, labelSuffixLen;

	/* check preconditions */
	if (text == NULL || labelSuffix == NULL)
	{
		OscLog(ERROR, "%s(0x%x, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, label, labelSuffix, text);
		return NULL;
	}
	/* no label is always found at the beginning of the text */
	if (label == NULL)
	{
		return text;
	}

	labelLen = strlen(label);
	labelSuffixLen = strlen(labelSuffix);
	offset = 0;
	for (textStr = text; textStr!=NULL; textStr = strchr(textStr, newLine))
	{
		/* find label */
		textStr = &textStr[offset];
		offset = 1;
		{ /* find white spaces to ignore */
			char ignoreStr[80]; /* copy white spaces to this ignored string */
			int result = sscanf(textStr, "%80[ \t]", ignoreStr);
			if (result == 1) {
				textStr = &textStr[strlen(ignoreStr)]; /* move pointer forward */
			}
		}
		tmpStr = OscCfgIsSubStr(label, labelLen, textStr);
		if (tmpStr != NULL)
		{
			/* find label suffix */
			tmpStr = OscCfgIsSubStr(labelSuffix, labelSuffixLen, tmpStr);
			if (tmpStr != NULL)
			{
				return tmpStr;
			}
		}
	}

	return NULL;
}

static char* OscCfgFindNextSectionStart(char* pSectionStart)
{
	char tmpText[80]="";
	const char newLine = '\n';
	unsigned int offset = 0;
	char *pSection;
	int result;

	/* check preconditions */
	pSection = pSectionStart;
	if (pSection == NULL) {
		OscLog(ERROR, "%s(0x%x): Invalid parameter.\n",
				__func__, pSection);
		return NULL;
	}

	while (pSection!=NULL) {
		if (strlen(pSection) > 0) {
			pSection = &pSection[offset];
			offset = 1;
		} else {
			break; /* EOF */
		}

		{/*ignore white spaces*/
			char ignoreStr[80]; /* copy white spaces to this ignored string */
			result = sscanf(pSection, "%80[ \t]", ignoreStr);
			if (result == 1) {
				pSection = &pSection[strlen(ignoreStr)]; /* move pointer forward */
			}
		}
		result = sscanf(pSection, "%80[^\n]", tmpText);
		if (result ==1) {
			/*text found*/
			if (tmpText[0] != '#' && tmpText[0] != '%' && tmpText[0] != '/') {
				/* line is no comment */
				if (strchr(tmpText, ':') == NULL) {
					/* line is no tag*/
					OscLog(DEBUG, " -- EOS (tmp = %s)\n", tmpText);
					return pSection; /* in this case it must be a section */
				}
			}
		}
		/* goto next line */
		pSection = strchr(pSection, newLine);
	}
	OscLog(DEBUG, " -- EOF\n");
	return &pSectionStart[strlen(pSectionStart)]; /* return EOF */
}


OscFunction( static OscCfgReplaceStr,
		const unsigned int  contentIndex,
		const char *oldStr,
		const char *newStr,
		char* text)

	size_t newStrLen, oldStrLen, textLen, diffLen;
	int16 i;

	/* check preconditions */
	if (newStr == NULL || oldStr == NULL || text == NULL)
	{
		OscLog(ERROR, "%s(%d, 0x%x, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, oldStr, newStr, text);
		return -ECFG_ERROR;
	}
	newStrLen = strlen(newStr);
	oldStrLen = strlen(oldStr);
	textLen = strlen(text);

	/* make space for newStr in text */
	if (newStrLen > oldStrLen)
	{
		/* shift text right, start from end  */
		diffLen = newStrLen - oldStrLen;
		/* check maximum file size */
		if (diffLen + strlen(cfg.contents[contentIndex].data) > cfg.contents[contentIndex].dataSize)
		{
			OscLog(ERROR, "%s: file length exceeded!\n",
					__func__);
			return -ECFG_ERROR;
		}
		for (i=(int16)textLen; i >= (int16)oldStrLen; i--)
		{
			text[i+diffLen] = text[i];
		}
	}
	else
	{
		/* shift text left, start from beginning */
		diffLen = oldStrLen - newStrLen;
		for (i=(unsigned int)newStrLen; i<textLen + 1; i++)
		{
			text[i] = text[i+diffLen];
		}
	}
	/* insert newStr (faster than strcpy because sizes are known here) */
	for (i=0; i<newStrLen; i++)
	{
		text[i] = newStr[i];
	}
	return SUCCESS;
	
OscFunctionEnd()
	
static char* OscCfgInsertText(
		const unsigned int  contentIndex,
		char* insertPosition,
		const char* insertText)
{
	size_t insertTextLen;
	int16 i;
	
	/* check preconditions */
	if (insertPosition == NULL) {
		OscLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter.\n",
				__func__, insertPosition, insertText);
		return NULL;
	}
	/* do nothing if no insertText is handed over */
	if (insertText == NULL) {
		return insertPosition;
	}
	/* check file size */
	insertTextLen = strlen(insertText);
	if(strlen(cfg.contents[contentIndex].data) + insertTextLen > cfg.contents[contentIndex].dataSize) {
		OscLog(ERROR, "%s: cannot insert text '%s'; file length exceeded!\n",
				__func__, insertText);
		return NULL;
	}
	
	/* shift file data right, starting from end of file */
	for (i=(int16)strlen(insertPosition); i >= 0; i--)
	{
		insertPosition[i+insertTextLen] = insertPosition[i]; /* copy all including '\0' at string end */
	}
	/* insert text */
	for (i=0; i<insertTextLen; i++)
	{
		insertPosition[i] = insertText[i];
	}

	return &insertPosition[insertTextLen];
}

OscFunction(static getUBootEnv, char * key, char ** value)
#ifdef OSC_HOST
	*value = NULL;
	OscFail_es(ECFG_UBOOT_ENV_NOT_FOUND);
#endif
#ifdef OSC_TARGET
	static char buffer[80];
	
	{
		FILE * file;
		char * ferr;
		int err;
		
		// FIXME: key should be escaped or checked for invalid characters.
		err = snprintf(buffer, sizeof buffer, "fw_printenv '%s' 2> /dev/null", key);
		OscAssert_em(err < sizeof buffer, -ECFG_UBOOT_ENV_READ_ERROR, "No buffer space left.");
		
		file = popen(buffer, "r");
		OscAssert_em(file != NULL, -ECFG_UBOOT_ENV_READ_ERROR, "Error starting command: %s", strerror(errno));
		
		ferr = fgets(buffer, sizeof buffer, file);
		
		if (feof(file) != 0)
			OscFail_es(ECFG_UBOOT_ENV_NOT_FOUND);
			
		err = pclose(file);
		
		OscAssert_em(ferr != NULL || feof(file) != 0, -ECFG_UBOOT_ENV_READ_ERROR, "Error reading from command: %s", strerror(errno));
		OscAssert_em(err != -1, -ECFG_UBOOT_ENV_READ_ERROR, "Error closing command: %s", strerror(errno));
		
		if (err == 1) {
			*value = NULL;
			OscFail_es(ECFG_UBOOT_ENV_NOT_FOUND);
		} else if (err != 0) {
			OscFail_em(-ECFG_UBOOT_ENV_READ_ERROR, "Error in command: %d", err);
		}
	}
	
	{
		char * equals, * newline;
		
		newline = strchr(buffer, '\n');
		OscAssert_em(newline != NULL, -ECFG_UBOOT_ENV_READ_ERROR, "No newline found.");
		*newline = 0;
		
		equals = strchr(buffer, '=');
		OscAssert_em(equals != NULL, -ECFG_UBOOT_ENV_READ_ERROR, "No equals sign found.");
		*value = equals + 1;
	}
#endif
OscFunctionEnd()

OscFunction(static parseInteger, char * str, int * res)
	char * pEnd;
	int num = strtol(str, &pEnd, 10);
	
	OscAssert_m(str != pEnd, "Not a valid integer: %s", str);
	OscAssert_m(*pEnd == 0, "Garbage at end of integer: %s", str);
	
	*res = num;
OscFunctionEnd()

OscFunction(static getUClinuxVersion, char ** res, int* major, int* minor, int* patch_level, int* rc)
	FILE * file = NULL;
	
	int err;
	int ret;
	*major=0, *minor=0, *patch_level=0, *rc=0;
	char* next = NULL;
	char* occur = NULL;
	
	file=fopen("/proc/version", "r");
				
	OscAssert(file != NULL);
	
	static char version[200];
	ret = fread(version, sizeof(char), 200, file);
	OscAssert(ret > 0);
	
	occur=strstr(version, "Git_");
	occur+=4;

	ret = sscanf(occur, "v%d.%d-p%d-RC%d%*s", major, minor, patch_level, rc);
	if(ret == 4 && *major >= 0 && *minor >= 0 && *patch_level >= 0 && *rc >= 0)
	{
		// Version number of form v1.2-p1-RC2
		next = strstr(occur, "RC");
		next += 2 + (1 + *patch_level/10);
		goto cleanup_and_exit;
	}

	ret = sscanf(occur, "v%d.%d-RC%d%*s", major, minor, rc);
	if(ret == 3 && *major >= 0 && *minor >= 0 && *patch_level >= 0 && *rc >= 0)
	{
		// Version number of form v1.2-RC2
		next = strstr(occur, "RC");
		next += 2 + (1 + *patch_level/10);
		goto cleanup_and_exit;
	}

	ret = sscanf(occur, "v%d.%d-p%d%*s", major, minor, patch_level);
	if(ret == 3 && *major >= 0 && *minor >= 0 && *patch_level >= 0)
	{
		// Version number of form v1.2-p1
		next = strstr(occur, "-p");
		next += 2 + (1 + *patch_level/10);
		goto cleanup_and_exit;
	}

	ret = sscanf(occur, "v%d.%d%*s", major, minor);
	if(ret == 2 && *major >= 0 && *minor >= 0)
	{
		// Version umber of form v1.2
		next = strstr(occur, ".");
		next += 1 + (1 + *minor/10);
		goto cleanup_and_exit;
	}
	
	// Not able to parse => No valid version found.
	OscLog(ERROR, "No valid uCLinux version string found!\n");
	*res = "v0.0-p0";

cleanup_and_exit:
	OscAssert(next != NULL);
	*next=0;
	OscAssert(occur != NULL);
	*res=occur;

	err = fclose(file);
	OscAssert(err == 0);
	
OscFunctionCatch()
//	fclose(file); FIXME: File's not in scope anymore!
	*res = "v0.0-p0";
OscFunctionEnd()
