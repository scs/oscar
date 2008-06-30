/*! @file cam_multibuffer.c
 * @brief Multibuffer implementation
 
 * Used to automatically manage multi
 * buffers (e.g. double buffers).
 * 
 * @author Markus Berner
 */
#include "cam_multibuffer.h"
/* For defines */
#include "cam_pub.h"
/* For logging */
#include <log/log_pub.h>
#include <string.h>

/*==================== Private method prototypes =======================*/
/*********************************************************************//*!
 * @brief Returns the next buffer in line.
 * 
 * Returns the correct buffer id to sync next and adjusts the idNextSync
 * and idNextCapture variables accordingly. Can return 
 * LCV_CAM_INVALID_BUFFER_ID if sync currently not possible.
 * 
 * @see LCVCamMultiBufferCapture
 * @see LCVCamMultiBufferSync
 * @see LCVCamCreateMultiBuffer
 * 
 * @param pMB Pointer to corresponding multi buffer.
 * @param fbID Frame buffer ID to get the next from.
 * @return Buffer ID for the next capture.
 *//*********************************************************************/
static uint8 LCVCamMultiBufferGetNext(struct LCV_CAM_MULTIBUFFER * pMB,
        uint8 fbID);


/*=========================== Code =====================================*/

LCV_ERR LCVCamMultiBufferCreate(struct LCV_CAM_MULTIBUFFER * pMB,
        const uint8 multiBufferDepth,
        const uint8 bufferIDs[])
{
    if(pMB->multiBufferDepth != 0)
    {
        LCVLog(NOTICE, "%s: \
                Replacing existing multibuffer.\n",
                __func__);
    }

    pMB->multiBufferDepth = multiBufferDepth;
    memcpy(&pMB->fbIDs,
            bufferIDs,
            multiBufferDepth*sizeof(uint8));

    pMB->idNextCapture = pMB->fbIDs[0];
    /* Since there was no capture yet, we have nothing to sync to */
    pMB->idNextSync = LCV_CAM_INVALID_BUFFER_ID;

    return SUCCESS;
}

inline LCV_ERR LCVCamMultiBufferDestroy(struct LCV_CAM_MULTIBUFFER * pMB)
{
    if (pMB->multiBufferDepth == 0)
    {
        LCVLog(WARN, "%s: Nothing to delete.\n", __func__);
    }
    pMB->multiBufferDepth = 0;
    return SUCCESS;
}

static uint8 LCVCamMultiBufferGetNext(struct LCV_CAM_MULTIBUFFER * pMB,
        uint8 fbID)
{
    int i;
    uint8 ret;

    ret = LCV_CAM_INVALID_BUFFER_ID;
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

inline uint8 LCVCamMultiBufferGetCapBuf(
        const struct LCV_CAM_MULTIBUFFER *pMB)
{
    return pMB->idNextCapture;
}

void LCVCamMultiBufferCapture(struct LCV_CAM_MULTIBUFFER *pMB)
{
    uint8 cur;

    cur = pMB->idNextCapture;

    /* Find the next buffer to capture to */
    pMB->idNextCapture =
    LCVCamMultiBufferGetNext(pMB, pMB->idNextCapture);

    /* Find the next buffer to sync */
    if(pMB->idNextSync == LCV_CAM_INVALID_BUFFER_ID)
    {
        pMB->idNextSync = cur;
    }
    else if(pMB->idNextSync == cur)
    {
        pMB->idNextSync =
        LCVCamMultiBufferGetNext(pMB, pMB->idNextSync);
    }
}

inline uint8 LCVCamMultiBufferGetSyncBuf(
        const struct LCV_CAM_MULTIBUFFER *pMB)
{
    return pMB->idNextSync;
}

void LCVCamMultiBufferSync(struct LCV_CAM_MULTIBUFFER * pMB)
{
    uint8 cur;

    cur = pMB->idNextSync;

    pMB->idNextSync =
        LCVCamMultiBufferGetNext(pMB, pMB->idNextSync);
    
    if(pMB->idNextSync == pMB->idNextCapture)
    {
        /* We need to prevent a sync before the capture has happened */
        pMB->idNextSync = LCV_CAM_INVALID_BUFFER_ID;
    }
}
