/*! @file ipc_target.c
 * @brief Interprocess communication module implementation for host 
 * 
 * @author Markus Berner
 */

# include "framework_types_target.h"

#include "ipc_pub.h"
#include "ipc_priv.h"
#include "framework_intern.h"

/*! The camera module singelton instance. Declared in ipc_shared.c*/
extern struct LCV_IPC ipc; 

LCV_ERR LCVIpcGetParam(const LCV_IPC_CHAN_ID chanID,
        void *pData, 
        const uint32 paramID, 
        const uint32 paramSize)
{
    struct LCV_IPC_MSG      msg;
    LCV_ERR                 err;

    msg.enCmd = CMD_RD_PARAM;
    msg.paramID = paramID;
    msg.paramProp = (uint32)pData;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE)||
            (pData == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
                __func__, pData, paramID, paramSize);
        return -EINVALID_PARAMETER;
    }

    /* This function only works in blocking mode. */
    if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
    {
        LCVLog(ERROR, "%s: Only works in non-blocking mode!\n", __func__);
        return -EBLOCKING_MODE_ONLY;
    }

    /* Send the message. The server will write the requested
     * data directly to the specified data pointer. */
    err = LCVIpcSendMsg(chanID, &msg);
    if(err != SUCCESS)
    {
        return err;
    }  
    
    /* Wait for an acknowledge. As long as the server has not opened
     * the other side yet, we will receive -ENO_MSG_AVAIL. */
    do
    {
        err = LCVIpcRecvMsg(chanID, &msg);
    } while(err == -ENO_MSG_AVAIL);
    
    if(err != SUCCESS)
    {
        return err;
    }

    if(likely(msg.enCmd == CMD_RD_PARAM_ACK))
    {
        if(likely(msg.paramID == paramID))
        {
            return SUCCESS;
        } else {
            LCVLog(ERROR, "%s: Ack did not match to request issued!\n");
            return -EDEVICE;
        }
    } 
    else if(likely(msg.enCmd == CMD_RD_PARAM_NACK))
    {
        if(likely(msg.paramID == paramID))
        {
            return -ENEGATIVE_ACKNOWLEDGE;
        } else {
            LCVLog(ERROR, "%s: Ack did not match to request issued!\n");
            return -EDEVICE;
        }     
    } 
    else
    {
        /* We got the wrong message, this must not happen. */
        LCVLog(ERROR, "%s: Received no ack!\n", __func__);
        return -EDEVICE;
    }
}

LCV_ERR LCVIpcSetParam(const LCV_IPC_CHAN_ID chanID,
        void *pData, 
        const uint32 paramID, 
        const uint32 paramSize)
{
    struct LCV_IPC_MSG      msg;
    LCV_ERR                 err;

    msg.enCmd = CMD_WR_PARAM;
    msg.paramID = paramID;
    msg.paramProp = (uint32)pData;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE) ||
            (pData == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
                __func__, pData, paramID, paramSize);
        return -EINVALID_PARAMETER;
    }

    /* This function only works in blocking mode. */
    if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
    {
        LCVLog(ERROR, "%s: Only works in non-blocking mode!\n", 
                __func__);
        return -EBLOCKING_MODE_ONLY;
    }
    
    /* Send the message. The server will write the requested
     * data directly to the specified data pointer. */
    err = LCVIpcSendMsg(chanID, &msg);
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Error sending request! (%d)\n",
                __func__, err);
        return err;
    }  

    /* Wait for an acknowledge. As long as the server has not opened
     * the other side yet, we will receive -ENO_MSG_AVAIL. */
    do
    {
        err = LCVIpcRecvMsg(chanID, &msg);
    } while(err == -ENO_MSG_AVAIL);
    
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Error receiving acknowledge! (%d)\n",
                __func__, err);
        return err;
    }
    
    if(likely(msg.enCmd == CMD_WR_PARAM_ACK))
    {
        if(likely(msg.paramID == paramID))
        {
            return SUCCESS;
        } else {
            LCVLog(ERROR, "%s: Ack did not match to request issued!\n");
            return -EDEVICE;
        }
    } 
    else if(likely(msg.enCmd == CMD_WR_PARAM_NACK))
    {
        if(likely(msg.paramID == paramID))
        {
            return -ENEGATIVE_ACKNOWLEDGE;
        } else {
            LCVLog(ERROR, "%s: Ack did not match to request issued!\n");
            return -EDEVICE;
        }   
    } 
    else
    {
        /* We got the wrong message, this must not happen. */
        LCVLog(ERROR, "%s: Received no ack!\n", __func__);
        return -EDEVICE;
    }

    return SUCCESS;
}


LCV_ERR LCVIpcGetRequest(const LCV_IPC_CHAN_ID chanID,
        struct LCV_IPC_REQUEST *pRequest)
{
    struct LCV_IPC_MSG msg;
    LCV_ERR err;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE) ||
            (pRequest == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x): Invalid parameter!\n",
                __func__, chanID, pRequest);
        return -EINVALID_PARAMETER;
    }
        
    err = LCVIpcRecvMsg(chanID, &msg);
    if(err != SUCCESS)
    {
        /* Probably -ENO_MSG_AVAILABLE but may also be a 
         * real error. */
        return err;
    }

    switch(msg.enCmd)
    {
    case CMD_RD_PARAM:
        pRequest->enType = REQ_TYPE_READ;
        break;
    case CMD_WR_PARAM:
        pRequest->enType = REQ_TYPE_WRITE;
        break;
    default:
        /* Must not happen. */
        return -EDEVICE;
    }
    pRequest->pAddr = (void*)msg.paramProp;
    pRequest->paramID = msg.paramID;

    return SUCCESS;
}

LCV_ERR LCVIpcAckRequest(const LCV_IPC_CHAN_ID chanID,
        const struct LCV_IPC_REQUEST *pRequest,
        const bool bSucceeded)
{
    struct LCV_IPC_MSG msg;
    LCV_ERR err;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE) ||
            (pRequest == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x, %d): Invalid parameter!\n",
                __func__, chanID, pRequest, bSucceeded);
        return -EINVALID_PARAMETER;
    }
        
    /* Fill out the acknowledge message structure */
    msg.paramProp = (uint32)pRequest->pAddr;
    msg.paramID = pRequest->paramID;
    if(likely(bSucceeded == TRUE))
    {
        switch(pRequest->enType)
        {
        case REQ_TYPE_READ:
            msg.enCmd = CMD_RD_PARAM_ACK;
            break;
        case REQ_TYPE_WRITE:
            msg.enCmd = CMD_WR_PARAM_ACK;
            break;
        default:
            return -EINVALID_PARAMETER;
        }
    } else {
        switch(pRequest->enType)
        {
        case REQ_TYPE_READ:
            msg.enCmd = CMD_RD_PARAM_NACK;
            break;
        case REQ_TYPE_WRITE:
            msg.enCmd = CMD_WR_PARAM_NACK;
            break;
        default:
            return -EINVALID_PARAMETER;
        }
    }

    /* Send the acknowledge. This call may return -ETRY_AGAIN, if
     * the client on the other side of the FIFO did not open the
     * FIFO for reading yet. We pass it on and leave it to the caller
     * to try it again later. */
    err = LCVIpcSendMsg(chanID, &msg);
    if(err != SUCCESS)
    {
        if(err != -ETRY_AGAIN)
        {
            LCVLog(ERROR, "%s: Failed to send acknowledge! (%d)\n", 
                    __func__, err);
        }
        return err;
    }

    return SUCCESS;
}
