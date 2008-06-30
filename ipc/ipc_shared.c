/*! @file ipc_shared.c
 * * @brief Interprocess communication module implementation shared 
 * part for host and target 
 * 
 * @author Markus Berner
 */

#ifdef LCV_HOST
# include "framework_types_host.h"
#else /* LCV_HOST */
# include "framework_types_target.h"
#endif /* LCV_HOST */

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "ipc_pub.h"
#include "ipc_priv.h"
#include "framework_intern.h"

/*! The camera module singelton instance. */
struct LCV_IPC ipc;     

/*! The dependencies of this module. */
struct LCV_DEPENDENCY ipc_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy}
};


LCV_ERR LCVIpcCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->ipc.useCnt != 0)
    {
        pFw->ipc.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }

    /* Load the module ipc_deps of this module. */
    err = LCVLoadDependencies(pFw, 
            ipc_deps, 
            sizeof(ipc_deps)/sizeof(struct LCV_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load ipc_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }

    memset(&ipc, 0, sizeof(struct LCV_IPC));

    /* Increment the use count */
    pFw->ipc.hHandle = (void*)&ipc;
    pFw->ipc.useCnt++; 

    return SUCCESS;
}

void LCVIpcDestroy(void *hFw)
{
    LCV_IPC_CHAN_ID         i;
    struct LCV_FRAMEWORK    *pFw;

    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->ipc.useCnt--;
    if(pFw->ipc.useCnt > 0)
    {
        return;
    }

    /* Close all sockets associated with open channels */
    for(i = 0; i < MAX_NR_IPC_CHANNELS; i++)
    {
        if(ipc.arybIpcChansBusy[i] == TRUE)
        {
            if(ipc.aryIpcChans[i].sock > 0)
            {
                LCVIpcUnregisterChannel(ipc.aryIpcChans[i].sock);
            }
        }
    }

    LCVUnloadDependencies(pFw, 
            ipc_deps, 
            sizeof(ipc_deps)/sizeof(struct LCV_DEPENDENCY));    

    memset(&ipc, 0, sizeof(struct LCV_IPC));
}

static inline void UniqueClientSocketName(char *strSocketPath)
{
	sprintf(strSocketPath,
			SOCKET_PATH "%d.sock",
			getpid());
}

