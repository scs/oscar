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

/*! @file cam_multibuffer.c
 * @brief Multibuffer implementation
	
 * Used to automatically manage multi
 * buffers (e.g. double buffers).
 * 
 */
#include "cam_multibuffer.h"
/* For defines */
#include "include/cam.h"
/* For logging */
#include <include/log.h>
#include <string.h>

/*==================== Private method prototypes =======================*/
/*********************************************************************//*!
 * @brief Returns the next buffer in line.
 * 
 * Returns the correct buffer id to sync next and adjusts the idNextSync
 * and idNextCapture variables accordingly. Can return
 * OSC_CAM_INVALID_BUFFER_ID if sync currently not possible.
 * 
 * @see OscCamMultiBufferCapture
 * @see OscCamMultiBufferSync
 * @see OscCamCreateMultiBuffer
 * 
 * @param pMB Pointer to corresponding multi buffer.
 * @param fbID Frame buffer ID to get the next from.
 * @return Buffer ID for the next capture.
 *//*********************************************************************/
static uint8 OscCamMultiBufferGetNext(struct OSC_CAM_MULTIBUFFER * pMB,
		uint8 fbID);


/*=========================== Code =====================================*/

OSC_ERR OscCamMultiBufferCreate(struct OSC_CAM_MULTIBUFFER * pMB,
		const uint8 multiBufferDepth,
		const uint8 bufferIDs[])
{
	if(pMB->multiBufferDepth != 0)
	{
		OscLog(NOTICE, "%s: \
				Replacing existing multibuffer.\n",
				__func__);
	}

	pMB->multiBufferDepth = multiBufferDepth;
	memcpy(&pMB->fbIDs,
			bufferIDs,
			multiBufferDepth*sizeof(uint8));

	pMB->idNextCapture = pMB->fbIDs[0];
	/* Since there was no capture yet, we have nothing to sync to */
	pMB->idNextSync = OSC_CAM_INVALID_BUFFER_ID;

	return SUCCESS;
}

inline OSC_ERR OscCamMultiBufferDestroy(struct OSC_CAM_MULTIBUFFER * pMB)
{
	if (pMB->multiBufferDepth == 0)
	{
		OscLog(WARN, "%s: Nothing to delete.\n", __func__);
	}
	pMB->multiBufferDepth = 0;
	return SUCCESS;
}

static uint8 OscCamMultiBufferGetNext(struct OSC_CAM_MULTIBUFFER * pMB,
		uint8 fbID)
{
	int i;
	uint8 ret;

	ret = OSC_CAM_INVALID_BUFFER_ID;
	for (i = 0; i < pMB->multiBufferDepth; i++)
	{
		if (pMB->fbIDs[i] == fbID)
		{
			i++; /* Select the next buffer */
			if (i == pMB->multiBufferDepth)
			{
				i = 0; /* wrap-around */
			}
			ret = pMB->fbIDs[i];
			return ret;
		}
	}
	return ret;
}

inline uint8 OscCamMultiBufferGetCapBuf(
		const struct OSC_CAM_MULTIBUFFER *pMB)
{
	return pMB->idNextCapture;
}

void OscCamMultiBufferCapture(struct OSC_CAM_MULTIBUFFER *pMB)
{
	uint8 cur;

	cur = pMB->idNextCapture;

	/* Find the next buffer to capture to */
	pMB->idNextCapture =
	OscCamMultiBufferGetNext(pMB, pMB->idNextCapture);

	/* Find the next buffer to sync */
	if(pMB->idNextSync == OSC_CAM_INVALID_BUFFER_ID)
	{
		pMB->idNextSync = cur;
	}
	else if(pMB->idNextSync == cur)
	{
		pMB->idNextSync =
		OscCamMultiBufferGetNext(pMB, pMB->idNextSync);
	}
}

inline uint8 OscCamMultiBufferGetSyncBuf(
		const struct OSC_CAM_MULTIBUFFER *pMB)
{
	return pMB->idNextSync;
}

void OscCamMultiBufferSync(struct OSC_CAM_MULTIBUFFER * pMB)
{
	uint8 cur;

	cur = pMB->idNextSync;

	pMB->idNextSync =
		OscCamMultiBufferGetNext(pMB, pMB->idNextSync);
	
	if(pMB->idNextSync == pMB->idNextCapture)
	{
		/* We need to prevent a sync before the capture has happened */
		pMB->idNextSync = OSC_CAM_INVALID_BUFFER_ID;
	}
}
