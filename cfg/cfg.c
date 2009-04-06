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

/*! @file cfg.c
 * @brief Configuration file module implementation for target and host
 * 
 */

#include "cfg_pub.h"
#include "cfg_priv.h"
#include "oscar_intern.h"

/*! @brief The module singelton instance. */
struct OSC_CFG cfg;

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY cfg_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscCfgCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->cfg.useCnt != 0)
	{
		pFw->cfg.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	/* Load the module dependencies of this module. */
	err = OscLoadDependencies(pFw,
			cfg_deps,
			sizeof(cfg_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load dependencies! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	memset(&cfg, 0, sizeof(struct OSC_CFG));
	
	/* Increment the use count */
	pFw->cfg.hHandle = (void*)&cfg;
	pFw->cfg.useCnt++;
	
	return SUCCESS;
}

void OscCfgDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	int i;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->cfg.useCnt--;
	if(pFw->cfg.useCnt > 0)
	{
		return;
	}
	
	/* free memory of file content buffers */
	for(i=0; i < cfg.nrOfContents; i++)
	{
		free(cfg.contents[i].data);
	}

	OscUnloadDependencies(pFw,
			cfg_deps,
			sizeof(cfg_deps)/sizeof(struct OSC_DEPENDENCY));
	
	memset(&cfg, 0, sizeof(struct OSC_CFG));
}

OSC_ERR OscCfgRegisterFile(
		CFG_FILE_CONTENT_HANDLE *pFileContentHandle,
		const char *strFileName,
		const unsigned int maxFileSize)
{
	FILE    *pCfgFile;
	size_t  fileSize;
	unsigned int    actIndex = cfg.nrOfContents;
	unsigned int    invalidCharIndex;
	
	/* check preconditions */
	if(pFileContentHandle == NULL || strFileName == NULL || strFileName[0] == '\0')
	{
		OscLog(ERROR, "%s(0x%x, %s): Invalid parameter.\n",
				__func__, pFileContentHandle, strFileName);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}
	if(cfg.nrOfContents >= CONFIG_FILE_MAX_NUM) {
		OscLog(ERROR, "%s: too many handles open (%d=%d) !\n",
				__func__, cfg.nrOfContents, CONFIG_FILE_MAX_NUM);
		return -ECFG_NO_HANDLES;
	}

	/* copy file name and open file */
	pCfgFile = fopen(strFileName, "r");
	if(pCfgFile == NULL)
	{
		OscLog(WARN, "%s: Unable to open config file %s!\n",
				__func__, strFileName);
		return -ECFG_UNABLE_TO_OPEN_FILE;
	}

	/* save data in content manager */
	cfg.contents[actIndex].data = malloc(maxFileSize + 1);
	if (cfg.contents[actIndex].data == NULL)
	{
		OscLog(ERROR, "%s: could not allocate memory!\n",
				__func__);
		return -ECFG_ERROR;
	}
	fileSize = fread(cfg.contents[actIndex].data, sizeof(char), maxFileSize + 1, pCfgFile);
	fclose(pCfgFile);
	if (fileSize == maxFileSize + 1)
	{
		OscLog(ERROR, "%s: config file too long!\n",
				__func__);
		free(cfg.contents[actIndex].data);
		return -ECFG_UNABLE_TO_OPEN_FILE;
	}
	cfg.nrOfContents++;
	
	/* append string termination */
	invalidCharIndex = OscCfgFindInvalidChar((unsigned char *)cfg.contents[actIndex].data, fileSize);
	cfg.contents[actIndex].data[invalidCharIndex] = '\0';
	OscLog(DEBUG, "%s: string length set to %d\n",
			__func__, invalidCharIndex);

	cfg.contents[actIndex].dataSize = maxFileSize + 1;
	*pFileContentHandle = actIndex+1; /* return content handle */
	strcpy(cfg.contents[actIndex].fileName, strFileName); /* store file name */
	
/*    OscLog(DEBUG, "Read config file (%s):\n%s\n", strFileName, cfg.contents[actIndex].data); does not work on OSC*/

	return SUCCESS;
}

OSC_ERR OscCfgDeleteAll( void)
{
	return SUCCESS;
}

OSC_ERR OscCfgFlushContent(const CFG_FILE_CONTENT_HANDLE hFileContent)
{
	return OscCfgFlushContentHelper(hFileContent, FALSE);
}

OSC_ERR OscCfgFlushContentAll(const CFG_FILE_CONTENT_HANDLE hFileContent)
{
	return OscCfgFlushContentHelper(hFileContent, TRUE);
}

OSC_ERR OscCfgGetStr(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		struct CFG_VAL_STR *pVal)
{
	char *pStrVal = NULL;
	int  stdErr;
	OSC_ERR err;
	
	/* check preconditions */
	if(pKey == NULL || pVal == NULL || hFileContent == 0 || hFileContent > CONFIG_FILE_MAX_NUM)
	{
		OscLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, hFileContent, pKey, pVal);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}

	/* find value pointer */
	err = OscCfgGetValPtr(hFileContent-1, pKey, &pStrVal);
	if (err != SUCCESS)
	{
		return err;
	}
	/* function may return null pointer */
	else if(pStrVal == NULL)
	{
		OscLog(WARN, "%s: tag or section not found (%s)!\n",
				__func__, pKey->strTag);
		return -ECFG_INVALID_KEY;
	}
	
	/* scan value at beginning of file */
	stdErr = sscanf(pStrVal, CONFIG_FILE_ESCAPE_CHARS, pVal->str);
	if (stdErr == EOF)
	{
		OscLog(WARN, "%s: no val found! (TAG=%s)\n",
				__func__, pKey->strTag);
		return -ECFG_INVALID_VAL;
	}

	OscLog(DEBUG, "Read Tag '%s': Value '%s'\n", pKey->strTag, pVal->str);
	return SUCCESS;
}


