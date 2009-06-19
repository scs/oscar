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
 * @brief Camera module implementation for target
	
 * On the OSC-specific hardware featuring
 * a Micron MT9V032 CMOS image sensor.
 */

#include "cam.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

OSC_ERR OscCamCreate();
OSC_ERR OscCamDestroy();

/*! @brief The module definition. */
struct OscModule OscModule_cam = {
	.create = OscCamCreate,
	.destroy = OscCamDestroy,
	.dependencies = {
		&OscModule_log,
		NULL // To end the flexible array.
	}
};

struct OSC_CAM cam; /*!< @brief The camera module singelton instance */

OSC_ERR OscCamCreate()
{
	OSC_ERR err;
	uint16 dummy;
	
	cam = (struct OSC_CAM) { };
	
	/* Open the video device file.
	 * We will be communicating with the camera driver by issuing
	 * IOCTLs over this file descriptor. */
	cam.vidDev = open(VIDEO_DEVICE_FILE, 2, 0);
	if(unlikely(cam.vidDev < 0))
	{
		printf("%s: Error: Unable to open video device file %s (%s).\n",
				__func__,
				VIDEO_DEVICE_FILE,
				strerror(errno));
		cam.vidDev = 0;
		return -ENO_VIDEO_DEVICE_FOUND;
	}
	
	err = SUCCESS;
#ifdef TARGET_TYPE_LEANXCAM
	/* Disable LED_OUT on leanXcam so the GPIOs can function correctly.
	 * This output is or'ed with GPIO_OUT2_N. */
	err |=  OscCamSetRegisterValue(CAM_REG_LED_OUT_CONTROL, 0x01);
#endif /* TARGET_TYPE_LEANXCAM */

	/* Read the current camera register values and build a model of the
	 * current state from them. */
	err |= OscCamGetShutterWidth(&cam.curExpTime);
	err |= OscCamGetRegisterValue(CAM_REG_HORIZ_BLANK,
			&cam.curHorizBlank);
	/* Read back the area of interest to implicitely update
	 * cam.curCamRowClks. */
	err |= OscCamGetAreaOfInterest(&dummy, &dummy, &dummy, &dummy);
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to read current settings from "
				"camera!\n",
				__func__);
	}
	
	return SUCCESS;
}

OSC_ERR OscCamDestroy()
{
	close(cam.vidDev);
}

OSC_ERR OscCamSetFileNameReader(void* hReaderHandle)
{
	/* Stump implementation on target platform. */
	return SUCCESS;
}

OSC_ERR OscCamSetAreaOfInterest(const uint16 lowX,
								const uint16 lowY,
								const uint16 width,
								const uint16 height)
{
	int ret;
	uint16 w, h, x, y; /* effective width/height/lowX/lowY */
	struct capture_window capWinNotMirror;

	/* Input validation */
	if(width%2 != 0 ||
			lowX + width > MAX_IMAGE_WIDTH ||
			lowY + height > MAX_IMAGE_HEIGHT)
	{
		OscLog(ERROR,
				"%s: Invalid parameter (%dx%d at %d/%d). "
				"Must fit %dx%d and width must be even\n",
				__func__,
				width, height, lowX,lowY,
				MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
		return -EINVALID_PARAMETER;
	}
		
	/* Reset the window to the default. */
	if((width == 0) || (height == 0))
	{
		x = 0;
		y = 0;
		w = MAX_IMAGE_WIDTH;
		h = MAX_IMAGE_HEIGHT;
	} else {
		x = lowX;
		y = lowY;
		w = width;
		h = height;
	}
	
	capWinNotMirror.col_off = x;
	capWinNotMirror.row_off = y;
	capWinNotMirror.width   = w;
	capWinNotMirror.height  = h;
	
	/* Apply flip bits depending on the perspective setup */
	if( cam.flipHorizontal)
	{
		x = MAX_IMAGE_WIDTH - (x + w);
	}
	if( cam.flipVertical)
	{
		y = MAX_IMAGE_HEIGHT - (y + h);
	}
		
	cam.capWin.col_off = x;
	cam.capWin.row_off = y;
	cam.capWin.width   = w;
	cam.capWin.height  = h;
	
	/* Communicate the desired window to the driver.
	 * The driver does input validation */
	ret = ioctl(cam.vidDev, CAM_SWINDOW, &cam.capWin);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to set capture window: \
				IOCTL failed with %d.\n",
				__func__, errno);
		
		/* An error has occured */
		if(errno == EINVAL) /* Invalid parameter */
		{
			OscLog(ERROR, "%s(%u, %u, %u, %u): Invalid parameter!\n",
					__func__, lowX, lowY, width, height);
			return -EINVALID_PARAMETER;
		}
		return -EDEVICE;
	}
	
	/* The row time of the camera consists of the pixel readout time
	 * for the width of the row plus the horizontal blanking. The time
	 * has a lower bound. */
	cam.curCamRowClks = cam.capWin.width + cam.curHorizBlank;
	if(cam.curCamRowClks < CAM_MIN_ROW_CLKS)
	{
		cam.curCamRowClks = CAM_MIN_ROW_CLKS;
	}
	
	OscLog(DEBUG, "%s: Area of interest set to %dx%d at %d/%d.\n",
			__func__,
			capWinNotMirror.width,
			capWinNotMirror.height,
			capWinNotMirror.col_off,
			capWinNotMirror.row_off);
	
	return SUCCESS;
}

