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

/*! @file ipc_target.c
 * @brief Interprocess communication module implementation for host
 * 
 */

# include "oscar_types_target.h"

#include "ipc_pub.h"
#include "ipc_priv.h"
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
	msg.paramProp = (uint32)pData;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE)||
			(pData == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
				__func__, pData, paramID, paramSize);
		return -EINVALID_PARAMETER;
	}

	/* This function only works in blocking mode. */
	if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
	{
		OscLog(ERROR, "%s: Only works in non-blocking mode!\n", __func__);
		return -EBLOCKING_MODE_ONLY;
	}

	/* Send the message. The server will write the requested
	 * data directly to the specified data pointer. */
	err = OscIpcSendMsg(chanID, &msg);
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
			OscLog(ERROR, "%s: Ack did not match to request issued!\n");
			return -EDEVICE;
		}
	}
	else if(likely(msg.enCmd == CMD_RD_PARAM_NACK))
	{
		if(likely(msg.paramID == paramID))
		{
			return -ENEGATIVE_ACKNOWLEDGE;
		} else {
			OscLog(ERROR, "%s: Ack did not match to request issued!\n");
			return -EDEVICE;
		}
	}
	else
	{
		/* We got the wrong message, this must not happen. */
		OscLog(ERROR, "%s: Received no ack!\n", __func__);
		return -EDEVICE;
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
	msg.paramProp = (uint32)pData;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE) ||
			(pData == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x, %u, %u): Invalid parameter!\n",
				__func__, pData, paramID, paramSize);
		return -EINVALID_PARAMETER;
	}

	/* This function only works in blocking mode. */
	if(unlikely(ipc.aryIpcChans[chanID].flags & F_IPC_NONBLOCKING))
	{
		OscLog(ERROR, "%s: Only works in non-blocking mode!\n",
				__func__);
		return -EBLOCKING_MODE_ONLY;
	}
	
	/* Send the message. The server will write the requested
	 * data directly to the specified data pointer. */
	err = OscIpcSendMsg(chanID, &msg);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error sending request! (%d)\n",
				__func__, err);
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
		OscLog(ERROR, "%s: Error receiving acknowledge! (%d)\n",
				__func__, err);
		return err;
	}
	
	if(likely(msg.enCmd == CMD_WR_PARAM_ACK))
	{
		if(likely(msg.paramID == paramID))
		{
			return SUCCESS;
		} else {
			OscLog(ERROR, "%s: Ack did not match to request issued!\n");
			return -EDEVICE;
		}
	}
	else if(likely(msg.enCmd == CMD_WR_PARAM_NACK))
	{
		if(likely(msg.paramID == paramID))
		{
			return -ENEGATIVE_ACKNOWLEDGE;
		} else {
			OscLog(ERROR, "%s: Ack did not match to request issued!\n");
			return -EDEVICE;
		}
	}
	else
	{
		/* We got the wrong message, this must not happen. */
		OscLog(ERROR, "%s: Received no ack!\n", __func__);
		return -EDEVICE;
	}

	return SUCCESS;
}


OSC_ERR OscIpcGetRequest(const OSC_IPC_CHAN_ID chanID,
		struct OSC_IPC_REQUEST *pRequest)
{
	struct OSC_IPC_MSG msg;
	OSC_ERR err;

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

OSC_ERR OscIpcAckRequest(const OSC_IPC_CHAN_ID chanID,
		const struct OSC_IPC_REQUEST *pRequest,
		const bool bSucceeded)
{
	struct OSC_IPC_MSG msg;
	OSC_ERR err;

	/* Input validation */
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) ||
			(ipc.arybIpcChansBusy[chanID] == FALSE) ||
			(pRequest == NULL)))
	{
		OscLog(ERROR, "%s(%d, 0x%x, %d): Invalid parameter!\n",
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
	err = OscIpcSendMsg(chanID, &msg);
	if(err != SUCCESS)
	{
		if(err != -ETRY_AGAIN)
		{
			OscLog(ERROR, "%s: Failed to send acknowledge! (%d)\n",
					__func__, err);
		}
		return err;
	}

	return SUCCESS;
}
