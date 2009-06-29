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

static struct OscModule ** loadedModuels = NULL;

OscFunction(static loadModules, struct OscModule ** deps)
	for (struct OscModule ** dep = deps; *dep != NULL; dep += 1) {
		OscAssert((*dep)->useCount >= 0);
		
		if ((*dep)->useCount == 0) {
			char * name = (*dep)->name;
			
			if (name == NULL)
				name = "<noname>";
			
			OscCall(loadModules, (*dep)->dependencies);
			
			OscMark_m("Loading module %s ...", name);
			if ((*dep)->create != NULL)
				OscCall((*dep)->create);
		}
		
		(*dep)->useCount += 1;
	}
OscFunctionEnd()

OscFunction(static unloadModules, struct OscModule ** deps)
	for (struct OscModule ** dep = deps; *dep != NULL; dep += 1) {
		(*dep)->useCount -= 1;
		
		if ((*dep)->useCount == 0) {
			if ((*dep)->destroy != NULL)
				OscCall((*dep)->destroy);
			
			OscCall(unloadModules, (*dep)->dependencies);
		}
		
		OscAssert((*dep)->useCount >= 0);
	}
OscFunctionEnd()

OscFunction(OscCreateFunction, struct OscModule ** modules)
	OscAssert_m(loadedModuels == NULL, "The Framework is already loaded!");
	
	// This requires modules point to static data which is done so by the OscCreate Macro. But it may not be a good idea ...
	loadedModuels = modules;
	OscCall(loadModules, loadedModuels);

OscFunctionCatch()
	OscMark_m("Failed to load the Framework.");
	OscCall(OscDestroy);
OscFunctionEnd()

OscFunction(OscDestroy)
	OscAssert_m(loadedModuels != NULL, "The Framework is not loaded!");
	
	OscCall(unloadModules, loadedModuels);
	loadedModuels = NULL;
OscFunctionEnd()

OSC_ERR OscGetVersionNumber(char *hMajor, char *hMinor, char *hPatch)
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
