#include "inc/framework.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>

void * hFramework;

int testLog()
{
	/* Test without log setting a log level first */
	LCVLog(DEBUG, "This is a debug message.\n");
	LCVLog(INFO, "This is an info message.\n");
	LCVLog(WARN, "This is a warning message.\n");
	LCVLog(ERROR, "This is an error message.\n");
	LCVLog(CRITICAL, "This is a critical error message.\n");

	LCVLog(SIMULATION, "This is a simulation log message.\n");

	/* Set a log level and monitor the difference */
	LCVLogSetConsoleLogLevel(INFO);
	LCVLogSetFileLogLevel(ERROR);

	LCVLog(DEBUG, "This is a debug message.\n");
	LCVLog(INFO, "This is an info message.\n");
	LCVLog(WARN, "This is a warning message.\n");
	LCVLog(ERROR, "This is an error message.\n");
	LCVLog(CRITICAL, "This is a critical error message.\n");

	LCVLog(SIMULATION, "This is a simulation log message.\n");

	LCVLog(INFO, "-------------------------------------------------------\n");

	LCVLogSetConsoleLogLevel(DEBUG);
	LCVLogSetFileLogLevel(DEBUG);

	return 0;
}

int testLgx()
{
	LCV_ERR err;
	uint16 clock = FALSE;

	if((err = LCVLgxCreate(hFramework)))
                return -1;
       
        LCVLogSetConsoleLogLevel(DEBUG);
        LCVLogSetFileLogLevel(DEBUG);

        LCVLgxIoPinWrite( LCV_LGX_OUT1, TRUE);
	LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);
        LCVSimStep();
        LCVLgxIoPinWrite( LCV_LGX_OUT1, FALSE);
        LCVLgxIoPinWrite( LCV_LGX_OUT3, TRUE);
        LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);
        LCVSimStep();

        LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);

        LCVSimStep();

        LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);

        LCVSimStep();

        LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);

        LCVSimStep();

        LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);


        LCVSimStep();

        LCVLgxIoPinRead( LCV_LGX_IN1, &clock);
        printf("t=%d, CLOCK=%d\n", (int)LCVSimGetCurTimeStep(), clock);

        LCVLog(INFO, "-------------------------------------------------------\n");
	LCVLgxDestroy(hFramework);
        return 0;
}

int testSwr()
{
#ifdef LCV_HOST
	LCV_ERR err;
	void* pWr;
	void* pSig1;
	void* pSig2;
	float sig1;
	char sig2[20];

        if((err = LCVSwrCreate(hFramework)))
                return -1;

	LCVSwrCreateWriter( 
        	&pWr,        
	        "swr.txt", 
        	FALSE, /* bReportTime */
	        FALSE ); /* bReportCyclic */

	LCVSwrRegisterSignal( 
	        &pSig1,        
	        pWr, 
	        "Sig1",
	        SWR_FLOAT,        
	        NULL, /* default value */
	        "%1.3f" );

        LCVSwrRegisterSignal(        
                &pSig2,
                pWr,
                "Sig2",
                SWR_STRING,
                NULL, /* default value */
                "%s" );


        sig1 = 1.23456;
        strcpy( sig2, "Some Text");
	LCVSwrUpdateSignal( pSig1, &sig1);
	LCVSwrUpdateSignal( pSig2, sig2);
	LCVSwrManualReport( pWr);

        sig1 = 9.8765;
        strcpy( sig2, "Other Text");
        LCVSwrUpdateSignal( pSig1, &sig1);
        LCVSwrUpdateSignal( pSig2, sig2);
        LCVSwrManualReport( pWr);
        
	LCVSwrManualReport( pWr);

#endif /*LCV_HOST*/
	return 0;
}

