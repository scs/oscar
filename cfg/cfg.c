/*! @file cfg.c
 * @brief Configuration file module implementation for target and host
 * 
 * @author Men Muheim
 */

#include "cfg_pub.h"
#include "cfg_priv.h"
#include "framework_intern.h"

/*! @brief The module singelton instance. */
struct LCV_CFG cfg;     

/*! @brief The dependencies of this module. */
struct LCV_DEPENDENCY cfg_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy}
};

LCV_ERR LCVCfgCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;
    
    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->cfg.useCnt != 0)
    {
        pFw->cfg.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module dependencies of this module. */
    err = LCVLoadDependencies(pFw, 
            cfg_deps, 
            sizeof(cfg_deps)/sizeof(struct LCV_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load dependencies! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&cfg, 0, sizeof(struct LCV_CFG));
    
    /* Increment the use count */
    pFw->cfg.hHandle = (void*)&cfg;
    pFw->cfg.useCnt++;    
    
    return SUCCESS;
}

void LCVCfgDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    int i;
    
    pFw = (struct LCV_FRAMEWORK *)hFw;
    
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

    LCVUnloadDependencies(pFw, 
            cfg_deps, 
            sizeof(cfg_deps)/sizeof(struct LCV_DEPENDENCY));
    
    memset(&cfg, 0, sizeof(struct LCV_CFG));
}

LCV_ERR LCVCfgRegisterFile(
		CFG_FILE_CONTENT_HANDLE *pFileContentHandle, 
		const char *strFileName,
		const unsigned int maxFileSize)
{
    FILE    *pCfgFile;
    size_t	fileSize;
    unsigned int 	actIndex = cfg.nrOfContents;
    unsigned int    invalidCharIndex;
    
    /* check preconditions */
    if(pFileContentHandle == NULL || strFileName == NULL || strFileName[0] == '\0')
    {
        LCVLog(ERROR, "%s(0x%x, %s): Invalid parameter.\n", 
                __func__, pFileContentHandle, strFileName);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }
    if(cfg.nrOfContents >= CONFIG_FILE_MAX_NUM) {
        LCVLog(ERROR, "%s: too many handles open (%d=%d) !\n",
                __func__, cfg.nrOfContents, CONFIG_FILE_MAX_NUM);
        return -ECFG_NO_HANDLES;
    }

    /* copy file name and open file */
    pCfgFile = fopen(strFileName, "r");
    if(pCfgFile == NULL)
    {
        LCVLog(WARN, "%s: Unable to open config file %s!\n",
                __func__, strFileName);
        return -ECFG_UNABLE_TO_OPEN_FILE;
    }

    /* save data in content manager */
    cfg.contents[actIndex].data = malloc(maxFileSize + 1);
    if (cfg.contents[actIndex].data == NULL)
    {
        LCVLog(ERROR, "%s: could not allocate memory!\n",
                __func__);
        return -ECFG_ERROR;
    }
    fileSize = fread(cfg.contents[actIndex].data, sizeof(char), maxFileSize + 1, pCfgFile);
    fclose(pCfgFile);
    if (fileSize == maxFileSize + 1)
    {
        LCVLog(ERROR, "%s: config file too long!\n",
                __func__);
        free(cfg.contents[actIndex].data);
        return -ECFG_UNABLE_TO_OPEN_FILE;
    }
    cfg.nrOfContents++; 
    
    /* append string termination */
    invalidCharIndex = LCVCfgFindInvalidChar(cfg.contents[actIndex].data, fileSize);
    cfg.contents[actIndex].data[invalidCharIndex] = '\0';
    LCVLog(DEBUG, "%s: string length set to %d\n",
            __func__, invalidCharIndex);

    cfg.contents[actIndex].dataSize = maxFileSize + 1;
    *pFileContentHandle = actIndex+1; /* return content handle */
    strcpy(cfg.contents[actIndex].fileName, strFileName); /* store file name */
    
/*    LCVLog(DEBUG, "Read config file (%s):\n%s\n", strFileName, cfg.contents[actIndex].data); does not work on LCV*/

    return SUCCESS;
}

LCV_ERR LCVCfgDeleteAll( void)
{
    return SUCCESS;
}

LCV_ERR LCVCfgFlushContent(const CFG_FILE_CONTENT_HANDLE hFileContent)
{
    return LCVCfgFlushContentHelper(hFileContent, FALSE);
}

LCV_ERR LCVCfgFlushContentAll(const CFG_FILE_CONTENT_HANDLE hFileContent)
{
    return LCVCfgFlushContentHelper(hFileContent, TRUE);
}