OSC_ERR OscCfgSetStr(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const char *strNewVal)
{
	char *pStrSecStart = NULL;  /* points to beginning of section content */
	char *pStrVal = NULL;
	struct CFG_VAL_STR oldVal;
	int  stdErr;
	OSC_ERR err;
	unsigned int index;
	
	/* check preconditions */
	if(pKey == NULL || hFileContent == 0 || hFileContent > CONFIG_FILE_MAX_NUM)
	{
		OscLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, hFileContent, pKey, strNewVal);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}
	index = hFileContent - 1;

	/* find value pointer */
	err = OscCfgGetValPtr(index, pKey, &pStrVal);
	if (err != SUCCESS)
	{
		return err;
	}
	if (pStrVal == NULL) /* if section or tag not found */
	{
		/* find section */
		pStrSecStart = OscCfgFindNewlineLabel(pKey->strSection, CONFIG_FILE_SECTION_SUFFIX, cfg.contents[index].data);
		if(pStrSecStart == NULL)
		{
			/* append section label and get pointer to content string termination */
			pStrSecStart = OscCfgAppendLabel(cfg.contents[index].data, cfg.contents[index].dataSize, pKey->strSection, CONFIG_FILE_LABEL_PREFIX, ""/* \n added with tag*/);
			if (pStrSecStart == NULL)
			{
				return -ECFG_ERROR;
			}
		}
		pStrVal = OscCfgFindNewlineLabel(pKey->strTag, CONFIG_FILE_TAG_SUFFIX, pStrSecStart);
		if(pStrVal == NULL)
		{
			/* append tag label and get pointer to content string termination */
			pStrVal = OscCfgAppendLabel(cfg.contents[index].data, cfg.contents[index].dataSize, pKey->strTag, CONFIG_FILE_LABEL_PREFIX, CONFIG_FILE_TAG_SUFFIX);
			if (pStrVal == NULL)
			{
				return -ECFG_ERROR;
			}
		}
	}
	/* scan value after tag */
	stdErr = sscanf(pStrVal, CONFIG_FILE_ESCAPE_CHARS, oldVal.str);
	if (stdErr == EOF)
	{
		oldVal.str[0] = '\0'; /* set string termination */
	}
	
	/* insert the new string into config file */
	err = OscCfgReplaceStr(index, oldVal.str, strNewVal, pStrVal);
	if (err == SUCCESS)
	{
		OscLog(DEBUG, "Wrote Tag '%s': Value '%s'\n", pKey->strTag, strNewVal);
	}
	else
	{
		OscLog(WARN, "Unable to write Tag '%s': Value '%s'\n", pKey->strTag, strNewVal);
	}
	return err;
}

