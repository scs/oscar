/*! @file frd.c
 * @brief Filename reader module implementation.
 * 
 * @author Markus Berner
 ************************************************************************/
#ifdef LCV_HOST
#include "framework_types_host.h"
#endif

#ifdef LCV_TARGET
#include "framework_types_target.h"
#endif

#include "frd_pub.h"
#include "frd_priv.h"
#include "framework_intern.h"
#include <errno.h>

#include <sim/sim_pub.h>

struct LCV_FRD frd; /*!< Module singelton instance */

/*! The dependencies of this module. */
struct LCV_DEPENDENCY frd_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy},
        {"sim", LCVSimCreate, LCVSimDestroy}
};

LCV_ERR LCVFrdCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->frd.useCnt != 0)
    {
        pFw->frd.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }

    /* Load the module frd_deps of this module. */
    err = LCVLoadDependencies(pFw, 
            frd_deps, 
            sizeof(frd_deps)/sizeof(struct LCV_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load frd_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&frd, 0, sizeof(struct LCV_FRD)); 

    /* Register a callback to be invoked every simulation cycle
     * to be able to fetch the next file name from the list. */
    err = LCVSimRegisterCycleCallback(LCVFrdSimCycleCallback);
    if(err != SUCCESS)
    {
    	LCVLog(ERROR, "%s: Unable to register cycle callback (%d)\n",
    			__func__, err);
    	return err;
    }
    
    /* Increment the use count */
    pFw->frd.hHandle = (void*)&frd;
    pFw->frd.useCnt++;  
    
    return SUCCESS;
}

void LCVFrdDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    struct LCV_FRD_READER *pReader;
    uint32 rd;
        
    pFw = (struct LCV_FRAMEWORK *)hFw; 
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
			LCVLog(ERROR, 
					"%s: Unsupported reader type configured (%d)!\n",
					__func__, pReader->enType);
		}
    }
    LCVUnloadDependencies(pFw, 
            frd_deps, 
            sizeof(frd_deps)/sizeof(struct LCV_DEPENDENCY));
    
    
    memset(&frd, 0, sizeof(struct LCV_FRD));
}

