/*! @file ipc_host.c
 * @brief Interprocess communication module implementation for host 
 * 
 * @author Markus Berner
 */

#ifdef LCV_HOST
# include "framework_types_host.h"
#else
# include "framework_types_target.h"
#endif

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
    msg.paramProp = (uint32)paramSize;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE) ||
            (pData == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
                __func__, chanID, pData, paramID, paramSize);
        return -EINVALID_PARAMETER;
    }

    /* This function only works in blocking mode. */
    if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
    {
        LCVLog(ERROR, "%s: Only works in blocking mode!\n", __func__);
        return -EBLOCKING_MODE_ONLY;
    }

    /* Send the message. The server will write the requested
     * data directly to the specified data pointer. */
    err = LCVIpcSendMsg(chanID, &msg);
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Error sending message! (%d)\n",
                __func__,
                err);
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
        LCVLog(ERROR, "%s: Error receiving message! (%d)\n", 
                __func__,
                err);
        return err;
    }
    
    if(unlikely(msg.enCmd != CMD_RD_PARAM_ACK && 
            msg.enCmd != CMD_RD_PARAM_NACK))
    {
        /* We got the wrong message. */

        LCVLog(ERROR, "%s: Received wrong message!\n", __func__);
        return -EDEVICE;
    }

    /* Wait for the actual data. As long as the server has not opened
     * the other side yet, we will receive -ENO_MSG_AVAIL. */
    do
    {
        err = LCVIpcRecv(chanID, pData, paramSize);
    } while(err == -ENO_MSG_AVAIL);
    
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Error receiving data! (%d)\n", 
                __func__,
                err);
        return err;
    }
    
    
    if(likely(msg.enCmd == CMD_RD_PARAM_ACK))
    {
        return SUCCESS;
    } 
    else
    {
        /* NACK */
        return -ENEGATIVE_ACKNOWLEDGE;
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
    msg.paramProp = paramSize;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE) ||
            (pData == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
                __func__, chanID, pData, paramID, paramSize);
        return -EINVALID_PARAMETER;
    }

    /* This function only works in blocking mode. */
    if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
    {
        LCVLog(ERROR, "%s: Only works in blocking mode!\n", __func__);
        return -EBLOCKING_MODE_ONLY;
    }

    /* Send the message. The server will write the requested
     * data directly to the specified data pointer. */
    err = LCVIpcSendMsg(chanID, &msg);
    if(err != SUCCESS)
    {
        return err;
    }  

    err = LCVIpcSend(chanID, pData, paramSize);
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

    if(likely(msg.enCmd == CMD_WR_PARAM_ACK))
    {
        return SUCCESS;
    } 
    else
    {
        /* NACK */
        return -ENEGATIVE_ACKNOWLEDGE;
    }
}

LCV_ERR LCVIpcGetRequest(const LCV_IPC_CHAN_ID chanID,
        struct LCV_IPC_REQUEST *pRequest)
{
    struct LCV_IPC_MSG msg;
    struct LCV_IPC_PARAM_MEMORY *pTempMem;
    LCV_ERR err = SUCCESS;

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

    /* msg.paramProp specifies the size of the data to read
     * for the host. */
    pTempMem = 
        malloc(msg.paramProp + sizeof(struct LCV_IPC_PARAM_MEMORY));

    if(pTempMem == NULL)
        return -EOUT_OF_MEMORY;

    /* Remember the length of the temporary memory area for 
     * LCVIpcAckRequest. */
    pTempMem->memLen = msg.paramProp;
    pRequest->pAddr = &pTempMem->data;

    switch(msg.enCmd)
    {
    case CMD_RD_PARAM:
        pRequest->enType = REQ_TYPE_READ;
        pRequest->paramID = msg.paramID;

        /* All done for read. */
        return SUCCESS;
        break;
    case CMD_WR_PARAM:
        /* Will receive data to write further down. */
        pRequest->enType = REQ_TYPE_WRITE;
        pRequest->paramID = msg.paramID;
        break;
    default:
        /* Must not happen. */
        err = -EDEVICE;
        goto exit_fail;
    }
    
    do
    {
        usleep(1); /* yield */
        err = LCVIpcRecv(chanID, pRequest->pAddr, pTempMem->memLen);
    } while(err == -ENO_MSG_AVAIL);

    if(err != SUCCESS)
    {
        goto exit_fail;
    }

    return SUCCESS;

    exit_fail:
    free(pTempMem);
    return err;
}

LCV_ERR LCVIpcAckRequest(const LCV_IPC_CHAN_ID chanID,
        const struct LCV_IPC_REQUEST *pRequest,
        const bool bSucceeded)
{
    struct LCV_IPC_MSG              msg;
    struct LCV_IPC_PARAM_MEMORY     *pMem;
    uint8                           *pTemp;
    LCV_ERR                         err;

    /* Input validation */
    if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
            (ipc.arybIpcChansBusy[chanID] == FALSE) ||
            (pRequest == NULL)))
    {
        LCVLog(ERROR, "%s(%d, 0x%x, %d): Invalid parameter!\n",
                __func__, chanID, pRequest, bSucceeded);
        return -EINVALID_PARAMETER;
    }
        
    err = SUCCESS;
    /* Recover the struct LCV_IPC_PARAM_MEMORY pointer from the pointer
     * to its member data. We need this to know the length of the 
     * parameter memory area to be able to send it. */
    pTemp = (uint8*)pRequest->pAddr;
    /* Shift the whole structure back by the length of the member before
     * the actual data.
     * This looks ugly but is save to do since the value of sizeof
     * is determined at compile time. */
    pTemp -= sizeof(pMem->memLen); 
    pMem = (struct LCV_IPC_PARAM_MEMORY*)pTemp;

    msg.paramProp = 0;
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
            err = -EINVALID_PARAMETER;
            goto exit;
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
            err = -EINVALID_PARAMETER;
            goto exit;
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
            LCVLog(ERROR, "%s: Failed to send acknowledge!\n", __func__);
        }
        goto exit;
    }

    /* If this was a read command we need to send back its result. */
    if(pRequest->enType == CMD_RD_PARAM)
    {
        err = LCVIpcSend(chanID, &pMem->data, pMem->memLen);
        if(err != SUCCESS)
        {
            LCVLog(ERROR, "%s: Unable to send data.\n", __func__);
            goto exit;
        }
    }
    
exit:
    free(pMem);
    return err;
}
