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
 * @brief Oscar framework implementation
 * 
 * Create, destroy and module dependency functionality.
 */

 /*!
 * \mainpage
 * \image html camera-title.png
 * \image html scs-logo.png
 * 
 * \section Introduction
 * This is the documentation of the Open-Source Camera (OSCAR) software framework.
 */

#include <stdio.h>
#include "oscar.h"

struct OSC_FRAMEWORK fw;    /*!< @brief Module singelton instance */

OSC_ERR OscCreate(void **phFw) {
OscFunctionBegin
	memset(&fw, 0, sizeof(struct OSC_FRAMEWORK));
	
/*	OscAssert_e( NULL == &fw, EOUT_OF_MEMORY);
	OscAssert_em( NULL != &fw, EOUT_OF_MEMORY);*/
	
	OscMark();
	{
		if (!(((void *)0) == &fw)) {
	OscMark();
			OscLog(ERROR, "%s: %s(): Line %d" ": " "mem" "\n", "oscar.c", __FUNCTION__, 43);
			{
				_OscInternal_err_ = EOUT_OF_MEMORY; goto fail;
			};
		}
	};
	OscMark();
	{
		if (!(((void *)0) != &fw)) {
	OscMark();
			OscLog(ERROR, "%s: %s(): Line %d" ": " "mem" "\n", "oscar.c", __FUNCTION__, 44);
			{
				_OscInternal_err_ = EOUT_OF_MEMORY; goto fail;
			};
		}
	};
	OscMark();
	
	/* OSC Create does not instantiate any modules */
	
	/* Return the handle */
	*phFw = &fw;
		
OscFunctionCatch
	OscMark_m( "Faild to reserve memory!");
	
OscFunctionEnd
}

OSC_ERR OscDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	
	if(NULL == hFw)
	{
		return SUCCESS;
	}
	
	pFw = (struct OSC_FRAMEWORK*)hFw;
	
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
	if(pFw->dspl.useCnt)
	{
		fprintf(stderr, "%s: ERROR: Dspl module still loaded!\n",
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
	memset(hFw, 0, sizeof(struct OSC_FRAMEWORK));
	return SUCCESS;
}


OSC_ERR OscLoadDependencies(void *hFw,
		const struct OSC_DEPENDENCY aryDeps[],
		const uint32 nDeps)
{
	int         i;
	OSC_ERR     err = SUCCESS;
	struct OSC_FRAMEWORK *pFw = (struct OSC_FRAMEWORK*)hFw;
	
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

void OscUnloadDependencies(void *hFw,
		const struct OSC_DEPENDENCY aryDeps[],
		const uint32 nDeps)
{
	int i;
	struct OSC_FRAMEWORK *pFw = (struct OSC_FRAMEWORK*)hFw;

	if(NULL == hFw)
	{
		return SUCCESS;
	}
	
	for(i = 0; i < nDeps; i++)
	{
		aryDeps[nDeps - i - 1].destroy(pFw);
	}
}

OSC_ERR OscGetVersionNumber(
	char *hMajor, 
	char *hMinor, 
	char *hPatch)
{
	*hMajor 	= OSC_VERSION_MAJOR;
	*hMinor 	= OSC_VERSION_MINOR;
	*hPatch 	= OSC_VERSION_PATCH;
	return SUCCESS;
}

OSC_ERR OscGetVersionString( char *hVersion)
{
	sprintf(hVersion, "v%d.%d", OSC_VERSION_MAJOR, OSC_VERSION_MINOR);
	if(OSC_VERSION_PATCH)
	{
		sprintf(hVersion, "%s-p%d", hVersion, OSC_VERSION_PATCH);
	}
	return SUCCESS;
}
