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
	err = LCVIpcRegisterChannel(&chan, "/tmp/SHTSocket.sock", 0);
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
