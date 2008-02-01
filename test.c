#include "inc/framework.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sched.h>
#include <errno.h>

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

int testCamRegs()
{
	uint16 maxShutterWidth;
	uint32 shutterWidth;
	LCV_ERR err;

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

	LCVLog(INFO, "-------------------------------------------------------\n");
	return 0;
}

int testBMP()
{
	struct LCV_PICTURE pic1;
	struct LCV_PICTURE pic2;
	struct LCV_PICTURE pic3;

	uint32 imageSize;

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

int main()
{
	void * hFramework;
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

	if((err = LCVBmpCreate(hFramework)))
		return -1;

	if((err = LCVCamCreate(hFramework)))
		return -1;

	if((err = LCVLogCreate(hFramework)))
		return -1;

	if((err = LCVIpcCreate(hFramework)))
		return -1;

	if(testLog())
		return -1;

	if(testCamRegs())
		return -1;

#ifdef LCV_HOST
	if(testBMP())
		return -1;
#endif
	if(testCam())
		return -1;

	LCVDestroy(hFramework);
	return 0;
}