LCV_ERR LCVIpcRegisterChannel(
		LCV_IPC_CHAN_ID * pIpcChan,
        const char *strSocketPath,
        const int flags)
{
    unsigned int 		sock;
    struct sockaddr_un 	addr;
    int 				len, ret, chan;

    if(unlikely((pIpcChan == NULL) ||
    		(strSocketPath == NULL) || (strSocketPath[0] == '\0')))
    {
    	LCVLog(ERROR, "%s(0x%x, 0x%x): Invalid parameter!\n",
    			__func__, pIpcChan, strSocketPath);
    	return -EINVALID_PARAMETER;
    }
    
    /* Find a free IPC channel */
    for(chan = 0; chan < MAX_NR_IPC_CHANNELS; chan++)
    {
    	if(ipc.arybIpcChansBusy[chan] == FALSE)
    	{
    		break;
    	}
    }
    if(unlikely(chan == MAX_NR_IPC_CHANNELS))
    {
    	LCVLog(ERROR, "%s: All IPC channels busy!\n", __func__);
    	return -EDEVICE_BUSY;
    }
        
    /* Create the socket. */
    ipc.aryIpcChans[chan].sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sock < 0)
    {
    	LCVLog(ERROR, "%s: Error allocating socket! (%s)\n", 
    			__func__,
    			strerror(errno));
    	return -ESOCKET;
    }

    if(flags & F_IPC_NONBLOCKING)
    {
	    /* Make the file descriptor non-blocking so future operations do
	     * not block. */
	    ret = fcntl(ipc.aryIpcChans[chan].sock,
	    		F_SETFL,
	    		O_NONBLOCK);
	    if(ret < 0)
	    {
	    	LCVLog(ERROR, "%s: Unable to make socket non-blocking! (%s)\n",
	    			__func__,
	    			strerror(errno));
	    	return -ESOCKET;
	    }
    }
   
    ipc.aryIpcChans[chan].flags = flags;
    strcpy(ipc.aryIpcChans[chan].strSocketPath, strSocketPath);
    
    if(flags & F_IPC_SERVER)
    {
    	/* Open the IPC channel as a server. This means just binding
    	 * to the specified socket name and waiting for incoming 
    	 * requests. */
        
        addr.sun_family = AF_UNIX;  
        strcpy(addr.sun_path, ipc.aryIpcChans[chan].strSocketPath);
        len = strlen(addr.sun_path) + sizeof(addr.sun_family);
        
        /* Remove the socket file before bind() if already present */
        unlink(addr.sun_path);
        
    	ret = bind(ipc.aryIpcChans[chan].sock, 
    			(struct sockaddr *)&addr, 
    			len);
    	if(ret < 0)
    	{
    		LCVLog(ERROR, "%s: Unable to bind socket! (%s)\n",
    				__func__, 
    				strerror(errno));
    		close(ipc.aryIpcChans[chan].sock);
    		return -ESOCKET;
    	}
    	
    	/* Make the socket file node accessible by all processes. */
    	ret = chmod(addr.sun_path, SERV_SOCKET_PERMISSIONS);
    	if(ret < 0)
    	{
    		LCVLog(WARN, "%s: Unable to set access permissions of "
    				"socket file node \"%s\"! (%s)",
    				__func__,
    				addr.sun_path,
    				strerror(errno));
    	}
    	
    	/* Start listening on the socket. */
    	ret = listen(ipc.aryIpcChans[chan].sock, ACCEPT_WAIT_QUEUE_LEN);
    	if(ret < 0)
    	{
    		LCVLog(ERROR, "%s: Unable to listen to socket! (%s)\n",
    				__func__, 
    				strerror(errno));
    		close(ipc.aryIpcChans[chan].sock);
    		unlink(ipc.aryIpcChans[chan].strSocketPath);
    		return -ESOCKET;
    	}
    } else {
    	/* Open the IPC channel as client. Connect to the specified 
    	 * server socket. */
    	
	    	
    	/* Connect to remote socket. */
    	addr.sun_family = AF_UNIX;
    	strcpy(addr.sun_path, strSocketPath);
    	len = strlen(addr.sun_path) + sizeof(addr.sun_family);
    	ret = connect(ipc.aryIpcChans[chan].sock,
    			(struct sockaddr *)&addr,
    			len);
    	if(ret < 0)
    	{
    		if(errno == EINPROGRESS)
    		{
    			/* Nonblocking connect. */
    			LCVLog(WARN, "%s: Socket not fully connected yet "
    					"due to non-blocking operation...",
    					__func__);
    		} else {
    			LCVLog(ERROR, "%s: Unable to connect socket! (%s)\n",
    					__func__,
    					strerror(errno));
    			close(ipc.aryIpcChans[chan].sock);
    			unlink(ipc.aryIpcChans[chan].strSocketPath);
    			return -ESOCKET;
    		}
    	}
    }
    
    *pIpcChan = chan;
    ipc.arybIpcChansBusy[chan] = TRUE;
    return SUCCESS;
}

LCV_ERR LCVIpcUnregisterChannel(LCV_IPC_CHAN_ID chanID)
{
	struct LCV_IPC_CHANNEL 	*pChan;
	
	pChan = &ipc.aryIpcChans[chanID];
	
	if(unlikely((chanID >= MAX_NR_IPC_CHANNELS) || 
			(ipc.arybIpcChansBusy[chanID] == FALSE)))
	{
		return -EINVALID_PARAMETER;
	}
	
	if(pChan->sock > 0)
	{
		close(pChan->sock);
	}
	
	/* Delete the file node associated with this channel's socket. */
	if(pChan->flags & F_IPC_SERVER)
	{
		unlink(ipc.aryIpcChans[chanID].strSocketPath);
	}
	ipc.arybIpcChansBusy[chanID] = FALSE;
	return SUCCESS;
}

inline LCV_ERR LCVIpcRecvMsg(const LCV_IPC_CHAN_ID chanID, 
        struct LCV_IPC_MSG *pMsg)
{
    return LCVIpcRecv(chanID, pMsg, sizeof(struct LCV_IPC_MSG));
}