LCV_ERR LCVFrdCreateReader(void ** phReaderHandle, 
        const char strReaderConfigFile[])
{
    FILE                        *pConfigF;
    struct LCV_FRD_READER       	*pReader;
    int                         assigned;
    LCV_ERR                     err;
    char						strTemp[1024];
    
    if(unlikely((strReaderConfigFile == NULL) || 
            (strReaderConfigFile[0] == '\0') ||
            (phReaderHandle == NULL)))
    {
        LCVLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
        		__func__, phReaderHandle, strReaderConfigFile);
        return -EINVALID_PARAMETER;
    }
    
    if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
    {
        LCVLog(ERROR, "%s: Maximum number of readers reached!\n",
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
    	err = LCVFrdParseListReader(pConfigF,
    			&pReader->reader.list);
    	
    	if(err != SUCCESS)
    	{
    		LCVLog(ERROR, "%s: Error parsing list reader config"
    				"(%d)!\n", __func__, err);
    		goto exit_fail;
    	}
    	pReader->enType = FRD_READER_TYPE_LIST;
    } 
    else if(strcmp(strTemp, "FRD_SEQUENCE_READER") == 0)
    {
    	/* Sequential Reader. */
    	err = LCVFrdParseSequentialReader(pConfigF, 
    			&pReader->reader.seq);
    	if(err != SUCCESS)
    	{
    		LCVLog(ERROR, "%s: Error parsing sequential reader config"
    				"(%d)!\n", __func__, err);
    		goto exit_fail;
    	}
    	pReader->enType = FRD_READER_TYPE_SEQUENCE;
    } else if(strcmp(strTemp, "FRD_CONSTANT_READER") == 0)
    {
    	/* Constant Reader. */
    	err = LCVFrdParseConstantReader(pConfigF,
    			&pReader->reader.constant);
    	if(err != SUCCESS)
    	{
    		LCVLog(ERROR, "%s: Error parsing constant reader config"
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

LCV_ERR LCVFrdGetCurrentFileName(const void *hReaderHandle, 
        char strCurName[])
{
    struct LCV_FRD_READER *pReader;
    
    /* Input validation. */
    if(unlikely((hReaderHandle == NULL) || (strCurName == NULL)))
    {
        LCVLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
                __func__, hReaderHandle, strCurName);
        return -EINVALID_PARAMETER;
    }
    pReader = (struct LCV_FRD_READER*)hReaderHandle;

    switch(pReader->enType)
    {
    case FRD_READER_TYPE_LIST:
    	LCVFrdListGetCurrentFileName(&pReader->reader.list, strCurName);
    	break;
    case FRD_READER_TYPE_SEQUENCE:
    	LCVFrdSeqGetCurrentFileName(&pReader->reader.seq, strCurName);
    	break;
    case FRD_READER_TYPE_CONSTANT:
    	LCVFrdConstGetCurrentFileName(&pReader->reader.constant, strCurName);
    	break;
    default:
    	LCVLog(ERROR, "%s: Unsupported reader type configured (%d)!\n",
    			__func__, pReader->enType);
    	return -EFRD_INVALID_VALUES_CONFIGURED;
    }
    
    return SUCCESS;
}

static void LCVFrdSimCycleCallback()
{
	int rd;
	struct LCV_FRD_READER *pReader;
	
	/* Go through all readers and execute the necessary actions
	 * depending on their type. */
	for(rd = 0; rd < frd.nrOfReaders; rd++)
	{
		pReader = &frd.rd[rd];
		
		switch(pReader->enType)
		{
		case FRD_READER_TYPE_LIST:
			LCVFrdListFetchNextFileName(&pReader->reader.list);
			break;
		case FRD_READER_TYPE_SEQUENCE:
		case FRD_READER_TYPE_CONSTANT:
			/* No operation necessary. */
			break;
		default:
			LCVLog(ERROR, 
					"%s: Unsupported reader type configured (%d)!\n",
					__func__, pReader->enType);
		}
	}
}

/* ------------------------ Sequential reader -----------------------------*/

static LCV_ERR LCVFrdParseSequentialReader(FILE *pConfigF, 
		struct LCV_FRD_SEQUENCE_READER * pReader)
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

LCV_ERR LCVFrdCreateSequenceReader(void **phReaderHandle,
        char strPrefix[], 
        int seqNrDigits,
        char strSuffix[])
{
    struct LCV_FRD_READER           *pReader;
    struct LCV_FRD_SEQUENCE_READER  *pSeqReader;
    
    /* Input validation. */
    if(unlikely(phReaderHandle == NULL || 
            strPrefix == NULL ||
            strSuffix == NULL ||
            seqNrDigits < 1 || seqNrDigits > 10))
    {
        LCVLog(ERROR, 
                "%s(0x%x, 0x%x, %d, 0x%x): "
                "Invalid parameter!\n", __func__, 
                (uint32)phReaderHandle, strPrefix,
                seqNrDigits, strSuffix);
        return -EINVALID_PARAMETER;
    }

    if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
    {
        LCVLog(ERROR, "%s: Maximum number of readers reached!\n",
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

static void LCVFrdSeqGetCurrentFileName(
		const struct LCV_FRD_SEQUENCE_READER *pReader, 
        char strCurName[])
{
    char        strFormat[16];
    char        strSeq[16];	
    uint32      curSeqNr;
    
    /* Get the current time step from the simulation module. */
    curSeqNr = LCVSimGetCurTimeStep();
    
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
static LCV_ERR LCVFrdParseListReader(FILE *pConfigF, 
		struct LCV_FRD_FILELIST_READER * pReader)
{
	char 						strTemp[1024];
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
    	LCVLog(ERROR, "%s: Unable to open file list (%s)! Errno: %s\n",
    			__func__, strTemp, strerror(errno));
    	return -EUNABLE_TO_OPEN_FILE;
    }
    
    strcpy(pReader->strFileList, strTemp);
    
    return SUCCESS;
}

LCV_ERR LCVFrdCreateFileListReader(void **phReaderHandle,
        const char strFileList[])
{
    struct LCV_FRD_READER           *pReader;
    struct LCV_FRD_FILELIST_READER  *pFLReader;
    /* Input validation. */
    if(unlikely(phReaderHandle == NULL || strFileList == NULL ||
            strFileList[0] == '\0'))
    {
        LCVLog(ERROR, "%s(0x%x, %s): Invalid parameter!\n",
                __func__, (uint32)phReaderHandle, strFileList);
        return -EINVALID_PARAMETER;
    }
    
    if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
    {
        LCVLog(ERROR, "%s: Maximum number of readers reached!\n",
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
        LCVLog(ERROR, "%s: Unable to open file list! (%s)\n",
                __func__, strFileList);
        return -EUNABLE_TO_OPEN_FILE;
    }
        
    frd.nrOfReaders++;
    
    pReader->enType = FRD_READER_TYPE_LIST;
    strcpy(pFLReader->strFileList, strFileList);
    *phReaderHandle = (void*)pReader;
    
    return SUCCESS;
}

static inline void LCVFrdListGetCurrentFileName(
		const struct LCV_FRD_FILELIST_READER *pReader, 
        char strCurName[])
{
	strcpy(strCurName, pReader->curFileName); 
}

static void LCVFrdListFetchNextFileName(
		struct LCV_FRD_FILELIST_READER *pReader)
{
	int assigned;
	
	if(unlikely(pReader->pFList == NULL))
	{
		LCVLog(ERROR, "%s: No file list open!\n", __func__);
		return;
	}
	
	/* Get the next file name from the list. */
	assigned = fscanf(pReader->pFList, "%[^\t\n]\n", 
			pReader->curFileName);
	if(unlikely(assigned != 1))
	{
		if(assigned == EOF)
		{
			LCVLog(WARN, "%s: Parse error: End of File! (%s)\n",
					__func__, pReader->strFileList);
		} else {
			LCVLog(ERROR, "%s: Parse error: Unable to parse next "
					"file name! (%s)\n", __func__, 
					pReader->strFileList);
		}
		pReader->curFileName[0] = '\0';
	} else { /* succeeded */
		/* Sanity checks */
		if(unlikely(pReader->curFileName[0] == '\0'))
		{
			LCVLog(WARN, "%s: Next file name is empty string! "
					"Probably not intended.\n", __func__);
		}
	}
}

/* -------------------------- Constant reader ------------------------------*/
static LCV_ERR LCVFrdParseConstantReader(FILE *pConfigF, 
		struct LCV_FRD_CONSTANT_READER * pReader)
{
	int 						assigned;
		
	/* The only parameter is the constant file name. */
	assigned = fscanf(pConfigF, "FILENAME = %s\n",
			pReader->strFN);
	if(unlikely(assigned != 1))
	{
		return -EFRD_PARSING_FAILURE;
	}
	
	return SUCCESS;
}

LCV_ERR LCVFrdCreateConstantReader(void **phReaderHandle, char strFN[])
{
	struct LCV_FRD_READER			*pReader;
	struct LCV_FRD_CONSTANT_READER	*pConstReader;
	
	/* Input validation */
	if(unlikely((phReaderHandle == NULL) ||
				(strFN == NULL) ||
				(*strFN == '\0')))
	{
		LCVLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
					__func__, phReaderHandle, strFN);
		return -EINVALID_PARAMETER;
	}
	
	if(unlikely(frd.nrOfReaders >= MAX_NR_READERS))
    {
        LCVLog(ERROR, "%s: Maximum number of readers reached!\n",
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

static void LCVFrdConstGetCurrentFileName(
		const struct LCV_FRD_CONSTANT_READER *pReader, 
        char strCurName[])
{
	/* Simply copy the constant file name. */
	strcpy(strCurName, pReader->strFN);
}