OSC_ERR OscCfgGetInt(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal)
{
	OSC_ERR err;
	int32 tmpVal;
	err = OscCfgGetInt32(hFileContent, pKey, &tmpVal);
	if (err == SUCCESS)
	{
			*iVal = (int16)tmpVal;
	}
	return err;
}

OSC_ERR OscCfgGetUInt8(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint8 *iVal)
{
	OSC_ERR err;
	uint32 tmpVal;
	err = OscCfgGetUInt32(hFileContent, pKey, &tmpVal);
	if (err == SUCCESS)
	{
			*iVal = (uint8)tmpVal;
	}
	return err;
}


OSC_ERR OscCfgGetIntRange(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal,
		const int16 min,
		const int16 max)
{
	OSC_ERR err;
	int32 tmpVal;
	err = OscCfgGetInt32Range(hFileContent, pKey, &tmpVal, min, max);
	if (err == SUCCESS)
	{
			*iVal = (int16)tmpVal;
	}
	return err;
}

OSC_ERR OscCfgGetUInt16Range(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint16 *iVal,
		const uint16 min,
		const uint16 max)
{
	OSC_ERR err;
	uint32 tmpVal;
	err = OscCfgGetUInt32Range(hFileContent, pKey, &tmpVal, min, max);
	if (err == SUCCESS)
	{
			*iVal = (uint16)tmpVal;
	}
	return err;
}


OSC_ERR OscCfgGetInt32(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int32 *iVal)
{
	struct CFG_VAL_STR val;
	OSC_ERR err;
	/* check preconditions */
	if(pKey == NULL || iVal == NULL)
	{
		OscLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, hFileContent, pKey, iVal);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}

	err = OscCfgGetStr(hFileContent, pKey, &val);
	if (err == SUCCESS)
	{
		*iVal = (int32)atoi(val.str);
	}
	return err;
}

OSC_ERR OscCfgGetUInt32(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint32 *iVal)
{
	struct CFG_VAL_STR val;
	OSC_ERR err;
	/* check preconditions */
	if(pKey == NULL || iVal == NULL)
	{
		OscLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, hFileContent, pKey, iVal);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}

	err = OscCfgGetStr(hFileContent, pKey, &val);
	if (err == SUCCESS)
	{
		*iVal = (uint32)atoi(val.str);
	}
	return err;
}

OSC_ERR OscCfgGetInt32Range(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int32 *iVal,
		const int32 min,
		const int32 max)
{
	OSC_ERR err;
	err = OscCfgGetInt32(hFileContent, pKey, iVal);
	if ((max > min) && (err == SUCCESS))
	{
		if ((*iVal < min) || (*iVal > max))
		{
			OscLog(WARN, "%s: Value out of range (%s: %d)!\n",
					__func__, pKey->strTag, *iVal);
			return -ECFG_INVALID_VAL;
		}
	}
	return err;
}

OSC_ERR OscCfgGetUInt32Range(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint32 *iVal,
		const uint32 min,
		const uint32 max)
{
	OSC_ERR err;
	err = OscCfgGetUInt32(hFileContent, pKey, iVal);
	if ((max > min) && (err == SUCCESS))
	{
		if ((*iVal < min) || (*iVal > max))
		{
			OscLog(WARN, "%s: Value out of range (%s: %d)!\n",
					__func__, pKey->strTag, *iVal);
			return -ECFG_INVALID_VAL;
		}
	}
	return err;
}





/*======================= Private methods ==============================*/