LCV_ERR LCVCfgGetStr(
		const CFG_FILE_CONTENT_HANDLE hFileContent, 
		const struct CFG_KEY *pKey,
		struct CFG_VAL_STR *pVal)
{
	char *pStrVal = NULL;
	int  stdErr;
	LCV_ERR err;
	
    /* check preconditions */
    if(pKey == NULL || pVal == NULL || hFileContent == 0 || hFileContent > CONFIG_FILE_MAX_NUM)
    {
        LCVLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n", 
                __func__, hFileContent, pKey, pVal);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }

    /* find value pointer */
	err = LCVCfgGetValPtr(hFileContent-1, pKey, &pStrVal);
	if (err != SUCCESS)
	{
		return err;
	}
	/* function may return null pointer */
	else if(pStrVal == NULL)
    {
        LCVLog(WARN, "%s: tag or section not found (%s)!\n",
                __func__, pKey->strTag);
        return -ECFG_INVALID_KEY;
	}
	
    /* scan value at beginning of file */
	stdErr = sscanf(pStrVal, CONFIG_FILE_ESCAPE_CHARS, pVal->str);
    if (stdErr == EOF)
    {
        LCVLog(WARN, "%s: no val found! (TAG=%s)\n",
                __func__, pKey->strTag);
        return -ECFG_INVALID_VAL;
    }

    LCVLog(DEBUG, "Read Tag '%s': Value '%s'\n", pKey->strTag, pVal->str);
    return SUCCESS;
}


LCV_ERR LCVCfgSetStr(
		const CFG_FILE_CONTENT_HANDLE hFileContent, 
		const struct CFG_KEY *pKey,
		const char *strNewVal)
{
	char *pStrSecStart = NULL;	/* points to beginning of section content */
	char *pStrVal = NULL;
	struct CFG_VAL_STR oldVal;
	int  stdErr;
	LCV_ERR err;
    unsigned int index;
	
    /* check preconditions */
    if(pKey == NULL || hFileContent == 0 || hFileContent > CONFIG_FILE_MAX_NUM)
    {
        LCVLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n", 
                __func__, hFileContent, pKey, strNewVal);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }
    index = hFileContent - 1;

    /* find value pointer */
	err = LCVCfgGetValPtr(index, pKey, &pStrVal);
	if (err != SUCCESS)
	{
		return err;
	}
	if (pStrVal == NULL) /* if section or tag not found */
	{
	    /* find section */
	    pStrSecStart = LCVCfgFindNewlineLabel(pKey->strSection, CONFIG_FILE_SECTION_SUFFIX, cfg.contents[index].data);
	    if(pStrSecStart == NULL)
	    {
	    	/* append section label and get pointer to content string termination */
	    	pStrSecStart = LCVCfgAppendLabel(cfg.contents[index].data, cfg.contents[index].dataSize, pKey->strSection, CONFIG_FILE_LABEL_PREFIX, ""/* \n added with tag*/); 
	    	if (pStrSecStart == NULL)
	    	{
	    	    return -ECFG_ERROR;
	    	}
	    }
    	pStrVal = LCVCfgFindNewlineLabel(pKey->strTag, CONFIG_FILE_TAG_SUFFIX, pStrSecStart);
	    if(pStrVal == NULL)
	    {
	    	/* append tag label and get pointer to content string termination */
	    	pStrVal = LCVCfgAppendLabel(cfg.contents[index].data, cfg.contents[index].dataSize, pKey->strTag, CONFIG_FILE_LABEL_PREFIX, CONFIG_FILE_TAG_SUFFIX); 
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
    err = LCVCfgReplaceStr(index, oldVal.str, strNewVal, pStrVal);
    if (err == SUCCESS)
    {
	    LCVLog(DEBUG, "Wrote Tag '%s': Value '%s'\n", pKey->strTag, strNewVal);
    }
    else
    {
        LCVLog(WARN, "Unable to write Tag '%s': Value '%s'\n", pKey->strTag, strNewVal);
    }
	return err;
}

LCV_ERR LCVCfgGetInt(
		const CFG_FILE_CONTENT_HANDLE hFileContent, 
		const struct CFG_KEY *pKey,
		int16 *iVal)
{
    LCV_ERR err;
    int32 tmpVal;
    err = LCVCfgGetInt32(hFileContent, pKey, &tmpVal);
    if (err == SUCCESS)
    {
            *iVal = (int16)tmpVal;
    }
    return err;
}

LCV_ERR LCVCfgGetUInt8(
        const CFG_FILE_CONTENT_HANDLE hFileContent, 
        const struct CFG_KEY *pKey,
        uint8 *iVal)
{
    LCV_ERR err;
    uint32 tmpVal;
    err = LCVCfgGetUInt32(hFileContent, pKey, &tmpVal);
    if (err == SUCCESS)
    {
            *iVal = (uint8)tmpVal;
    }
    return err;
}


LCV_ERR LCVCfgGetIntRange(
		const CFG_FILE_CONTENT_HANDLE hFileContent, 
		const struct CFG_KEY *pKey,
		int16 *iVal, 
		const int16 min, 
		const int16 max)
{
    LCV_ERR err;
    int32 tmpVal;
    err = LCVCfgGetInt32Range(hFileContent, pKey, &tmpVal, min, max);
    if (err == SUCCESS)
    {
            *iVal = (int16)tmpVal;
    }
    return err;
}

LCV_ERR LCVCfgGetUInt16Range(
        const CFG_FILE_CONTENT_HANDLE hFileContent, 
        const struct CFG_KEY *pKey,
        uint16 *iVal, 
        const uint16 min, 
        const uint16 max)
{
    LCV_ERR err;
    uint32 tmpVal;
    err = LCVCfgGetUInt32Range(hFileContent, pKey, &tmpVal, min, max);
    if (err == SUCCESS)
    {
            *iVal = (uint16)tmpVal;
    }
    return err;
}


LCV_ERR LCVCfgGetInt32(
        const CFG_FILE_CONTENT_HANDLE hFileContent, 
        const struct CFG_KEY *pKey,
        int32 *iVal)
{
    struct CFG_VAL_STR val;
    LCV_ERR err;
    /* check preconditions */
    if(pKey == NULL || iVal == NULL)
    {
        LCVLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n", 
                __func__, hFileContent, pKey, iVal);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }

    err = LCVCfgGetStr(hFileContent, pKey, &val);
    if (err == SUCCESS)
    {
        *iVal = (int32)atoi(val.str);
    }
    return err;
}

