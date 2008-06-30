/*! @file swr_host.c
 * @brief Stimuli writer module implementation for host.
 *
 ************************************************************************/

#include "oscar_intern.h"

#include "swr_pub.h"
#include "swr_priv.h"

#include <sim/sim_pub.h>

struct OSC_SWR swr; /*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY swr_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscSwrCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->swr.useCnt != 0)
    {
        pFw->swr.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }

    /* Load the module swr_deps of this module. */
    err = OscLoadDependencies(pFw,
            swr_deps,
            sizeof(swr_deps)/sizeof(struct OSC_DEPENDENCY));

    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load swr_deps! (%d)\n",
                __func__,
                err);
        return err;
    }

    memset(&swr, 0, sizeof(struct OSC_SWR));

    OscSimRegisterCycleCallback( &OscSwrCycleCallback);

    /* Increment the use count */
    pFw->swr.hHandle = (void*)&swr;
    pFw->swr.useCnt++;

    return SUCCESS;
}

void OscSwrDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    uint16 wrId;    

    pFw = (struct OSC_FRAMEWORK *)hFw;
    /* Check if we really need to release or whether we still
     * have users. */
    pFw->swr.useCnt--;
    if(pFw->swr.useCnt > 0)
    {
        return;
    }

    OscUnloadDependencies(pFw,
            swr_deps,
            sizeof(swr_deps)/sizeof(struct OSC_DEPENDENCY));

    /* close all files */
    for( wrId = 0; wrId<swr.nrOfWriters; wrId++)
    {
        fclose( swr.wr[ wrId].pFile);
        OscLog(INFO, "Close %s\n", &swr.wr[ wrId].strFile);
    }
    
    
    memset(&swr, 0, sizeof(struct OSC_SWR));
}


OSC_ERR OscSwrCreateWriter( 
        void** ppWriter,        
        const char* strFile, 
        const bool bReportTime,
        const bool bReportCyclic )
{
    FILE* pFile;
    uint16 id;

    /* Open writer file */
    pFile = fopen( strFile, "w");
    if( pFile == 0)
    {
        printf("Error: Unable open to writer file %s.\n",
                strFile);
        return -EUNABLE_TO_OPEN_FILE;
    }   
    if( MAX_NR_WRITER == swr.nrOfWriters)
    {
        printf("Error: Maximum number of writers exhausted");
        return -ENR_OF_INSTANCES_EXHAUSTED;
    }
    id = swr.nrOfWriters;
    swr.nrOfWriters ++;
    swr.wr[id].pFile = pFile;
    swr.wr[id].bReportTime = bReportTime;
    swr.wr[id].bReportCyclic = bReportCyclic;
    
    strcpy(swr.wr[id].strFile, strFile);
    OscLog(INFO, "Open %s\n", strFile);
    
    *ppWriter = &swr.wr[ id];

    return SUCCESS;
}

OSC_ERR OscSwrRegisterSignal( 
        void** ppSignal,        
        const void* pWriter, 
        const char* strSignal,
        const enum EnOscSwrSignalType type,        
        const void* pDefaultValue,
        const char* strFormat )
{
    struct OSC_SWR_WRITER* pWr;
    struct OSC_SWR_SIGNAL* pSig;
    uint16 id;

    pWr = (void*) pWriter;
    if( MAX_NR_SIGNAL_PER_WRITER == pWr->nrOfSignals)
    {
        printf("Error: Maximum number of signals exhausted");
        return -ENR_OF_INSTANCES_EXHAUSTED;
    }
    id = pWr->nrOfSignals;
    pWr->nrOfSignals++;
    pSig = &pWr->sig[ id];
    
    pWr->sig[ id].strName = (char*)strSignal;
    pWr->sig[ id].type = type;
    
    if( pDefaultValue)
    {
        switch( pWr->sig[ id].type)
        {
        case SWR_INTEGER:
            pSig->value.nValue = *(int32*)pDefaultValue;
            break;
        case SWR_FLOAT:
            pSig->value.fValue = *(float*)pDefaultValue;
            break;
        case SWR_STRING:
            strcpy( pSig->value.strValue, (char*)pDefaultValue);
        }    
    }
    
    strcpy( pWr->sig[ id].strFormat, strFormat);    
    
    *ppSignal = pSig;
    return SUCCESS;
}

OSC_ERR OscSwrUpdateSignal( 
        const void* pSignal, 
        const void* pValue)       
{
    struct OSC_SWR_SIGNAL* pSig;
    pSig = (void*) pSignal;
    
    switch( pSig->type)
    {
    case SWR_INTEGER:
        pSig->value.nValue = *(int32*)pValue;
        break;
    case SWR_FLOAT:
        pSig->value.fValue = *(float*)pValue;
        break;
    case SWR_STRING:
        strcpy( pSig->value.strValue, (char*)pValue);
    }
    return SUCCESS;
}

OSC_ERR OscSwrManualReport( const void* pWriter)
{
    OscSwrReport( pWriter);
    return SUCCESS;
}

void OscSwrReport( const void* pWriter)
{
    uint16 sigId;    
    struct OSC_SWR_WRITER* pWr;
    struct OSC_SWR_SIGNAL* pSig;
    
    pWr = (void*) pWriter;
    
    /* print descriptor line */
    if( FALSE == pWr->bDescriptorPrinted)
    {
        if(pWr->bReportTime)
        {
            fprintf( pWr->pFile, "!\tTime\t");
        }
        for( sigId = 0; sigId < pWr->nrOfSignals; sigId++)
        {
            fprintf( pWr->pFile, "%s\t",
                    pWr->sig[ sigId].strName);
        }
        fprintf( pWr->pFile, "\n");
        pWr->bDescriptorPrinted = TRUE;

    }
    
    /* print instruction line */
    if(pWr->bReportTime)
    {    
        fprintf( pWr->pFile, "@\t%lu\t", OscSimGetCurTimeStep() );
    }
    for( sigId = 0; sigId < pWr->nrOfSignals; sigId++)
    { 
        pSig = &pWr->sig[ sigId];
        switch( pSig->type)
        {
        case SWR_INTEGER:
            fprintf( pWr->pFile, pSig->strFormat,
                    pSig->value.nValue);
            break;
        case SWR_FLOAT:
            fprintf( pWr->pFile, pSig->strFormat,
                    pSig->value.fValue);
            break;
        case SWR_STRING:
            fprintf( pWr->pFile, pSig->strFormat,
                    pSig->value.strValue); 
        }        
        fprintf( pWr->pFile, "\t");
    }
    fprintf( pWr->pFile, "\n");    
}

void OscSwrCycleCallback( void)
{
    uint16 wrId;

    for( wrId = 0; wrId<swr.nrOfWriters; wrId++)
    {
        /* report all writers (all signals) if cyclic reporting
         * is selected */
        if( swr.wr[ wrId].bReportCyclic)
        {
            OscSwrReport( &swr.wr[ wrId]);            
        }
    }

    return;
}

