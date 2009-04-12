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

/*! @file ipc_host.c
 * @brief Interprocess communication module implementation for host
 * 
 */

#ifdef OSC_HOST
# include "oscar_types_host.h"
#else
# include "oscar_types_target.h"
#endif

#include "include/ipc.h"
#include "ipc.h"
#include "oscar_intern.h"

/*! The camera module singelton instance. Declared in ipc_shared.c*/
extern struct OSC_IPC ipc;

OSC_ERR OscIpcGetParam(const OSC_IPC_CHAN_ID chanID,
		void *pData,
		const uint32 paramID,
		const uint32 paramSize)
{
	struct OSC_IPC_MSG      msg;
	OSC_ERR                 err;

	msg.enCmd = CMD_RD_PARAM;
	msg.paramID = paramID;
	msg.paramProp = (uint32)paramSize;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE) ||
			(pData == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
				__func__, chanID, pData, paramID, paramSize);
		return -EINVALID_PARAMETER;
	}

	/* This function only works in blocking mode. */
	if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
	{
		OscLog(ERROR, "%s: Only works in blocking mode!\n", __func__);
		return -EBLOCKING_MODE_ONLY;
	}

	/* Send the message. The server will write the requested
	 * data directly to the specified data pointer. */
	err = OscIpcSendMsg(chanID, &msg);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error sending message! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	/* Wait for an acknowledge. As long as the server has not opened
	 * the other side yet, we will receive -ENO_MSG_AVAIL. */
	do
	{
		err = OscIpcRecvMsg(chanID, &msg);
	} while(err == -ENO_MSG_AVAIL);
	
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error receiving message! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	if(unlikely(msg.enCmd != CMD_RD_PARAM_ACK &&
			msg.enCmd != CMD_RD_PARAM_NACK))
	{
		/* We got the wrong message. */

		OscLog(ERROR, "%s: Received wrong message!\n", __func__);
		return -EDEVICE;
	}

	/* Wait for the actual data. As long as the server has not opened
	 * the other side yet, we will receive -ENO_MSG_AVAIL. */
	do
	{
		err = OscIpcRecv(chanID, pData, paramSize);
	} while(err == -ENO_MSG_AVAIL);
	
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error receiving data! (%d)\n",
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

OSC_ERR OscIpcSetParam(const OSC_IPC_CHAN_ID chanID,
		void *pData,
		const uint32 paramID,
		const uint32 paramSize)
{
	struct OSC_IPC_MSG      msg;
	OSC_ERR                 err;

	msg.enCmd = CMD_WR_PARAM;
	msg.paramID = paramID;
	msg.paramProp = paramSize;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE) ||
			(pData == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
				__func__, chanID, pData, paramID, paramSize);
		return -EINVALID_PARAMETER;
	}

	/* This function only works in blocking mode. */
	if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
	{
		OscLog(ERROR, "%s: Only works in blocking mode!\n", __func__);
		return -EBLOCKING_MODE_ONLY;
	}

	/* Send the message. The server will write the requested
	 * data directly to the specified data pointer. */
	err = OscIpcSendMsg(chanID, &msg);
	if(err != SUCCESS)
	{
		return err;
	}

	err = OscIpcSend(chanID, pData, paramSize);
	if(err != SUCCESS)
	{
		return err;
	}

	/* Wait for an acknowledge. As long as the server has not opened
	 * the other side yet, we will receive -ENO_MSG_AVAIL. */
	do
	{
		err = OscIpcRecvMsg(chanID, &msg);
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

OSC_ERR OscIpcGetRequest(const OSC_IPC_CHAN_ID chanID,
		struct OSC_IPC_REQUEST *pRequest)
{
	struct OSC_IPC_MSG msg;
	struct OSC_IPC_PARAM_MEMORY *pTempMem;
	OSC_ERR err = SUCCESS;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE) ||
			(pRequest == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x): Invalid parameter!\n",
				__func__, chanID, pRequest);
		return -EINVALID_PARAMETER;
	}
	
	err = OscIpcRecvMsg(chanID, &msg);
	if(err != SUCCESS)
	{
		/* Probably -ENO_MSG_AVAILABLE but may also be a
		 * real error. */
		return err;
	}

	/* msg.paramProp specifies the size of the data to read
	 * for the host. */
	pTempMem =
		malloc(msg.paramProp + sizeof(struct OSC_IPC_PARAM_MEMORY));

	if(pTempMem == NULL)
		return -EOUT_OF_MEMORY;

	/* Remember the length of the temporary memory area for
	 * OscIpcAckRequest. */
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
		err = OscIpcRecv(chanID, pRequest->pAddr, pTempMem->memLen);
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

OSC_ERR OscIpcAckRequest(const OSC_IPC_CHAN_ID chanID,
		const struct OSC_IPC_REQUEST *pRequest,
		const bool bSucceeded)
{
	struct OSC_IPC_MSG              msg;
	struct OSC_IPC_PARAM_MEMORY     *pMem;
	uint8                           *pTemp;
	OSC_ERR                         err;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE) ||
			(pRequest == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x, %d): Invalid parameter!\n",
				__func__, chanID, pRequest, bSucceeded);
		return -EINVALID_PARAMETER;
	}
		
	err = SUCCESS;
	/* Recover the struct OSC_IPC_PARAM_MEMORY pointer from the pointer
	 * to its member data. We need this to know the length of the
	 * parameter memory area to be able to send it. */
	pTemp = (uint8*)pRequest->pAddr;
	/* Shift the whole structure back by the length of the member before
	 * the actual data.
	 * This looks ugly but is save to do since the value of sizeof
	 * is determined at compile time. */
	pTemp -= sizeof(pMem->memLen);
	pMem = (struct OSC_IPC_PARAM_MEMORY*)pTemp;

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
	err = OscIpcSendMsg(chanID, &msg);
	if(err != SUCCESS)
	{
		if(err != -ETRY_AGAIN)
		{
			OscLog(ERROR, "%s: Failed to send acknowledge!\n", __func__);
		}
		goto exit;
	}

	/* If this was a read command we need to send back its result. */
	if(pRequest->enType == CMD_RD_PARAM)
	{
		err = OscIpcSend(chanID, &pMem->data, pMem->memLen);
		if(err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to send data.\n", __func__);
			goto exit;
		}
	}
	
exit:
	free(pMem);
	return err;
}