OSC_ERR OscCfgFlushContentHelper(const CFG_FILE_CONTENT_HANDLE hFileContent, bool all)
{
	FILE        *pCfgFile;
	char        *strFileName;
	size_t      fileSize;
	unsigned int strSize;
	unsigned int index;

	/* check preconditions */
	if(hFileContent == 0 || hFileContent > CONFIG_FILE_MAX_NUM)
	{
		OscLog(ERROR, "%s(%d): Invalid parameter.\n",
				__func__, hFileContent);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}
	index = hFileContent-1;
	strSize = strlen(cfg.contents[index].data); /* string size without \0 */
	if (strSize > cfg.contents[index].dataSize - 1)
		{
		OscLog(ERROR, "%s: invalid content size!\n",
				__func__);
		return -ECFG_ERROR;
	}

	/* open file */
	strFileName = cfg.contents[index].fileName;
	pCfgFile = fopen(strFileName, "w+");
	if(pCfgFile == NULL)
	{
		OscLog(ERROR, "%s: Unable to open config file %s!\n",
				__func__, strFileName);
		return -ECFG_UNABLE_TO_OPEN_FILE;
	}
	
	fileSize = fwrite(cfg.contents[index].data, sizeof(char), strSize, pCfgFile);   /* write string */
	if (all)
	{
		memset(&cfg.contents[index].data[strSize], 0, cfg.contents[index].dataSize - strSize);
		fileSize += fwrite(&cfg.contents[index].data[strSize], sizeof(char), cfg.contents[index].dataSize - strSize - 1, pCfgFile);
	}
	fclose(pCfgFile);
	if (fileSize < strSize)
	{
		OscLog(ERROR, "%s: could not write data!\n",
				__func__);
		return -ECFG_UNABLE_TO_WRITE_FILE;
	}

/*    OscLog(DEBUG, "Wrote config file (%s):\n%s\n", strFileName, cfg.contents[index].data);*/

	return SUCCESS;
}

OSC_ERR OscCfgGetValPtr(
		const unsigned int  contentIndex,
		const struct CFG_KEY *pKey,
		char **pPStrVal)
{
	char            *pStrSecStart;  /* points to beginning of section content */
	
	/* check preconditions */
	if(pPStrVal == NULL ||
			pKey == NULL || pKey->strTag == NULL)
	{
		OscLog(ERROR, "%s(%d, 0x%x): Invalid parameter.\n",
				__func__, contentIndex, pKey->strTag);
		return -ECFG_INVALID_FUNC_PARAMETER;
	}
	
	/* find section */
	pStrSecStart = OscCfgFindNewlineLabel(pKey->strSection, CONFIG_FILE_SECTION_SUFFIX, cfg.contents[contentIndex].data);
	if(pStrSecStart == NULL)
	{
		*pPStrVal = NULL;
		return SUCCESS;
	}

	/* find tag */
	*pPStrVal = OscCfgFindNewlineLabel(pKey->strTag, CONFIG_FILE_TAG_SUFFIX, pStrSecStart);
	return SUCCESS;
}

char* OscCfgIsSubStr(
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

char* OscCfgFindNewlineLabel(
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

OSC_ERR OscCfgReplaceStr(
		const unsigned int  contentIndex,
		const char *oldStr,
		const char *newStr,
		char* text)
{
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
}

char* OscCfgAppendLabel(
		char* text,
		const unsigned int maxTextLen,
		const char* label,
		const char* labelPrefix,
		const char* labelSuffix)
{
	/* check preconditions */
	if (text == NULL || labelPrefix == NULL || labelSuffix == NULL)
	{
		OscLog(ERROR, "%s(0x%x, %d, 0x%x, 0x%x, 0x%x): Invalid parameter.\n",
				__func__, text, maxTextLen, label, labelPrefix, labelSuffix);
		return NULL;
	}
	
	/* do nothing if label == NULL */
	if (label == NULL)
	{
		return text;
	}

	/* check file size */
	if(strlen(text) + strlen(label) + strlen(labelSuffix) + 1 > maxTextLen)
	{
		OscLog(ERROR, "%s: cannot insert label '%s'; file length exceeded!\n",
				__func__, label);
		return NULL;
	}
	/* add label string to end of file */
	strcat(text, labelPrefix);
	strcat(text, label);
	strcat(text, labelSuffix);
	return &text[strlen(text)];
}

unsigned int OscCfgFindInvalidChar(const unsigned char *str, const unsigned int strSize)
{
	int i;
	for (i=0; i<strSize; i++)
	{
		if ((str[i] < (unsigned char)0x0a) || (str[i] > (unsigned char)0x7f))
		{
			return i;
		}
	}
	return strSize;
}