LCV_ERR LCVCfgGetUInt32(
        const CFG_FILE_CONTENT_HANDLE hFileContent, 
        const struct CFG_KEY *pKey,
        uint32 *iVal)
{
    struct CFG_VAL_STR val;
    LCV_ERR err;
    /* check preconditions */
    if(pKey == NULL || iVal == NULL)
    {
        LCVLog(ERROR, "%s(%d, 0x%x, 0x%x): Invalid parameter.\n", 
                __func__, hFileContent, pKey, iVal);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }

    err = LCVCfgGetStr(hFileContent, pKey, &val);
    if (err == SUCCESS)
    {
        *iVal = (uint32)atoi(val.str);
    }
    return err;
}

LCV_ERR LCVCfgGetInt32Range(
        const CFG_FILE_CONTENT_HANDLE hFileContent, 
        const struct CFG_KEY *pKey,
        int32 *iVal, 
        const int32 min, 
        const int32 max)
{
    LCV_ERR err;
    err = LCVCfgGetInt32(hFileContent, pKey, iVal);
    if ((max > min) && (err == SUCCESS)) 
    {
        if ((*iVal < min) || (*iVal > max))
        {
            LCVLog(WARN, "%s: Value out of range (%s: %d)!\n",
                    __func__, pKey->strTag, *iVal);
            return -ECFG_INVALID_VAL;
        }
    }
    return err;
}

LCV_ERR LCVCfgGetUInt32Range(
        const CFG_FILE_CONTENT_HANDLE hFileContent, 
        const struct CFG_KEY *pKey,
        uint32 *iVal, 
        const uint32 min, 
        const uint32 max)
{
    LCV_ERR err;
    err = LCVCfgGetUInt32(hFileContent, pKey, iVal);
    if ((max > min) && (err == SUCCESS)) 
    {
        if ((*iVal < min) || (*iVal > max))
        {
            LCVLog(WARN, "%s: Value out of range (%s: %d)!\n",
                    __func__, pKey->strTag, *iVal);
            return -ECFG_INVALID_VAL;
        }
    }
    return err;
}





/*======================= Private methods ==============================*/

