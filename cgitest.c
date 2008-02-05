#include "inc/framework.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sched.h>
#include <errno.h>

/* This implements the client side of the IPC. The server side is implemented
   in the 'test' executable, which needs to be started before this. */
LCV_ERR testIpc()
{
	LCV_IPC_CHAN_ID chan;
	LCV_ERR err;
	uint32 readParam;
	uint32 writeParam = 0xbeefdead;


	/* Open channel as blocking and as client */
	err = LCVIpcRegisterChannel(&chan, "LCVResponse.fifo", "LCVRequest.fifo", TRUE, FALSE);
	LCVLog(DEBUG, "IpcRegisterChannel: %d => %d\n", err, chan);
	if(err)
	{
		LCVLog(ERROR, "Unable to register channel!\n");
		return err;
	}

	LCVLog(DEBUG, "Channel %d registered!\n", chan);

	err = LCVIpcGetParam(chan, &readParam, 0, sizeof(readParam));
	if(err)
	{
		LCVLog(ERROR, "Unable to read parameter.\n");
		return -1;
	} else {
		LCVLog(DEBUG, "Read 0x%x!\n", readParam);
	}

	err = LCVIpcSetParam(chan, &writeParam, 1, sizeof(writeParam));
	if(err)
	{
		LCVLog(ERROR, "Unable to write parameter.\n");
		return -1;
	}

	return 0;
}

int main()
{
	void * hFramework;
	LCV_ERR err;

	err = LCVCreate(&hFramework);
	if(err < 0) 
	{
		printf("Unable to create framework.\n");
		return -1;
	}
      
	err = LCVLogCreate(hFramework);

	err = LCVIpcCreate(hFramework);
	if(err < 0)
	{
		LCVLog(ERROR, "Unable to create IPC module\n");
	}

	err = testIpc();
	if(err < 0)
		LCVLog(ERROR, "%s: Ipc test failed! (%d)\n", __func__, err);

	LCVIpcDestroy(hFramework);
	LCVLogDestroy(hFramework);
	LCVDestroy(hFramework);
	return 0;
}