OSC_ERR OscCamSetRegisterValue(const uint32 reg, const uint16 value)
{
	int                 ret;
	struct reg_info     reg_info;
	
	reg_info.addr = reg;
	reg_info.value = value;

	/* Communicate the desired register value to the driver.
	 * No input validation is done to retain flexibility */
	ret = ioctl(cam.vidDev, CAM_SCAMREG, &reg_info);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to set register 0x%x to 0x%x: \
				IOCTL failed with %d.\n",
				__func__, reg, value, errno);
		
		/* An error has occured */
		if(errno == -EINVAL) /* Invalid parameter */
		{
			OscLog(ERROR, "%s(%u, %u): Invalid parameter!\n",
					__func__, reg, value);
			return -EINVALID_PARAMETER;
		}
		return -EDEVICE;
	}
	return SUCCESS;
}

OSC_ERR OscCamGetRegisterValue(const uint32 reg, uint16 *pResult)
{
	int                 ret;
	struct  reg_info    reg_info;
	
	reg_info.addr = reg;
	reg_info.value = 0;
	
	/* Input validation */
	if(unlikely(pResult == NULL))
	{
		return -EINVALID_PARAMETER;
	}
	
	/* Communicate the desired register value to the driver.
	 * No input validation is done to retain flexibility */
	ret = ioctl(cam.vidDev, CAM_GCAMREG, &reg_info);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to get register 0x%x: \
				IOCTL failed with %d.\n",
				__func__, reg, errno);
		
		/* An error has occured */
		if(errno == -EINVAL) /* Invalid parameter */
		{
			OscLog(ERROR, "%s(%u, 0x%x): Invalid parameter!\n",
					__func__, reg, pResult);
			return -EINVALID_PARAMETER;
		}
		return -EDEVICE;
	}
	*pResult = reg_info.value;
	return SUCCESS;
}

OSC_ERR OscCamSetFrameBuffer(const uint8 fbID,
		const uint32 uSize,
		const void * pData,
		const int bCached)
{
	struct frame_buffer     fb;
	int                     ret;
	int                     i;
	
	/* Input validation */
	if(unlikely(fbID > MAX_NR_FRAME_BUFFERS ||
			(pData == NULL && uSize != 0)))
	{
		OscLog(ERROR,
				"%s(%d, %d, 0x%x, %d): \
				Invalid parameter.\n",
				__func__, fbID, uSize, pData, bCached);
		return -EINVALID_PARAMETER;
	}
	
	fb.id = fbID;
	fb.size = uSize;
	fb.data = (void *)pData;
	fb.flags = 0;
	if(bCached)
	{
		fb.flags = FB_FLAG_CACHED;
	}
	
	if(unlikely(pData == NULL))
	{
		OscLog(INFO, "%s: Deleting frame buffer number %d.\n",
				__func__, fbID);
		/* Check whether the deleted frame buffer belongs to a multi
		 * buffer. */
		for(i = 0; i < cam.multiBuffer.multiBufferDepth; i++)
		{
			if(cam.multiBuffer.fbIDs[i] == fbID)
			{
				OscLog(ERROR,
						"%s Deleting frame buffer %d being part of \
						a multi buffer!.\n",
						__func__, fbID);
				return -ECANNOT_DELETE;
			}
		}
	}
	
	ret = ioctl(cam.vidDev, CAM_SFRAMEBUF, &fb);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to set framebuffer %d: \
				IOCTL failed with %d.\n",
				__func__, fbID, errno);

		switch(errno)
		{
		case EINVAL:  /* Invalid parameter */
			OscLog(ERROR, "%s(%u, %u, 0x%x, %d): Invalid parameter!\n",
					__func__, fbID, uSize, pData, bCached);
			return -EINVALID_PARAMETER;
			break;
		case EBUSY:
			OscLog(ERROR,
					"%s: Camera device is busy.\n",
					__func__);
			return -EDEVICE_BUSY;
			break;
		default:
			OscLog(ERROR, "%s: IOCTL Error \"%s\" (%d)\n",
						__func__, strerror(errno), errno);
			return -EDEVICE;
		}
	}
	
	/* Store the info for later */
	memcpy(&cam.fbufs[fb.id], &fb, sizeof(struct frame_buffer));
	
	return SUCCESS;
}

