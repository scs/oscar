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

/*! @file ipc.h
 * @brief Private interprocess communication module definition
 * 
 */
#ifndef IPC_PRIV_H_
#define IPC_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "include/log.h"

/*! @brief The maximum number of IPC channels at any time. */
#define MAX_NR_IPC_CHANNELS 2
/*! @brief The number of incoming connection requests that get queued
 * until calling accept()*/
#define ACCEPT_WAIT_QUEUE_LEN 5
/*! @brief Path to the temporary sockets used by IPC clients. */
#define SOCKET_PATH "/tmp/OscIpc"
/*! @brief File permissions of the server socket file node. */
#define SERV_SOCKET_PERMISSIONS     \
	(S_IXUSR | S_IRUSR | S_IWUSR |  \
		S_IXGRP | S_IRGRP | S_IWGRP |  \
		S_IXOTH | S_IROTH | S_IWOTH)

/*! @brief Structure representing a full-duplex IPC channel. */
struct OSC_IPC_CHANNEL
{
	/*! @brief Socket to initiate communication with. */
	int     sock;
	/*! @brief The file name of the input socket */
	char    strSocketPath[256];
	/*! @brief The flags used when opening that channel. */
	uint32  flags;
	/*! @brief Socket returned by accept() and used for communication
	 * on the server side. */
	int     acceptedSock;
};

/*! @brief The different commands used in IPC messages. */
enum EnIpcCmds
{
	CMD_RD_PARAM,
	CMD_WR_PARAM,
	CMD_RD_PARAM_ACK,
	CMD_WR_PARAM_ACK,
	CMD_RD_PARAM_NACK,
	CMD_WR_PARAM_NACK
};

/*! @brief An interprocess communication message.
 * Represents an IPC message for communication between client and
 * server process on the same machine. This is held quite general
 * for maximum flexibility. */
struct OSC_IPC_MSG
{
	/*! @brief The type of action to be taken */
	enum EnIpcCmds enCmd;
	/*! @brief A parameter identifier to above cmd. */
	uint32 paramID;
	/*! @brief Additional property field of the message
	 * (architecture dependent).
	 * 
	 * Target: A pointer to above parameter.
	 * Host: The size of the parameter. */
	uint32 paramProp;
};

#if defined(OSC_HOST) || defined(OSC_SIM)
/*! @brief Used as a variable length memory area which remembers its own
 * length.
 * 
 * This structure can be cast on a pre-allocated memory area of variable
 * size. The purpose is to be able to store the length of the memory
 * area in the same place as the memory itself. The 'data' member acts
 * as an anker to the start of the data field. (&data).*/
struct OSC_IPC_PARAM_MEMORY
{
	/*! @brief The length of the following data field. */
	uint32 memLen;
	/*! @brief The pointer to this member equals the pointer to the
	 * start of the data area. */
	char data;
};

#endif /* OSC_HOST */

/*! @brief The object struct of the camera module */
struct OSC_IPC
{
	struct OSC_IPC_CHANNEL  aryIpcChans[MAX_NR_IPC_CHANNELS];
	bool                    arybIpcChansBusy[MAX_NR_IPC_CHANNELS];
};

/*********************************************************************//*!
 * @brief Receive an incoming data packet over IPC.
 * 
 * Depending on the parameters specified when registering the associated
 * channel this call is blocking or non-blocking. If it is non-blocking
 * and there currently is no message pending, -ENO_MSG_AVAIL is returned.
 * It only reads as much data as specified in the arguments, so it must
 * only be used when the amount of data expected is known.
 * 
 * @param chanID Channel ID of the channel to receive from.
 * @param pData Where to store incoming data.
 * @param dataLen The length of the expected data.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcRecv(const OSC_IPC_CHAN_ID chanID,
		void *pData,
		const uint32 dataLen);

/*********************************************************************//*!
 * @brief Send a data packet over IPC.
 * 
 * As long as there is a reader on the associated named pipe, this
 * call is always non-blocking. Otherwise, the call either blocks and
 * waits for a reader to connect or returns with an error code, depending
 * on the bBlocking option when opening the channel.
 * 
 * @param chanID Channel ID of the channel to send on.
 * @param pData Pointer to data to be sent.
 * @param dataLen The length of the expected data.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcSend(const OSC_IPC_CHAN_ID chanID,
		const void *pData,
		const uint32 dataLen);

/*********************************************************************//*!
 * @brief Receive an incoming IPC message.
 * 
 * Depending on the parameters specified when registering the associated
 * channel this call is blocking or non-blocking. If it is non-blocking
 * and there currently is no message pending, -ENO_MSG_AVAIL is returned.
 * Wrapper for OscIpcRecv.
 * @see OscIpcRecv
 * 
 * @param chanID Channel ID of the channel to receive from.
 * @param pMsg Where to store an incoming message.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcRecvMsg(const OSC_IPC_CHAN_ID chanID,
		struct OSC_IPC_MSG *pMsg);

/*********************************************************************//*!
 * @brief Send an IPC message.
 * 
 * As long as there is a reader on the associated named pipe, this
 * call is always non-blocking. Otherwise, the call either blocks and
 * waits for a reader to connect or returns with an error code, depending
 * on the bBlocking option when opening the channel.
 * Wrapper for OscIpcSend.
 * @see OscIpcSend
 * 
 * @param chanID Channel ID of the channel to send on.
 * @param pMsg Message to be sent
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcSendMsg(const OSC_IPC_CHAN_ID chanID,
		const struct OSC_IPC_MSG *pMsg);

#endif /*IPC_PRIV_H_*/
