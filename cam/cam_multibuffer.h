/*! @file cam_multibuffer.h
* @brief Private multibuffer definitions
 * 
 * @author Markus Berner
 */
#ifndef CAM_MULTIBUFFER_H_
#define CAM_MULTIBUFFER_H_

#ifdef LCV_HOST
#include <framework_types_host.h>
#else
#include <framework_types_target.h>
#endif /* LCV_HOST */

#include "mt9v032.h"

/*! @brief The structure representing a multibuffer.
 * 
 * Used to organize frame buffers into automatically managed 
 * multi buffers (e.g. double buffers) */
struct LCV_CAM_MULTIBUFFER
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
 * LCV_CAM_MULTI_BUFFER to acces the data in the automatically managed
 * multi buffer.
 * No input validation is done (must be done beforehand).
 * 
 * @see LCVCamMultiBufferDestroy
 * @see LCVCamMultiBufferCapture
 * @see LCVCamMultiBufferSync
 * 
 * @param pMB Memory location where the multi buffer is to be initialized.
 * @param multiBufferDepth The depth of the multi buffer.
 * @param bufferIDs Array of the buffer IDs forming the multi buffer.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
LCV_ERR LCVCamMultiBufferCreate(struct LCV_CAM_MULTIBUFFER * pMB,
        const uint8 multiBufferDepth,
        const uint8 bufferIDs[]);

/*********************************************************************//*!
 * @brief Delete a previously configured multi buffer
 * @see LCVCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer to destroy.
 * @return SUCCESS or an appropriate error code
 *//*********************************************************************/
inline LCV_ERR LCVCamMultiBufferDestroy(struct LCV_CAM_MULTIBUFFER * pMB);

/*********************************************************************//*!
 * @brief Return the frame buffer to capture to next.
 * 
 * Returns the correct buffer id to capture to next.
 * 
 * @see LCVCamMultiBufferCapture
 * @see LCVCamMultiBufferSync
 * @see LCVCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 * @return Buffer ID for the next capture.
 *//*********************************************************************/
inline uint8 LCVCamMultiBufferGetCapBuf(
        const struct LCV_CAM_MULTIBUFFER *pMB);

/*********************************************************************//*!
 * @brief Execute all multi buffer management associated with a capture.
 * 
 * Adjusts the idNextSync and idNextCapture variables. Captures are
 * always possible, the oldest picture simply gets overwritten. 
 * 
 * @see LCVCamMultiBufferGetCaptBuf
 * @see LCVCamMultiBufferSync
 * @see LCVCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 *//*********************************************************************/
void LCVCamMultiBufferCapture(struct LCV_CAM_MULTIBUFFER * pMB);

/*********************************************************************//*!
 * @brief Return the frame buffer to sync to next.
 * 
 * Returns the buffer id of the oldest valid image in the frame buffers.
 * Returns LCV_CAM_INVALID_BUFFER_ID if no valid image exists.
 * 
 * @see LCVCamMultiBufferCapture
 * @see LCVCamMultiBufferSync
 * @see LCVCamMultiBufferCreate
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 * @return Buffer ID for the next capture.
 *//*********************************************************************/
inline uint8 LCVCamMultiBufferGetSyncBuf(
        const struct LCV_CAM_MULTIBUFFER *pMB);

/*********************************************************************//*!
 * @brief Execute all multi buffer management associated with a sync.
 * 
 * Adjusts the idNextSync and idNextCapture variables. Must not be called
 * if a previous call to LCVCamMultiBufferGetSyncBuf failed.
 * 
 * @see LCVCamMultiBufferGetSyncBuf
 * @see LCVCamMultiBufferCapture
 * @see LCVCamCreateMultiBuffer
 * 
 * @param pMB Pointer to multi buffer this operation is done on.
 *//*********************************************************************/
void LCVCamMultiBufferSync(struct LCV_CAM_MULTIBUFFER * pMB);

#endif /*CAM_MULTIBUFFER_H_*/