OSC_ERR OscCamSetupCapture(uint8 fbID)
{
	struct capture_param   cp;
	int                    ret;
	uint8                  fb;
	
	if(unlikely((fbID != OSC_CAM_MULTI_BUFFER) &&
			(fbID > MAX_NR_FRAME_BUFFERS)))
	{
		return -EINVALID_PARAMETER;
	}
	
	if(unlikely(cam.capWin.width == 0 || cam.capWin.height == 0))
	{
		OscLog(ERROR, "%s: No area of interest set!\n",
				__func__);
		return -ENO_AREA_OF_INTEREST_SET;
	}
	
	cp.trigger_mode = TRIGGER_MODE_EXTERNAL;
	
	/* If the caller is using automatic multibuffer management,
	 * get the correct frame buffer. */
	fb = fbID;
	if(fbID == OSC_CAM_MULTI_BUFFER)
	{
		/* Get the buffer ID to write to next */
		fb = OscCamMultiBufferGetCapBuf(&cam.multiBuffer);
	}
	cp.frame_buffer = fb;
	cp.window = cam.capWin;
	
	OscLog(DEBUG,
				"%s: Setting up capture of %ux%d picture " \
				"on frame buffer %d.\n",
				__func__, cam.capWin.width, cam.capWin.height, fb);
	
	ret = ioctl(cam.vidDev, CAM_CCAPTURE, &cp);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to set up capture on framebuffer %d: \
				IOCTL failed with %d.\n",
				__func__, fb, errno);
		
		switch(errno)
		{
		case EINVAL:  /* Invalid parameter */
			OscLog(ERROR, "%s(%u): Invalid parameter!\n",
					__func__, fbID);
			return -EINVALID_PARAMETER;
			break;
		case EBUSY:
			OscLog(ERROR,
					"%s: Camera device is busy.\n",
					__func__);
			return -EDEVICE_BUSY;
			break;
		default:
			OscLog(ERROR, "%s: IOCTL Error \"%s\" (%d)\n",
					__func__, strerror(errno), errno);
			return -EDEVICE;
		}
	}
	
	/* The operation was successful */
	
	if(fbID == OSC_CAM_MULTI_BUFFER)
	{
		/* Allow the multi buffer to update is status according to this
		 * successful capture. */
		OscCamMultiBufferCapture(&cam.multiBuffer);
	}
	return SUCCESS;
}

OSC_ERR OscCamCancelCapture()
{
	int ret;
	
	ret = ioctl(cam.vidDev, CAM_CABORTCAPT, 0);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to cancel capture: \
				IOCTL failed with %d.\n",
				__func__, errno);

		/* An error has occured */
		if(errno == -ENOENT) /* Picture already finished */
		{
			OscLog(WARN, "%s: Cancel request when no picture \
					transfer to cancel.\n",
					__func__);
			return -ENOTHING_TO_ABORT;
		}
		return -EDEVICE;
	}
	return SUCCESS;
}