int testCamRegs()
{
	uint16 maxShutterWidth;
	uint32 shutterWidth;
	LCV_ERR err;

	if((err = LCVCamCreate(hFramework)))
		return -1;

	/* General register manipulation */
	err = LCVCamGetRegisterValue(0xbd, &maxShutterWidth);
	if(err < 0) {
		LCVLog(ERROR, "Unable to manually get max shutter width. (%d)\n", err);
		return err;
	} else {
		LCVLog(INFO, "Max shutter width: %d.\n", maxShutterWidth);
	}

	maxShutterWidth++;
	err = LCVCamSetRegisterValue(0xbd, maxShutterWidth);
	if(err < 0) {
		LCVLog(ERROR, "Unable to manually set max shutter width. (%d)\n", err);
		return err;
	} else {
		LCVLog(INFO, "Set max shutter width: %d.\n", maxShutterWidth);
	}

	err = LCVCamGetRegisterValue(0xbd, &maxShutterWidth);
	if(err < 0) {
		LCVLog(ERROR, "Unable to manually get max shutter width. (%d)\n", err);
		return err;
	} else {
		LCVLog(INFO, "Max shutter width: %d.\n", maxShutterWidth);
	}
	
	/* Specific register manipulation */
	err = LCVCamGetShutterWidth(&shutterWidth);
	if(err < 0) {
		LCVLog(ERROR, "Unable to get shutter width.(%d)\n", err);
		return err;
	} else {
		LCVLog(INFO, "Shutter width: %d.\n", shutterWidth);
	}

	shutterWidth += 1000; // 1 ms
	err = LCVCamSetShutterWidth(shutterWidth);
	if(err < 0) {
		LCVLog(ERROR, "Unable to set shutter width. (%d)\n", err);
		return err;
	} else {
		LCVLog(INFO, "Set shutter width: %d.\n", shutterWidth);
	}

	err = LCVCamGetShutterWidth(&shutterWidth);
	if(err < 0) {
		LCVLog(ERROR, "Unable to get shutter width.(%d)\n", err);
		return err;
	} else {
		LCVLog(INFO, "Shutter width: %d.\n", shutterWidth);
	}

	LCVCamDestroy(hFramework);
	LCVLog(INFO, "-------------------------------------------------------\n");
	return 0;
}

int testBMP()
{
	struct LCV_PICTURE pic1;
	struct LCV_PICTURE pic2;
	struct LCV_PICTURE pic3;
	LCV_ERR err;

	uint32 imageSize;

	if((err = LCVBmpCreate(hFramework)))
		return -1;

	memset(&pic1, 0, sizeof(struct LCV_PICTURE));
	memset(&pic2, 0, sizeof(struct LCV_PICTURE));
	memset(&pic3, 0, sizeof(struct LCV_PICTURE));

	/************ Loading Greyscale *******************/
	/* No pre-allocated memory, no assumptions */
	LCVBmpRead(&pic1, "test_grey_in.bmp");
	LCVLog(INFO, "%s: ## Pic1 loaded.(%dx%d|type %d)\n", __FUNCTION__, 
	       pic1.width, pic1.height, pic1.type);

	/* No pre-allocated memory, assumptions */
	pic2.width = pic1.width;
	pic2.height = pic1.height;
	pic2.type = pic1.type;
	LCVBmpRead(&pic2, "test_grey_in.bmp");
	LCVLog(INFO, "%s: ## Pic2 loaded.\n", __FUNCTION__);

	/* Pre-Allocated memory, assumptions */
	pic3.width = pic1.width;
	pic3.height = pic1.height;
	pic3.type = pic1.type;
	pic3.data = malloc(pic1.width * pic1.height * 1);
	LCVBmpRead(&pic3, "test_grey_in.bmp");
	LCVLog(INFO, "%s: ## Pic3 loaded.\n", __FUNCTION__);

	imageSize = pic1.width*pic1.height;
	if(memcmp(pic1.data, pic2.data, imageSize) ||
	   memcmp(pic1.data, pic3.data, imageSize))
	{
		LCVLog(ERROR, "%s: Greyscale pics differ.\n",
			__FUNCTION__);
		return -1;
	}

	/************ Writing Greyscale *******************/
	LCVBmpWrite(&pic1, "test_grey_out1.bmp");
	LCVLog(INFO, "%s: ## Pic1 written.\n", __FUNCTION__);

	free(pic1.data);
	free(pic2.data);
	free(pic3.data);
	memset(&pic1, 0, sizeof(struct LCV_PICTURE));

	/***************** Loading RGB ********************/
	LCVBmpRead(&pic1, "test_rgb_in.bmp");
	LCVLog(INFO, "%s: ## Pic1 RGB loaded.(%dx%d|type %d)\n", __FUNCTION__, 
	       pic1.width, pic1.height, pic1.type);

	/***************** Writing RGB ********************/
	LCVBmpWrite(&pic1, "test_rgb_out1.bmp");
	LCVLog(INFO, "%s: ## Pic1 RGB written.\n", __FUNCTION__);

	free(pic1.data);
	LCVLog(INFO, "-------------------------------------------------------\n");

	LCVBmpDestroy(hFramework);
	return 0;
}

