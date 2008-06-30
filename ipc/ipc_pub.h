/*! @file ipc_pub.h
 * @brief API definition for interprocess communication module
 * 
 * This connects two processes (a client and a server) with
 * a set of named pipes in the file system. Depending on the option
 * supplied when allocating a channel, communication can be blocking or
 * non-blocking.
 * 
 * @author Markus Berner
 */
#ifndef IPC_PUB_H_
#define IPC_PUB_H_

#include "framework_error.h"
#ifdef LCV_HOST
    #include "framework_types_host.h"
    #include "framework_host.h"
#else
    #include "framework_types_target.h"
    #include "framework_target.h"
#endif /* LCV_HOST */

/*! @brief Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvIpcErrors 
{
    ESOCKET = LCV_IPC_ERROR_OFFSET,
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
struct LCV_IPC_REQUEST 
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
#define LCV_IPC_CHAN_ID uint8

/*=========================== API functions ============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVIpcCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVIpcDestroy(void *hFw);

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
 * @see LCVIpcUnregisterChannel
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
LCV_ERR LCVIpcRegisterChannel(
		LCV_IPC_CHAN_ID * pIpcChan,
        const char *strSocketPath,
        const int flags);

/*********************************************************************//*!
 * @brief Unregister a previously allocated IPC channel.
 * @see LCVIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be unregistered.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
LCV_ERR LCVIpcUnregisterChannel(const LCV_IPC_CHAN_ID chanID);

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
 * @see LCVIpcSetParam
 * @see LCVIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pData Where to write the data read from the remote process.
 * @param paramID An identifier for the data field to be read agreed on
 * by both sides of the communication.
 * @param paramSize The length of above data field.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
LCV_ERR LCVIpcGetParam(const LCV_IPC_CHAN_ID chanID,
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
 * @see LCVIpcSetParam
 * @see LCVIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pData Pointer to data to write to remote process.
 * @param paramID An identifier for the data field to be written agreed 
 * on by both sides of the communication.
 * @param paramSize The length of above data field.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
LCV_ERR LCVIpcSetParam(const LCV_IPC_CHAN_ID chanID,
        void *pData, 
        const uint32 paramID, 
        const uint32 paramSize);

/*********************************************************************//*!
 * @brief Get a new IPC request to handle.
 * 
 * See if there is a new IPC request to be handled. If yes the request
 * is returned in pRequest. Otherwise -ENO_MSG_AVAIL is returned.
 * All requests received in this way must be acknowledged by calling
 * LCVIpcAckRequest.
 * 
 * Only to be called by the server side of an IPC channel.
 * 
 * @see LCVIpcAckRequest
 * @see LCVIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pRequest The returned request.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
LCV_ERR LCVIpcGetRequest(const LCV_IPC_CHAN_ID chanID,
        struct LCV_IPC_REQUEST *pRequest);

/*********************************************************************//*!
 * @brief Acknowledge the execution of an IPC request.
 * 
 * Acknowledge a request previously received by LCVIpcGetRequest
 * depending on the success of the execution.
 * 
 * Only to be called by the server side of an IPC channel.
 * 
 * @see LCVIpcGetRequest
 * @see LCVIpcRegisterChannel
 * 
 * @param chanID Channel ID of the channel to be used.
 * @param pRequest The request to be acknowledged.
 * @param bSucceeded True if the request was executed successfully.
 * @return SUCCESS on success or an appropriate error code otherwise.
 *//*********************************************************************/
LCV_ERR LCVIpcAckRequest(const LCV_IPC_CHAN_ID chanID,
        const struct LCV_IPC_REQUEST *pRequest,
        const bool bSucceeded);

#endif /*IPC_PUB_H_*/