LCV_ERR LCVCfgFlushContentHelper(const CFG_FILE_CONTENT_HANDLE hFileContent, bool all)
{
    FILE        *pCfgFile;
    char        *strFileName; 
    size_t      fileSize;
    unsigned int strSize;
    unsigned int index;

    /* check preconditions */
    if(hFileContent == 0 || hFileContent > CONFIG_FILE_MAX_NUM)
    {
        LCVLog(ERROR, "%s(%d): Invalid parameter.\n", 
                __func__, hFileContent);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }
    index = hFileContent-1;
    strSize = strlen(cfg.contents[index].data); /* string size without \0 */
    if (strSize > cfg.contents[index].dataSize - 1) 
     {
        LCVLog(ERROR, "%s: invalid content size!\n",
                __func__);
        return -ECFG_ERROR;
    }

    /* open file */
    strFileName = cfg.contents[index].fileName;
    pCfgFile = fopen(strFileName, "w+");
    if(pCfgFile == NULL)
    {
        LCVLog(ERROR, "%s: Unable to open config file %s!\n",
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
        LCVLog(ERROR, "%s: could not write data!\n",
                __func__);
        return -ECFG_UNABLE_TO_WRITE_FILE;
    }

/*    LCVLog(DEBUG, "Wrote config file (%s):\n%s\n", strFileName, cfg.contents[index].data);*/

    return SUCCESS;
}

LCV_ERR LCVCfgGetValPtr(
		const unsigned int 	contentIndex, 
		const struct CFG_KEY *pKey,
		char **pPStrVal)
{
	char			*pStrSecStart;	/* points to beginning of section content */
	
    /* check preconditions */
    if(pPStrVal == NULL ||
    		pKey == NULL || pKey->strTag == NULL)
    {
        LCVLog(ERROR, "%s(%d, 0x%x): Invalid parameter.\n", 
                __func__, contentIndex, pKey->strTag);
        return -ECFG_INVALID_FUNC_PARAMETER;
    }
    
    /* find section */
    pStrSecStart = LCVCfgFindNewlineLabel(pKey->strSection, CONFIG_FILE_SECTION_SUFFIX, cfg.contents[contentIndex].data);
    if(pStrSecStart == NULL)
    {
    	*pPStrVal = NULL;
        return SUCCESS;
	}

    /* find tag */
    *pPStrVal = LCVCfgFindNewlineLabel(pKey->strTag, CONFIG_FILE_TAG_SUFFIX, pStrSecStart);
    return SUCCESS;
}

char* LCVCfgIsSubStr(
		const char *subString, 
		const size_t subStringLen, 
		const char *string)
{
	unsigned int i = 0;
	/* check preconditions */
	if (subString==NULL || string==NULL)
	{
        LCVLog(ERROR, "%s(0x%x, %d, 0x%x): Invalid parameter.\n", 
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

char* LCVCfgFindNewlineLabel(
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
        LCVLog(ERROR, "%s(0x%x, 0x%x, 0x%x): Invalid parameter.\n", 
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
		tmpStr = LCVCfgIsSubStr(label, labelLen, textStr);
		if (tmpStr != NULL)
		{
			/* find label suffix */
			tmpStr = LCVCfgIsSubStr(labelSuffix, labelSuffixLen, tmpStr);
			if (tmpStr != NULL)
			{
				return tmpStr;
			}
		}
	}

	return NULL;
}

LCV_ERR LCVCfgReplaceStr(
		const unsigned int 	contentIndex, 
		const char *oldStr, 
		const char *newStr, 
		char* text)
{
	size_t newStrLen, oldStrLen, textLen, diffLen;
	int16 i;

	/* check preconditions */
	if (newStr == NULL || oldStr == NULL || text == NULL)
	{
        LCVLog(ERROR, "%s(%d, 0x%x, 0x%x, 0x%x): Invalid parameter.\n", 
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
	        LCVLog(ERROR, "%s: file length exceeded!\n",
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

char* LCVCfgAppendLabel(
		char* text, 
		const unsigned int maxTextLen, 
		const char* label, 
		const char* labelPrefix,
		const char* labelSuffix)
{
	/* check preconditions */
	if (text == NULL || labelPrefix == NULL || labelSuffix == NULL)
	{
        LCVLog(ERROR, "%s(0x%x, %d, 0x%x, 0x%x, 0x%x): Invalid parameter.\n", 
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
	    LCVLog(ERROR, "%s: cannot insert label '%s'; file length exceeded!\n",
	            __func__, label);
	    return NULL;
	}
	/* add label string to end of file */
	strcat(text, labelPrefix);
	strcat(text, label);
	strcat(text, labelSuffix);
	return &text[strlen(text)];
}

unsigned int LCVCfgFindInvalidChar(const char *str, const unsigned int strSize)
{
    int i;
    for (i=0; i<strSize; i++)
    {
        if ((str[i] < (char)0x0a) || (str[i] > (char)0x7f))
        {
            return i;
        }
    }
    return strSize;
}