OSC_ERR OscCamReadPicture(const uint8 fbID, 
		uint8 ** ppPic, 
		const uint16 maxAge,
		const uint16 timeout)
{
	struct sync_param   sp;
	int                 ret;
	OSC_ERR             err = SUCCESS;
	uint8               fb;
	
	/* If the caller is using automatic multibuffer management,
	 * get the correct frame buffer. */
	fb = fbID;
	if(fb == OSC_CAM_MULTI_BUFFER)
	{
		/* Get the correct buffer ID */
		fb = OscCamMultiBufferGetSyncBuf(&cam.multiBuffer);
		if(fb == OSC_CAM_INVALID_BUFFER_ID)
		{
			return -ENO_CAPTURE_STARTED;
		}
	}
		
	/* Input validation */
	if(unlikely(ppPic == NULL || fb > MAX_NR_FRAME_BUFFERS ||
			cam.fbufs[fb].data == NULL))
	{
		return -EINVALID_PARAMETER;
	}
	
	*ppPic = NULL;     /* Precaution */
	
	sp.frame = fb;
	sp.max_age = maxAge;
	sp.timeout = timeout;
	
	OscLog(DEBUG,
			"%s(%u, 0x%x, %u, %u): Syncing capture on frame buffer %d.\n",
			__func__, fbID, ppPic, maxAge, timeout, fb);
	ret = ioctl(cam.vidDev, CAM_CSYNC, &sp);
	if(ret < 0)
	{
		switch(errno)
		{
		case EINVAL: /* Invalid parameter */
			OscLog(ERROR, "%s(%u, 0x%x, %u, %u): Invalid parameter!\n",
					__func__, fb, ppPic, maxAge, timeout);
			return -EINVALID_PARAMETER;
			break;
		case EINTR: /* Interrupt */
			OscLog(WARN, "%s: Sync on frame buffer %d got interrupted!\n",
				__func__, fb);
			return -EINTERRUPTED;
			break;
		case EAGAIN: /* Timeout */
			OscLog(DEBUG, "%s: Sync on frame buffer %d timed out.\n",
					__func__, fb);
			return -ETIMEOUT;
			break;
		case ERANGE: /* Too old */
			OscLog(DEBUG,
					"%s: Sync on frame buffer %d returned too late.\n",
					__func__, fbID);
			err = -EPICTURE_TOO_OLD;
			break; /* User may still want picture */
		default:
			OscLog(ERROR, "%s: IOCTL Error \"%s\" (%d)\n",
					__func__, strerror(errno), errno);
			return -EDEVICE;
		}
	}
	
	*ppPic = cam.fbufs[fb].data;
	
	/* Apply image correction */
	if( cam.pCallback)
	{
		err = (*cam.pCallback)(
				*ppPic,
				cam.capWin.col_off,
				cam.capWin.row_off,
				cam.capWin.width,
				cam.capWin.height);
	}

	
	/* The operation was successful */
	if(fbID == OSC_CAM_MULTI_BUFFER)
	{
		/* Allow the multi buffer to update is status according to this
		 * successful read. */
		OscCamMultiBufferSync(&cam.multiBuffer);
	}
	return err;
}

OSC_ERR OscCamReadLatestPicture(uint8 ** ppPic)
{
	struct image_info image_info;
	int ret;
	
	if(unlikely(ppPic == NULL))
	{
		return -EINVALID_PARAMETER;
	}
	
	image_info.window = cam.capWin;
	/* The driver will automatically find the last image with the
	 * specified size */
	ret = ioctl(cam.vidDev, CAM_GLASTFRAME, &image_info);
	if(unlikely(ret < 0))
	{
		OscLog(ERROR,
				"%s: Unable to get last frame: \
				IOCTL failed with %d.\n",
				__func__, errno);

		switch(errno)
		{
		case EINVAL:
			return -EINVALID_PARAMETER;
			break;
		case ENOENT:
			OscLog(ERROR, "%s: No matching picture found.\n",
					__func__);
			return -ENO_MATCHING_PICTURE;
			break;
		default:
			OscLog(ERROR, "%s: IOCTL Error \"%s\" (%d)\n",
					__func__, strerror(errno), errno);
			return -EIO;
		}
	}
	*ppPic = (uint8*)image_info.fbuf;
	return SUCCESS;
}

OSC_ERR OscCamRegisterCorrectionCallback(
		int (*pCallback)(
				uint8 *pImg,
				const uint16 lowX,
				const uint16 lowY,
				const uint16 width,
				const uint16 height))
{
	cam.pCallback = pCallback;
	return SUCCESS;
}

#ifdef TARGET_TYPE_LEANXCAM

OSC_ERR OscCamConfigSensorLedOut(bool bSensorLedOut, bool bInvert)
{
	OSC_ERR err;
	
	if(bSensorLedOut)
	{
		if(bInvert)
		{
			/* Enable and invert the LED_OUT pin of the CMOS sensor. */
			err = OscCamSetRegisterValue(CAM_REG_LED_OUT_CONTROL, 0x02);
		} else {
			/* Enable the LED_OUT pin of the CMOS sensor. */
			err = OscCamSetRegisterValue(CAM_REG_LED_OUT_CONTROL, 0x00);
		}
	} else {
		/* Disable the LED_OUT pin of the CMOS sensor. */
		err = OscCamSetRegisterValue(CAM_REG_LED_OUT_CONTROL, 0x01);
	}
	return err;
}

#endif /* TARGET_TYPE_LEANXCAM */


