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

/*! @file cam_shared.c
 * @brief Camera module implementation for function shared by target
 * and host.
	
 * On the OSC-specific hardware featuring
 * a Micron MT9V032 CMOS image sensor.
 * 
 */
#include "oscar_intern.h"
#include "cam_pub.h"
#include "cam_priv.h"
#include <unistd.h>

extern struct OSC_CAM cam;

/*! @brief Determine whether an integer number is even. */
#define IS_EVEN(x)         (!((x) & 0x1))

OSC_ERR OscCamSetShutterWidth(const uint32 usecs)
{
	uint32          nPixelClks;
	uint16          shutterWidth;
	OSC_ERR         err;
	
	/* Perform a conversion from usecs to number of rows
	 * with rounding. */
	nPixelClks = (usecs*(CAM_PIX_CLK/1000000));
	/* Rounding division. */
	shutterWidth = (uint16)
		((nPixelClks + cam.curCamRowClks/2)/cam.curCamRowClks);
	
	err = OscCamSetRegisterValue(CAM_REG_SHUTTER_WIDTH,
			(uint16)shutterWidth);
	if(err == SUCCESS)
	{
		cam.curExpTime = usecs;
	}
	return err;
}

OSC_ERR OscCamGetShutterWidth(uint32 * pResult)
{
	uint16          shutterWidth;
	OSC_ERR         err;
	
	/* Input validation */
	if(unlikely(pResult == NULL))
	{
		return -EINVALID_PARAMETER;
	}
	
	err = OscCamGetRegisterValue(CAM_REG_SHUTTER_WIDTH,
			&shutterWidth);
	if(unlikely(err < 0))
	{
		return err;
	}
	
	/* Convert from number of rows to usecs */
	*pResult =
		(uint32)shutterWidth*cam.curCamRowClks/(CAM_PIX_CLK/1000000);
	
	return SUCCESS;
}

OSC_ERR OscCamGetAreaOfInterest(uint16 *pLowX,
		uint16 *pLowY,
		uint16 *pWidth,
		uint16 *pHeight)
{
	OSC_ERR err;
	
	if((err = OscCamGetRegisterValue(CAM_REG_COL_START, pLowX)) ||
			(err = OscCamGetRegisterValue(CAM_REG_ROW_START, pLowY)) ||
			(err = OscCamGetRegisterValue(CAM_REG_WIN_WIDTH, pWidth)) ||
			(err = OscCamGetRegisterValue(CAM_REG_WIN_HEIGHT, pHeight)))
	{
		OscLog(ERROR, "%s: Error retrieving area of interest from "
				"camera!\n", __func__);
		return err;
	}
	
#ifdef OSC_TARGET
	/* Apply flip bits depending on the perspective setup */
	if( cam.flipHorizontal)
	{
		*pLowX = MAX_IMAGE_WIDTH - (*pLowX + *pWidth);
	}
	if( cam.flipVertical)
	{
		*pLowY = MAX_IMAGE_HEIGHT - (*pLowY + *pHeight);
	}
#endif
	
	cam.capWin.col_off = *pLowX;
	cam.capWin.row_off = *pLowY;
	cam.capWin.width = *pWidth;
	cam.capWin.height = *pHeight;
	
	/* Update the row time. It has a lower bound. */
	cam.curCamRowClks = cam.capWin.width + cam.curHorizBlank;
	if(cam.curCamRowClks < CAM_MIN_ROW_CLKS)
	{
		cam.curCamRowClks = CAM_MIN_ROW_CLKS;
	}
	
	return SUCCESS;
}

OSC_ERR OscCamSetBlackLevelOffset(const uint16 offset)
{
	uint16 reg;
	
	if( (offset<<2) > 0xff)
	{
		reg = 0xff;
	}
	else
	{
		reg = offset << 2;
	}
	OscCamSetRegisterValue( CAM_REG_ROW_NOISE_CONST , reg);
	
	return SUCCESS;
}

OSC_ERR OscCamGetBlackLevelOffset(uint16 *pOffset)
{
	uint16 reg;
	
	OscCamGetRegisterValue( CAM_REG_ROW_NOISE_CONST , &reg);
	*pOffset = reg >> 2;
	
	return SUCCESS;
}

