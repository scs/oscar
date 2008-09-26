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

/*! @file cam_multibuffer.h
 * @brief Private multibuffer definitions
 * 
 */
#ifndef CAM_MULTIBUFFER_H_
#define CAM_MULTIBUFFER_H_

#ifdef OSC_HOST
#include <oscar_types_host.h>
#else
#include <oscar_types_target.h>
#endif /* OSC_HOST */

#include "mt9v032.h"

/*! @brief The structure representing a multibuffer.
 * 
 * Used to organize frame buffers into automatically managed
 * multi buffers (e.g. double buffers) */
struct OSC_CAM_MULTIBUFFER
{
	/*! The depth of this multibuffer, e.g. 2 for double-buffering */
	uint8 multiBufferDepth;
	/*! The frame buffer IDs of the frame buffers forming this
	 * multibuffer. */
	uint8 fbIDs[MAX_NR_FRAME_BUFFERS];
	/*! ID of the frame buffer where to start the next capture to. */
	uint8 idNextCapture;
	/*! ID of the frame buffer where to sync and read from next. */
	uint8 idNextSync;
};

/*=================== Public Method prototypes =========================*/

/*********************************************************************//*!
 * @brief Configure multiple frame buffer to form a multi buffer.
 * 
 * This can be used e.g. to allow automatic management of double or
 * triple buffering. Just specify the depth of the multi buffer and the
 * frame buffer IDs forming the multi buffer. Afterwards the commands
 * requiring a frame buffer number can be supplied with
 * OSC_CAM_MULTI_BUFFER to acces the data in the automatically managed
 * multi buffer.
 * No input validation is done (must be done beforehand).
 * 
 * @see OscCamMultiBufferDestroy
 * @see OscCamMultiBufferCapture
 * @see OscCamMultiBufferSync
 * 
 * @param pMB Memory location where the multi buffer is to be initialized.
 * @param multiBufferDepth The depth of the multi buffer.
 * @param bufferIDs Array of the buffer IDs forming the multi buffer.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
OSC_ERR OscCamMultiBufferCreate(struct OSC_CAM_MULTIBUFFER * pMB,
		const uint8 multiBufferDepth,
		const uint8 bufferIDs[]);

/*********************************************************************//*!
 * @brief Delete a previously configured multi buffer
 * @see OscCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer to destroy.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
inline OSC_ERR OscCamMultiBufferDestroy(struct OSC_CAM_MULTIBUFFER * pMB);

/*********************************************************************//*!
 * @brief Return the frame buffer to capture to next.
 * 
 * Returns the correct buffer id to capture to next.
 * 
 * @see OscCamMultiBufferCapture
 * @see OscCamMultiBufferSync
 * @see OscCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 * @return Buffer ID for the next capture.
 *//*********************************************************************/
inline uint8 OscCamMultiBufferGetCapBuf(
		const struct OSC_CAM_MULTIBUFFER *pMB);

/*********************************************************************//*!
 * @brief Execute all multi buffer management associated with a capture.
 * 
 * Adjusts the idNextSync and idNextCapture variables. Captures are
 * always possible, the oldest picture simply gets overwritten.
 * 
 * @see OscCamMultiBufferGetCaptBuf
 * @see OscCamMultiBufferSync
 * @see OscCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 *//*********************************************************************/
void OscCamMultiBufferCapture(struct OSC_CAM_MULTIBUFFER * pMB);

/*********************************************************************//*!
 * @brief Return the frame buffer to sync to next.
 * 
 * Returns the buffer id of the oldest valid image in the frame buffers.
 * Returns OSC_CAM_INVALID_BUFFER_ID if no valid image exists.
 * 
 * @see OscCamMultiBufferCapture
 * @see OscCamMultiBufferSync
 * @see OscCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 * @return Buffer ID for the next capture.
 *//*********************************************************************/
inline uint8 OscCamMultiBufferGetSyncBuf(
		const struct OSC_CAM_MULTIBUFFER *pMB);

/*********************************************************************//*!
 * @brief Execute all multi buffer management associated with a sync.
 * 
 * Adjusts the idNextSync and idNextCapture variables. Must not be called
 * if a previous call to OscCamMultiBufferGetSyncBuf failed.
 * 
 * @see OscCamMultiBufferGetSyncBuf
 * @see OscCamMultiBufferCapture
 * @see OscCamCreateMultiBuffer
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 *//*********************************************************************/
void OscCamMultiBufferSync(struct OSC_CAM_MULTIBUFFER * pMB);

#endif /*CAM_MULTIBUFFER_H_*/
