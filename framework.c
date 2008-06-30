/*! @file framework.c
 * @brief Framework implementation
 * 
 * Create, destroy and module dependency functionality.
 *   
 * @author Markus Berner, Samuel Zahnd
 */

#include "framework_priv.h"
#include "framework_error.h"

#include <stdio.h>

struct LCV_FRAMEWORK fw;	/*!< @brief Module singelton instance */

LCV_ERR LCVCreate(void **phFw)
{
	memset(&fw, 0, sizeof(struct LCV_FRAMEWORK));
	
	/* LCV Create does not instantiate any modules */
	
	/* Return the handle */
	*phFw = &fw;
	return SUCCESS;
}

LCV_ERR LCVDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    
    pFw = (struct LCV_FRAMEWORK*)hFw;
    
    /* Check if there are still any modules loaded. */
    if(pFw->bmp.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Bmp module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->cam.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Cam module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->cpld.useCnt)
    {
        fprintf(stderr, "%s: ERROR: CPLD module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->ipc.useCnt)
    {
        fprintf(stderr, "%s: ERROR: IPC module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->lgx.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Lgx module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->log.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Log module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->sim.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Sim module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->srd.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Srd module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->swr.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Swr module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->sup.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Sup module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->frd.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Frd module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }	
    if(pFw->rtl.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Rtl module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }   
    if(pFw->dma.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Dma module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }   
    if(pFw->hsm.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Hsm module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }   
    if(pFw->cfg.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Cfg module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }
    if(pFw->clb.useCnt)
    {
        fprintf(stderr, "%s: ERROR: Clb module still loaded!\n",
                __func__);
        return -ECANNOT_UNLOAD;
    }       
	memset(hFw, 0, sizeof(struct LCV_FRAMEWORK));
	return SUCCESS;
}


LCV_ERR LCVLoadDependencies(void *hFw,
        const struct LCV_DEPENDENCY aryDeps[], 
        const uint32 nDeps)
{
    int         i;
    LCV_ERR     err = SUCCESS;
    struct LCV_FRAMEWORK *pFw = (struct LCV_FRAMEWORK*)hFw;
    
    for(i = 0; i < nDeps; i++)
    {
        err = aryDeps[i].create(pFw);
        if(err < 0)
        {
            fprintf(stderr, "%s: ERROR: "\
                    "Unable to load dependency module %s! (%d)\n",
                    __func__, 
                    aryDeps[i].strName, 
                    err);
            break;
        }
    }
    
    if(err != 0)
    {
        for(i--; i >= 0; i--)
        {
            aryDeps[i].destroy(pFw);
        }
    }
    return err;
}

void LCVUnloadDependencies(void *hFw,
        const struct LCV_DEPENDENCY aryDeps[], 
        const uint32 nDeps)
{
    int i;
    struct LCV_FRAMEWORK *pFw = (struct LCV_FRAMEWORK*)hFw;
    
    for(i = 0; i < nDeps; i++)
    {
        aryDeps[nDeps - i - 1].destroy(pFw);
    }
}
