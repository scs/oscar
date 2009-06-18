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

struct {
	OSC_ERR (* create, destroy) (void *);
	bool isLoaded;
} OscModuleFunctions[] = {
	[OscModule_log] = { .create = OscLogCreate, .destroy = OscLogDestroy },
	[OscModule_cam] = { .create = OscCamCreate, .destroy = OscCamDestroy },
	[OscModule_cpld] = { .create = OscCpldCreate, .destroy = OscCpldDestroy },
#ifdef TARGET_TYPE_INDXCAM
	[OscModule_lgx] = { .create = OscLgxCreate, .destroy = OscLgxDestroy },
#endif
	[OscModule_sim] = { .create = OscSimCreate, .destroy = OscSimDestroy },
	[OscModule_bmp] = { .create = OscBmpCreate, .destroy = OscBmpDestroy },
	[OscModule_swr] = { .create = OscSwrCreate, .destroy = OscSwrDestroy },
	[OscModule_srd] = { .create = OscSrdCreate, .destroy = OscSrdDestroy },
	[OscModule_ipc] = { .create = OscIpcCreate, .destroy = OscIpcDestroy },
	[OscModule_sup] = { .create = OscSupCreate, .destroy = OscSupDestroy },
	[OscModule_frd] = { .create = OscFrdCreate, .destroy = OscFrdDestroy },
	[OscModule_dspl] = { .create = OscDsplCreate, .destroy = OscDsplDestroy },
	[OscModule_dma] = { .create = OscDmaCreate, .destroy = OscDmaDestroy },
	[OscModule_hsm] = { .create = OscHsmCreate, .destroy = OscHsmDestroy },
	[OscModule_cfg] = { .create = OscCfgCreate, .destroy = OscCfgDestroy },
	[OscModule_clb] = { .create = OscClbCreate, .destroy = OscClbDestroy },
	[OscModule_vis] = { .create = OscVisCreate, .destroy = OscVisDestroy },
	[OscModule_jpg] = { .create = OscJpgCreate, .destroy = OscJpgDestroy }
};

struct OSC_FRAMEWORK fw = { };    /*!< @brief Module singelton instance */

OSC_ERR _OscCreate(int count, enum OscModule * modules) {
OscFunctionBegin

	for (int i = 0; i < count; i += 1)
	{
		OscCall(OscModuleFunctions[modules[i]].create, &fw);
	}
OscFunctionCatch
	OscMark_m("Failed to load the Framework.");
	OscCall(OscDestroy);
OscFunctionEnd
}

OSC_ERR OscDestroy()
{
OscFunctionBegin
	for (int i = 0; i < length(OscModuleFunctions); i+= 1) {
		void (*destroy)(void *) = OscModuleFunctions[i].destroy;
		
		if (destroy != NULL)
			destroy(&fw);
	}
OscFunctionCatch
OscFunctionEnd
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