OSC_ERR OscCamCreateMultiBuffer(const uint8 multiBufferDepth,
		const uint8 bufferIDs[])
{
	int i;
	struct frame_buffer * pFB;

	/* Input validation */
	if((multiBufferDepth < 2) ||
					(multiBufferDepth > MAX_NR_FRAME_BUFFERS))
	{
		OscLog(ERROR, "%s(%u, 0x%x): Invalid Parameter!\n",
				__func__, multiBufferDepth, bufferIDs);
		return -EINVALID_PARAMETER;
	}
	for(i = 0; i < multiBufferDepth; i++)
	{
		if(bufferIDs[i] > MAX_NR_FRAME_BUFFERS)
		{
			OscLog(ERROR, "%s(%u, 0x%x): Invalid Parameter!\n",
					__func__, multiBufferDepth, bufferIDs);
			return -EINVALID_PARAMETER;
		}
		pFB = &cam.fbufs[bufferIDs[i]];
		if(pFB->data == NULL)
		{
			OscLog(ERROR, "%s: \
					Invalid frame buffer in multibuffer.\n",
					__func__);
			return -EINVALID_PARAMETER;
		}
	}

	/* Create the multi buffer */
	return OscCamMultiBufferCreate(&cam.multiBuffer,
			multiBufferDepth,
			bufferIDs);
}

OSC_ERR OscCamDeleteMultiBuffer()
{
	return OscCamMultiBufferDestroy(&cam.multiBuffer);
}

OSC_ERR OscCamSetupPerspective(const enum EnOscCamPerspective perspective)
{
	uint16  reg;
	bool    rowFlip, colFlip;

	switch(perspective)
	{
	case OSC_CAM_PERSPECTIVE_DEFAULT:
		rowFlip = FALSE;
		colFlip = FALSE;
		break;
	case OSC_CAM_PERSPECTIVE_HORIZONTAL_MIRROR:
		rowFlip = FALSE;
		colFlip = TRUE;
		break;
	case OSC_CAM_PERSPECTIVE_VERTICAL_MIRROR:
		rowFlip = TRUE;
		colFlip = FALSE;
		break;
	case OSC_CAM_PERSPECTIVE_180DEG_ROTATE:
		rowFlip = TRUE;
		colFlip = TRUE;
		break;
	default:
		return EINVALID_PARAMETER;
	}
	
#ifdef TARGET_TYPE_INDXCAM
	/* This board has a 180 deg rotated sensor related to the
	 * board bottom side */
	rowFlip = !rowFlip;
	colFlip = !colFlip;
#endif /* TARGET_TYPE_INDXCAM */
			
	/* Modify affected register fields */
	OscCamGetRegisterValue( CAM_REG_READ_MODE, &reg);
	if(rowFlip){
		reg = reg | (1<<CAM_REG_READ_MODE_ROW_FLIP);
	} else {
		reg = reg & (0xffff ^ (1<<CAM_REG_READ_MODE_ROW_FLIP));
	}
	if(colFlip){
		reg = reg | (1<<CAM_REG_READ_MODE_COL_FLIP);
	} else {
		reg = reg & (0xffff ^ (1<<CAM_REG_READ_MODE_COL_FLIP));
	}
	OscCamSetRegisterValue( CAM_REG_READ_MODE, reg);
	
	/* Store flip information in module */
	cam.flipHorizontal = colFlip;
	cam.flipVertical = rowFlip;
	
	return SUCCESS;
}

OSC_ERR PerspectiveCfgStr2Enum(const char *str, enum EnOscCamPerspective *per )
{
	OSC_ERR err = SUCCESS;
	
	if(0 == strcmp(str, OSC_CAM_PERSPECTIVE_CFG_STR_DEFAULT) )
	{
		*per = OSC_CAM_PERSPECTIVE_DEFAULT;
	} else {
		if(0 == strcmp(str, OSC_CAM_PERSPECTIVE_CFG_STR_HORIZONTAL_MIRROR) )
		{
			*per = OSC_CAM_PERSPECTIVE_HORIZONTAL_MIRROR;
		} else {
			if(0 == strcmp(str, OSC_CAM_PERSPECTIVE_CFG_STR_VERTICAL_MIRROR) )
			{
				*per = OSC_CAM_PERSPECTIVE_VERTICAL_MIRROR;
			} else {
				if(0 == strcmp(str, OSC_CAM_PERSPECTIVE_CFG_STR_180DEG_ROTATE) )
				{
					*per = OSC_CAM_PERSPECTIVE_180DEG_ROTATE;
				} else {
					err = EINVALID_PARAMETER;
					*per = OSC_CAM_PERSPECTIVE_DEFAULT;
				}
			}
		}
	}
	return err;
}