#define FB_SIZE 752*480*1
uint8 fbs[8][FB_SIZE];

int testCam()
{
	uint8 multibufferIDs[4] = {4, 5, 6, 7};
	struct LCV_PICTURE pic;
	LCV_ERR  err;
	void *pPic;
	int i;

	if((err = LCVCamCreate(hFramework)))
		return -1;

	/* The camera trigger is floating when specifying BF537-LCV-IND as the
	   target platform. Therefore this will only work with BF537-LCV and
	   the manual trigger or by adjusting mt9v032.c */
	LCVLog(DEBUG, "Setting up frame buffers.\n");
	for(i=0; i < 8; i++)
	{
		LCVCamSetFrameBuffer((uint8)i, 
				     (uint32)FB_SIZE, 
				     (void*)fbs[i], 
				     TRUE);
	}

	LCVLog(DEBUG, "Setting area of interest.\n");
	LCVCamSetAreaOfInterest(0, 0, 752, 130);

	LCVLog(DEBUG, "Capturing picture.\n");
	if((err = LCVCamSetupCapture(0, LCV_CAM_TRIGGER_MODE_MANUAL)) < 0)
		LCVLog(ERROR, "%s: Err %d\n", __func__, err);

	LCVLog(DEBUG, "Syncing picture without age/timeout.\n");
	if((err = LCVCamReadPicture(0, &pPic, 0, 0)) < 0)
		LCVLog(ERROR, "%s: Err %d\n", __func__, err);
	

	pic.data = pPic;
	pic.width = 752;
	pic.height = 130;
	pic.type = LCV_PICTURE_GREYSCALE;
	LCVBmpWrite(&pic, "Cap752x130.bmp");

//	usleep(10000);

	LCVLog(DEBUG, "Capturing picture.\n");
	LCVCamSetupCapture(1, LCV_CAM_TRIGGER_MODE_MANUAL);

	LCVLog(DEBUG, "Syncing picture with age/timeout.\n");
	while(1)
	{
		err = LCVCamReadPicture(1, &pPic, 1000, 1);
		if(err == -ETIMEOUT)
			LCVLog(DEBUG, "Timeout occured!\n");
		else
			break;
	}
	if(err != 0)
		return -1;

	LCVLog(DEBUG, "Registering double buffer.\n");
	LCVCamCreateMultiBuffer(2, multibufferIDs);

	LCVLog(DEBUG, "Using double buffers.\n");
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);

	LCVLog(DEBUG, "Deleting double buffer.\n");
	LCVCamDeleteMultiBuffer();

	LCVLog(DEBUG, "Registering 4-way multi buffer.\n");
	LCVCamCreateMultiBuffer(4, multibufferIDs);

	LCVLog(DEBUG, "Using multi buffers.\n");
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamSetupCapture(LCV_CAM_MULTI_BUFFER, LCV_CAM_TRIGGER_MODE_MANUAL);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);	
	LCVCamReadPicture(LCV_CAM_MULTI_BUFFER, &pPic, 0, 0);	
	

	LCVLog(DEBUG, "Getting latest picture.\n");
	LCVCamReadLatestPicture((uint8**)(&pPic));

	LCVCamDestroy(hFramework);
	LCVLog(DEBUG, "------------------------------------------------------\n");
	return 0;
}

