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

/*! @file
 * @brief API definition for interprocess communication module
 * 
 * This connects two processes (a client and a server) with
 * a set of named pipes in the file system. Depending on the option
 * supplied when allocating a channel, communication can be blocking or
 * non-blocking.
 * 
 */
#ifndef IPC_PUB_H_
#define IPC_PUB_H_

extern struct OscModule OscModule_ipc;

/*! @brief Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscIpcErrors
{
	ESOCKET = OSC_IPC_ERROR_OFFSET,
	ENO_MSG_AVAIL,
	EBLOCKING_MODE_ONLY,
	ENEGATIVE_ACKNOWLEDGE
};

/*! @brief The types of an IPC request. */
enum EnRequestType
{
	REQ_TYPE_READ,
	REQ_TYPE_WRITE
};

/*! @brief Optional flags when opening an IPC channel*/
enum EnChannelFlags
{
	/*! @brief Acting as server on the IPC channel. */
	F_IPC_SERVER = 0x1,
	/*! @brief Channel is non-blocking.*/
	F_IPC_NONBLOCKING = 0x2
};

/*! @brief Represents an IPC request. */
struct OSC_IPC_REQUEST
{
	/*! @brief The type of this request. */
	enum EnRequestType enType;
	/*! @brief The parameter of this request. */
	uint32 paramID;
	/*! @brief The source/destination address in the address space of
	 *  the peer process. */
	void *pAddr;
};

/*! The data type for an IPC channel Identifier */
#define OSC_IPC_CHAN_ID uint8

/*=========================== API functions ============================*/

/*********************************************************************//*!
 * @brief Register an IPC channel for future message communication
 * 
 * Checks if a free IPC channel is available and reserves it. Every IPC
 * channel connects a client and a server. If the
 * channel is registered as a server, the named pipes for the input and
 * output direction are created in the file system. The other party of
 * course needs to register its connection with the names for the pipes
 * reversed.The server must always register the connection first,
 * or any client trying to connect will fail.  When
 * not setting the blocking option, operations will always return
 * immediately.
 * @see OscIpcUnregisterChannel
 * 
 * @param pIpcChan Pointer where the channel ID of the allocated IPC
 * channel is stored on success.
 * @param strSocketPath Path to the well-known socket this channel is
 * using. A server will bind this socket and wait for incoming packets
 * while a client will connect to it.
 * @param flags Options for this IPC channel (F_IPC_SERVER or
 * F_IPC_NONBLOCKING)
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcRegisterChannel(
		OSC_IPC_CHAN_ID * pIpcChan,
		const char *strSocketPath,
		const int flags);

/*********************************************************************//*!
 * @brief Unregister a previously allocated IPC channel.
 * @see OscIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be unregistered.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcUnregisterChannel(const OSC_IPC_CHAN_ID chanID);

/*********************************************************************//*!
 * @brief Read the value of a parameter from the server over IPC.
 * 
 * Can only be called on a blocking channel. The server and client need
 * to agree on identifiers for the data fields (parameters) they want to
 * exchange. Then can issue read and write requests of these parameters,
 * which the server then executes. This is the function to issue a read
 * request to the server. It will block until the data of the requested
 * data is written to pData.
 * 
 * Only to be called by the client side of an IPC channel.
 * 
 * @see OscIpcSetParam
 * @see OscIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pData Where to write the data read from the remote process.
 * @param paramID An identifier for the data field to be read agreed on
 * by both sides of the communication.
 * @param paramSize The length of above data field.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcGetParam(const OSC_IPC_CHAN_ID chanID,
		void *pData,
		const uint32 paramID,
		const uint32 paramSize);

/*********************************************************************//*!
 * @brief Read the value of a parameter from the server over IPC.
 * 
 * Can only be called on a blocking channel. The server and client need
 * to agree on identifiers for the data fields (parameters) they want to
 * exchange. Then can issue read and write requests of these parameters,
 * which the server then executes. This is the function to issue a write
 * request to the server. It will block until the data of the requested
 * data is written to pData.
 * 
 * Only to be called by the client side of an IPC channel.
 * 
 * @see OscIpcSetParam
 * @see OscIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pData Pointer to data to write to remote process.
 * @param paramID An identifier for the data field to be written agreed
 * on by both sides of the communication.
 * @param paramSize The length of above data field.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcSetParam(const OSC_IPC_CHAN_ID chanID,
		void *pData,
		const uint32 paramID,
		const uint32 paramSize);

/*********************************************************************//*!
 * @brief Get a new IPC request to handle.
 * 
 * See if there is a new IPC request to be handled. If yes the request
 * is returned in pRequest. Otherwise -ENO_MSG_AVAIL is returned.
 * All requests received in this way must be acknowledged by calling
 * OscIpcAckRequest.
 * 
 * Only to be called by the server side of an IPC channel.
 * 
 * @see OscIpcAckRequest
 * @see OscIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pRequest The returned request.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcGetRequest(const OSC_IPC_CHAN_ID chanID,
		struct OSC_IPC_REQUEST *pRequest);

/*********************************************************************//*!
 * @brief Acknowledge the execution of an IPC request.
 * 
 * Acknowledge a request previously received by OscIpcGetRequest
 * depending on the success of the execution.
 * 
 * Only to be called by the server side of an IPC channel.
 * 
 * @see OscIpcGetRequest
 * @see OscIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pRequest The request to be acknowledged.
 * @param bSucceeded True if the request was executed successfully.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
OSC_ERR OscIpcAckRequest(const OSC_IPC_CHAN_ID chanID,
		const struct OSC_IPC_REQUEST *pRequest,
		const bool bSucceeded);

#endif /*IPC_PUB_H_*/
