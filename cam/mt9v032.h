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

/*! @file mt9v032.h
 * @brief API for mt9v032h camera driver
 */

#ifndef __MT9V032_H__
#define __MT9V032_H__

#include <linux/ioctl.h>

/*! @brief Maximum number of frame buffers in the driver. */
#define MAX_NR_FRAME_BUFFERS 8

/*! @brief The maximum image width of the sensor. */
#define MAX_IMAGE_WIDTH 752
/*! @brief The maximum image height of the sensor. */
#define MAX_IMAGE_HEIGHT 480

/*! @brief Argument structure for IOCTL CAM_SFRAMEBUF. */
struct frame_buffer {
	int size;           /*!< @brief The frame buffer size in bytes. */
	int id;             /*!< @brief The nunmber of this frame buffer. */
	int flags;          /*!< @brief Flags for this frame buffer. */
	/*! @brief The memory of this frame buffer is in a cached region. */
#define FB_FLAG_CACHED    0x1
	void * data;        /*!< @brief The pointer to the data. */
};

/*! @brief Argument structure for IOCTL CAM_SWINDOW and CAM_GWINDOW. */
struct capture_window {
	int width;     /*!< @brief The width of the frame to be captured. */
	int height;    /*!< @brief The height of the frame to be captured. */
	int col_off;   /*!< @brief The column (x) offset of the frame. */
	int row_off;   /*!< @brief The row (y) offset of the frame. */
};

/*! @brief Argument structure for IOCTL CAM_CCAPTURE. */
struct capture_param {
	/*! @brief The capture window of the frame to be taken. */
	struct capture_window window;
	/*! @brief The number of the frame buffer to capture into. */
	int frame_buffer;
	/*! @brief The trigger mode of the capture. */
	int trigger_mode;
	/*! @brief CMOS sensor is triggered by an external trigger. */
#define TRIGGER_MODE_EXTERNAL 1
	/*! @brief CMOS sensor is triggered manually over a GPIO. */
#define TRIGGER_MODE_MANUAL   2
};

/*! @brief Argument structure for IOCTL CAM_GLASTFRAME */
struct image_info {
	/*! @brief The window (and thus the format of the captured image). */
	struct capture_window window;
	/*! @brief The frame buffer where this image is stored. */
	unsigned char *fbuf;
};

/*! @brief Argument structure for IOCTL CAM_SCAMREG and CAM_GCAMREG */
struct reg_info {
	/*! @brief The address in the address space of the camera
	 *  designated to this register. */
	int addr;
	/*! @brief The corresponding value. */
	int value;
};

/*! @brief Argument structure for IOCTL CAM_CSYNC */
struct sync_param {
	/*! @brief The numeric identifier for the frame buffer to be
	 * synchronized. */
	int frame;
	/*! @brief The timeout of the command.
	 * 
	 * How many Milliseconds this the CAM_CSYNC IOCTL should maximally
	 * wait for pixel data before returning. 0 means no timeout.
	 * An error code of EAGAIN is returned if the request timed out. */
	unsigned int timeout;
	/*! @brief The maximum age of the picture to synchronize to in
	 * Milliseconds.
	 * 
	 * This is used to ensure a capture which is completed during a
	 * timeout is still accepted as long as not too much time passed.
	 * 0 means no restriction. An error code of ERANGE is returned if
	 *  picture too old. */
	unsigned int max_age;
};


/***************************** IOCTLS *******************************/
/*! @brief Magic number to create the IOCTL numbers */
#define MT9V032_MAGIC 'm'

/*! @brief Set the frame buffers to be used */
#define CAM_SFRAMEBUF  _IOW(MT9V032_MAGIC, 0, struct frame_buffer)
/*! @brief Set the capture window of future images */
#define CAM_SWINDOW    _IOW(MT9V032_MAGIC, 2, struct capture_window)
/*! @brief Get the capture window  */
#define CAM_GWINDOW    _IOR(MT9V032_MAGIC, 3, struct capture_window)
/*! @brief Initiate the capture of an image */
#define CAM_CCAPTURE   _IOW(MT9V032_MAGIC, 4, struct capture_param)
/*! @brief Abort a previous ioctl of CAM_CCAPTURE */
#define CAM_CABORTCAPT _IOW(MT9V032_MAGIC, 5, int)
/*! @brief Synchronize a frame buffer (wait until the image is captured
 * and in RAM) */
#define CAM_CSYNC      _IOR(MT9V032_MAGIC, 5, struct sync_param)
/*! @brief Get the last captured frame */
#define CAM_GLASTFRAME _IOR(MT9V032_MAGIC, 6, struct image_info)
/*! @brief Write the specified register of the camera */
#define CAM_SCAMREG    _IOW(MT9V032_MAGIC, 7, struct reg_info)
/*! @brief Read the specified register of the camera */
#define CAM_GCAMREG    _IOR(MT9V032_MAGIC, 8, struct reg_info)

#endif /* __MT9V032_H__ */
