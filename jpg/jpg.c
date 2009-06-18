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

struct OscModule OscModule_Jpg = {
	.create = OscJpgCreate,
	.destroy = OscJpgDestroy,
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

OSC_ERR OscJpgCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->jpg.useCnt != 0)
	{
		pFw->jpg.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	/* Load the module dependencies of this module. */
	err = OscLoadDependencies(pFw,
			jpg_deps,
			sizeof(jpg_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load dependencies! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	/* Increment the use count */
	pFw->bmp.hHandle = NULL;
	pFw->bmp.useCnt++;
	
	return SUCCESS;
}

void OscJpgDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->jpg.useCnt--;
	if(pFw->jpg.useCnt > 0)
	{
		return;
	}
	
	OscUnloadDependencies(pFw,
			jpg_deps,
			sizeof(jpg_deps)/sizeof(struct OSC_DEPENDENCY));
}