LCV_ERR LCVIpcRecv(const LCV_IPC_CHAN_ID chanID, 
        void *pData,
        const uint32 dataLen)
{
	int 					ret;
	struct LCV_IPC_CHANNEL 	*pChan;
	struct sockaddr_un		remoteAddr;
	socklen_t				remoteAddrLen;
	int						sock;
	
	/* No input validation since this is only called by module-internal
	 * functions. */
	
	pChan = &ipc.aryIpcChans[chanID];
	
	if(pChan->flags & F_IPC_SERVER)
	{
		if(pChan->acceptedSock <= 0)
		{
			/* No connected accepted yet. */
			remoteAddrLen = sizeof(remoteAddr);
			pChan->acceptedSock = accept(pChan->sock,
											(struct sockaddr*)&remoteAddr,
											&remoteAddrLen);
			if(pChan->acceptedSock < 0)
			{
				if(likely((errno == EAGAIN) || (errno == EWOULDBLOCK)))
				{
					/* No connection request pending.*/
					return -ENO_MSG_AVAIL;
				} else {
					LCVLog(ERROR, "%s: Accepting connection failed! (%s)\n",
							__func__, strerror(errno));
					return -ESOCKET;
				}
			}
			
			if(pChan->flags & F_IPC_NONBLOCKING)
			{
				/* Make the file descriptor non-blocking so receive and 
				 * send commands do not block. */
				ret = fcntl(pChan->acceptedSock,
						F_SETFL,
						O_NONBLOCK);
				if(ret < 0)
				{
					LCVLog(ERROR, 
							"%s: Unable to make socket non-blocking! (%s)\n",
							__func__,
							strerror(errno));
					return -ESOCKET;
				}
			}
		}
		sock = pChan->acceptedSock;
	} else {
		sock = pChan->sock;
	}
	
	if(pChan->flags & F_IPC_NONBLOCKING)
	{
	    /* If non-blocking, first peek at the data to see if a full
	     * message has arrived. */
	    errno = 0;
	    ret = recv(sock,
	            pData,
	            dataLen,
	            MSG_PEEK);

	    if(ret != dataLen)
	    {
	        if(likely(errno == EAGAIN || errno == EWOULDBLOCK))
	        {
	            /* No messages waiting */
	            return -ENO_MSG_AVAIL;
	        } else if(errno == 0 && ret == 0) { /* EOF */
	            /* Remote end of socket shut down. Mark it as invalid, so
	             * the next call will try to accept a new connection. */
	            close(pChan->acceptedSock);
	            pChan->acceptedSock = -1;			
	            return -ENO_MSG_AVAIL;
	        } else {
	            LCVLog(ERROR, "%s: Reading pending messages failed! (%s)\n",
	                    __func__, strerror(errno));
	            return -ESOCKET;
	        }			
	    }
	}
	
	errno = 0;
	ret = recv(sock,
	        pData,
	        dataLen,
	        MSG_WAITALL);
	
	if(ret != dataLen)
	{
	    if(likely(errno == EAGAIN || errno == EWOULDBLOCK))
	    {
	        /* No messages waiting */
	        return -ENO_MSG_AVAIL;
	    } else if(errno == 0 && ret == 0) { /* EOF */
	        /* Remote end of socket shut down. Mark it as invalid, so
	         * the next call will try to accept a new connection. */
	        close(pChan->acceptedSock);
	        pChan->acceptedSock = -1;           
	        return -ENO_MSG_AVAIL;
	    } else {
	        LCVLog(ERROR, "%s: Reading pending messages failed! (%s)\n",
	                __func__, strerror(errno));
	        return -ESOCKET;
	    }           
	}
	return SUCCESS;
}

inline LCV_ERR LCVIpcSendMsg(const LCV_IPC_CHAN_ID chanID, 
        const struct LCV_IPC_MSG *pMsg)
{
    return LCVIpcSend(chanID, (void*)pMsg, sizeof(struct LCV_IPC_MSG));
}

LCV_ERR LCVIpcSend(const LCV_IPC_CHAN_ID chanID, 
        const void *pData,
        uint32 dataLen)
{
	int 					ret = 0;
	struct LCV_IPC_CHANNEL 	*pChan;
	int 					sock;
	
	pChan = &ipc.aryIpcChans[chanID];
	
	if(pChan->flags & F_IPC_SERVER)
	{
		sock = pChan->acceptedSock;
	} else {
		sock = pChan->sock;
	}
	/* No input validation since this is only called by module-internal
	 * functions. */
	while ( dataLen != 0 )
	{ 
	    ret = send(sock, pData, dataLen, 0);
	    if (unlikely(ret == -1))
	    {
	        if ( errno == EAGAIN )
	        {
	            usleep(5000);
	        } else
	            break;
	    }
	    else
	    {
	        dataLen -= ret;
	        pData = (char*)pData + ret;
	    }
	}
	
	if(unlikely(ret == -1))
	{

		LCVLog(ERROR, "%s: Sending to remote process failed! (%s)\n",
				__func__, strerror(errno));
		return -ESOCKET;
	}
	
	return SUCCESS;
}