OSC_ERR OscCamPresetRegs()
{
	OSC_ERR err;
	
	/* Reset frame capture and AGC/Exposure logic.
	 * Registers are _not_ set to default as done on power cycle.  */
	err = OscCamSetRegisterValue( CAM_REG_RESET, 3);
	
	/* we need a short sleep here, otherwise the sensor might behave funny afterwards. */
	usleep(100);
	
	/* Snapshot mode; simultaneous readout */
	err |= OscCamSetRegisterValue( CAM_REG_CHIP_CONTROL, 0x398);
	
	
	/* Define color type and operation mode */
	#ifdef TARGET_TYPE_INDXCAM
	/* monochrone, linear */
	err |= OscCamSetRegisterValue( CAM_REG_PIXEL_OP_MODE, 0x0011);
	/* Disable LED_OUT on leanXcam so the GPIOs can function correctly.
	 * This output is or'ed with GPIO_OUT2_N. */
	err |=  OscCamSetRegisterValue(CAM_REG_LED_OUT_CONTROL, 0x03);
	#endif /*TARGET_TYPE_INDXCAM*/
	#ifdef TARGET_TYPE_LEANXCAM
	/* color, linear */
	err |= OscCamSetRegisterValue( CAM_REG_PIXEL_OP_MODE, 0x0015);
	#endif /*TARGET_TYPE_LEANXCAM*/
	
	/* This register is actually marked as reserved in the datasheet but a Micron
	 * representative stated that 0x3d5 is the "optimal" value for this register.
	 * It seems to help brightness a bit. The default value is 0x1d1 */
	err |= OscCamSetRegisterValue( CAM_REG_RESERVED_0x20, 0x3d5);
	
	/* Disable AGC/AEC */
	err |= OscCamSetRegisterValue( CAM_REG_AEC_AGC_ENA, 0x0);
	
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set camera registers! (%d)\n",
				__func__, err);
	}
	
	/* Apply row noise offset. This allows to push the black image histogram
	 * fully above zero for proper FPN correction. */
	err = OscCamSetBlackLevelOffset( CAM_BLACKLEVEL);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set black level! (%d)\n",
				__func__, err);
	}
	
	/* Apply default exposure time */
	err = OscCamSetShutterWidth( CAM_EXPOSURE);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set shutter width! (%d)\n",
				__func__, err);
	}
	
	/* Apply default (max) aera-of-interest */
	err = OscCamSetAreaOfInterest(0,0,0,0);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set area of interest! (%d)\n",
				__func__, err);
	}
		
					
	/* Set default camera - scene perspective relation */
	err = OscCamSetupPerspective(OSC_CAM_PERSPECTIVE_DEFAULT);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up camera perspective! (%d)\n",
				__func__, err);
	}
	
	return SUCCESS;
}

#ifdef TARGET_TYPE_LEANXCAM
OSC_ERR OscCamGetBayerOrder(enum EnBayerOrder *pBayerOrderFirstRow,
							const uint16 xPos,
							const uint16 yPos)
{
	bool bFirstPixIsGreen;
	bool bFirstRowIsRed;
	
	/* Input validation */
	if(unlikely(pBayerOrderFirstRow == NULL) ||
		unlikely(xPos >= OSC_CAM_MAX_IMAGE_WIDTH) ||
		unlikely(yPos >= OSC_CAM_MAX_IMAGE_HEIGHT))
	{
		return -EINVALID_PARAMETER;
	}
	
	/* The Adaptina mt9v032 sensor will automatically adjust for column and
	 * row mirroring by means of a dummy column and read, i.e. the color order
	 * will always be BGBG by default.*/
	bFirstRowIsRed = FALSE;
	bFirstPixIsGreen = FALSE;

	
	/* Take the specified offset to calculate the order of the first row.*/
	if(!IS_EVEN(xPos))
	{
		bFirstPixIsGreen = !bFirstPixIsGreen;
	}
	if(!IS_EVEN(yPos))
	{
		bFirstRowIsRed = !bFirstRowIsRed;
		bFirstPixIsGreen = !bFirstPixIsGreen;
	}
	
	if(bFirstRowIsRed)
	{
		if(bFirstPixIsGreen)
		{
			*pBayerOrderFirstRow = ROW_GRGR;
		} else {
			*pBayerOrderFirstRow = ROW_RGRG;
		}
	} else {
		if(bFirstPixIsGreen)
		{
			*pBayerOrderFirstRow = ROW_GBGB;
		} else {
			*pBayerOrderFirstRow = ROW_BGBG;
		}
	}

	return SUCCESS;
}
#else
OSC_ERR OscCamGetBayerOrder(enum EnBayerOrder *pBayerOrderFirstRow,
							const uint16 xPos,
							const uint16 yPos)
{
	return -ENO_COLOR_SENSOR;
}
#endif /* TARGET_TYPE_LEANXCAM */