int testPriority()
{
	int prio;
	int policy;
	int ret;
	struct sched_param param;

	prio = sched_get_priority_max(SCHED_FIFO);
	LCVLog(DEBUG, "Min prio: %d. Max prio: %d\n", sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));

	param.sched_priority = prio;
	ret =sched_setscheduler(0, /* This process */
				SCHED_FIFO, /* Real-time priority */
				&param);
	if(ret < 0)
		LCVLog(ERROR, "Error setting priority! (%s)\n", strerror(errno));

	prio = 0xdeadbeef;
	policy = sched_getscheduler(0);
	if(policy == SCHED_FIFO)
		LCVLog(DEBUG, "Successfully set real-time priority.\n");
	else {
		LCVLog(ERROR, "Setting real-time priority unsuccessful (%d).\n", 
		       policy);
		return -1;
	}

	LCVLog(DEBUG, "------------------------------------------------------\n");
	return 0;
}

int testWatchdog()
{
	LCV_ERR err;
	int i;

	if((err = LCVSupCreate(hFramework)))
		return -1;
	
	LCVSupWdtInit();
	LCVLog(DEBUG, "%s: Watchdog initialized!\n", __func__);
	
	for(i=0; i < 5; i++)
	{
		sleep(10); // Sleep for ten seconds
		LCVSupWdtKeepAlive();
	}
	LCVSupWdtClose();
	LCVLog(DEBUG, "%s: Watchdog closed!\n", __func__);

	LCVLog(INFO, "-------------------------------------------------------\n");
	LCVSupDestroy(hFramework);
	return 0;
}

int testCycles()
{
	LCV_ERR err;;
	uint32 start, end, musecs;
	static volatile int test;

	if((err = LCVSupCreate(hFramework)))
		return -1;

	test = 0xdeadbeef;

	LCVLog(DEBUG, "Starting cycle test.\n");
	start = LCVSupCycGet();
	usleep(100000);
	end = LCVSupCycGet();
	musecs = LCVSupCycToMicroSecs(end - start);

	LCVLog(INFO, "%s:\tstart: %u\tend: %u\tdiff: %u\tmuSecs: %u\n",
	       __func__, start, end, end - start, musecs);

	LCVLog(INFO, "-------------------------------------------------------\n");
	LCVSupDestroy(hFramework);
	return 0;
}


/* This process acts only as one part of the IPC connection.
It implements only the server part, the client part is in a 
separate executable called cgitest,
which must be started after this.
Also, for the host, the permissions of the two created FIFOs must
currently be changed manually, since they are automatically created
as root only.
*/
LCV_ERR testIpc()
{
	LCV_IPC_CHAN_ID chan;
	struct LCV_IPC_REQUEST req;
	uint32 servParam;
	LCV_ERR err;
	int i;

	if((err = LCVIpcCreate(hFramework)))
		return -1;

	/* Open channel as non-blocking and server */
	err = LCVIpcRegisterChannel(&chan, "/tmp/SHTSocket.sock", F_IPC_SERVER | F_IPC_NONBLOCKING);
	LCVLog(DEBUG, "IpcRegisterChannel: %d => %d\n", err, chan);
	if(err)
	{
		LCVLog(ERROR, "Unable to register channel!\n");
		return err;
	}

	for(i = 0; i < 2; i++)
//	while(1)
	{
		while(1)
		{
			/* receive request */
			err = LCVIpcGetRequest(chan, &req);
			if(err)
			{
				usleep(1); // yield
				if(err != -ENO_MSG_AVAIL)
				{
					LCVLog(ERROR, "Unable to receive IPC msg! (%d)\n", err);
					return err;
				}
			} else
				break;
		}
		
		if(req.enType == REQ_TYPE_READ)
		{
			/* Execute the request. When compiled with mudflap this will of course yield an error. */
			*((uint32*)req.pAddr) = 0xdeadbeef;
		} else { /* REQ_TYPE_WRITE */
			servParam = *((uint32*)req.pAddr);
			LCVLog(DEBUG, "Wrote 0x%x!\n", servParam);
		}
		
		do
		{
			err = LCVIpcAckRequest(chan, &req, TRUE);
		}while(err == -ETRY_AGAIN);

		if(err)
		{
			LCVLog(ERROR, "Unable to acknowledge request!\n");
			return err;
		}
	}


	err = LCVIpcUnregisterChannel(chan);
	if(err){
		LCVLog(ERROR, "Unregistering channel  failed!\n");
		return -1;
	}

	LCVIpcDestroy(hFramework);
	LCVLog(INFO, "-------------------------------------------------------\n");
	return err;
}

int testRtl()
{
	LCV_ERR err = SUCCESS;
	fract16 a;
	complex_fract16 c;

	c.re = 0x5a82; /* 0.707 */
	c.im = 0x5a82;

	LCVRtlCreate(hFramework);
	a = LCVRtl_cabs_fr16(c);
	printf("Complex absolute of %f + %fi = %f\n", LCVRtlFr16ToFloat(c.re), LCVRtlFr16ToFloat(c.im), LCVRtlFr16ToFloat(a));

	LCVRtlDestroy(hFramework);
	return err;
}

void pblkl (void *a, int w, int h, int s)
{
  unsigned *p = a;
  int x,y;
  printf ("%08x\n", (unsigned int)p);
  for (y=0;y<h;y++) {
    for (x=0;x<w;x++)
      printf ("%4d ", *p++);
    printf ("\n");
    if (s)
      p += (s-w);
  }
  printf ("\n");
}


int testDma()
{

	void * hDmaChain;
	LCV_ERR err = SUCCESS;
	int i;
	uint32 buf1[1024/4];
	uint32 buf2[1024/4];

	LCVDmaCreate(hFramework);
	
	for(i=0; i < 1024/4; i++)
	{
		buf1[i] = 0x10000 + i;
		buf2[i] = 0;
	}
	
	pblkl(buf1, 16, 16, 16);
	/* Flush the source memory. */
	FLUSH_REGION((void*)buf1, 1024 + CACHE_LINE_LEN);
	/* Invalidate the destination memory. Do this before the actual
	 transfer since if you do it afterwards, the newly written memory
	will get overwritten again by the contents of the cache. 
	(!FLUSH!)INVALIDATE */
	FLUSHINV_REGION((void*)buf2, 1024 + CACHE_LINE_LEN);
	
	LCVDmaAllocChain(&hDmaChain);
	LCVLog(INFO, "MemcpySync:\n\n");
	LCVDmaMemCpySync(hDmaChain, buf2, buf1, 1024);
	pblkl(buf2, 16, 16, 16);

	for(i=0; i < 1024/4; i++)
	{
		buf1[i] = 0;
	}
	FLUSHINV_REGION((void*)buf1, 1024 + CACHE_LINE_LEN);

	LCVLog(INFO, "1D/2D move:\n\n");
	LCVDmaResetChain(hDmaChain);
	err = LCVDmaAdd1DMove(hDmaChain,
			      buf1,
			      DMA_WDSIZE_16,
			      1024/4, 4,
			      buf2,
			      DMA_WDSIZE_16,
			      1024/4, 4);
	if(err != SUCCESS)
	{
		LCVLog(ERROR, "Unable to add 1D move!(%d)\n", err);
		return err;
	}
	err = LCVDmaAdd2DMove(hDmaChain,
			      buf2,
			      DMA_WDSIZE_32,
			      16, 4,
			      16, 4,
			      buf1,
			      DMA_WDSIZE_32,
			      16, 16*4,
			      16, -15*16*4 + 4);
	if(err != SUCCESS) {
		LCVLog(ERROR, "Unable to add 2D move!(%d)\n", err);
		return err;
		}
	err = LCVDmaAddSyncPoint(hDmaChain);
	if(err != SUCCESS){
		LCVLog(ERROR, "Unable to add sync point!(%d)\n", err);
		return err;
	}

	LCVDmaStart(hDmaChain);
	LCVLog(DEBUG, "Started...\n");
	err = LCVDmaSync(hDmaChain);
	if(err != SUCCESS)
	{
		LCVLog(ERROR, "Unable to sync to DMA chain! (%d)\n", err);
		return err;
	}
	LCVLog(INFO, "Buf1:\n");
	pblkl(buf1, 16, 16, 16);

	LCVLog(INFO, "Buf2:\n");
	pblkl(buf2, 16, 16, 16);

	LCVDmaDestroy(hFramework);
	return 0;
}

/*..........................................................................*/
/* For the HSM module test we use a small state diagram example called Fsm.
 * It consists of two main states (alpha, beta) and a substate (subbeta) of 
 * beta. Two events are defined which force transitions:
 * TOSUBBETA defined for state alpha, forces transition to subbeta.
 * TOALPHA defined for beta, forces transitions to alpha (from beta or subbeta)
 * The start transition goes to alpha.
 * */

typedef struct Fsm Fsm;
struct Fsm {
    Hsm super;
    State alpha, beta;
    State subbeta; /* substate of beta */
};

enum FsmEvents {
	TOALPHA_EVT,
	TOSUBBETA_EVT
};


Msg const *Fsm_top(Fsm *me, Msg *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(me, &me->alpha);
        return 0;
    }
    return msg;
}

Msg const *Fsm_alpha(Fsm *me, Msg *msg) {
	switch (msg->evt) {
	case ENTRY_EVT:
		LCVLog(INFO, "Enter state Alpha\n");
		return 0;
    
	case TOSUBBETA_EVT:
		LCVLog(INFO, "msg arrived TOSUBBETA_EVT\n");
		STATE_TRAN(me, &me->subbeta);	
		return 0;
    	}
    	return msg;
}

Msg const *Fsm_beta(Fsm *me, Msg *msg) {
        switch (msg->evt) {
        case ENTRY_EVT:
                LCVLog(INFO, "Enter state Beta\n");
                return 0;
    
        case TOALPHA_EVT:
                LCVLog(INFO, "msg arrived ALPHA_EVT\n");
                STATE_TRAN(me, &me->alpha);   
                return 0;
        }
        return msg;
}

Msg const *Fsm_subbeta(Fsm *me, Msg *msg) {
        switch (msg->evt) {
        case ENTRY_EVT:
                LCVLog(INFO, "Enter state Sub-Beta\n");
                return 0;
        }
        return msg;
}



void FsmCtor(Fsm *me){
	HsmCtor((Hsm *)me, "Fsm", (EvtHndlr)Fsm_top);
	StateCtor(&me->alpha, "alpha",
              &((Hsm *)me)->top, (EvtHndlr)Fsm_alpha);
        StateCtor(&me->beta, "beta",
              &((Hsm *)me)->top, (EvtHndlr)Fsm_beta);
        StateCtor(&me->subbeta, "subbeta",
              &me->beta, (EvtHndlr)Fsm_subbeta);
}

const Msg fsmMsg[] = { 
        {TOALPHA_EVT},
        {TOSUBBETA_EVT} 
};



int testHsm()
{
	LCV_ERR err = SUCCESS;

        LCVHsmCreate(hFramework);

	Msg const *msg;
	Fsm fsm;
	FsmCtor(&fsm);
	HsmOnStart((Hsm *)&fsm);

	msg = &fsmMsg[ TOSUBBETA_EVT];
	HsmOnEvent((Hsm *)&fsm, msg);

        msg = &fsmMsg[ TOALPHA_EVT];
        HsmOnEvent((Hsm *)&fsm, msg);

        msg = &fsmMsg[ TOSUBBETA_EVT];
        HsmOnEvent((Hsm *)&fsm, msg);

	LCVRtlDestroy(hFramework);
	return err;

}

/*..........................................................................*/

int main()
{
	LCV_ERR err;

#ifdef LCV_TARGET
	if(testPriority())
		return -1;
#endif


	err = LCVCreate(&hFramework);
	if(err < 0) 
	{
		printf("Unable to create framework.\n");
		return -1;
	}

	if((err = LCVLogCreate(hFramework)))
		return -1;

/*	if(testHsm())
		return -1;

	if(testRtl())
		return -1;

	if(testLgx())
		return -1;

	if(testCamRegs())
		return -1;

	if(testBMP())
		return -1;
*/
	if(testSwr())
		return -1;

/*
	if(testCam())
	        return -1;
	
	if(testCycles())
		return -1;

	if(testWatchdog())
		return -1;

	if(testIpc())
		return -1;
	else
		LCVLog(DEBUG, "IPC test succeeded!\n");

	if(testDma())
		return -1;
*/
	LCVLogDestroy(hFramework);
	LCVDestroy(hFramework);
	return 0;
}
