/*! @file cam_shared.c
 * @brief Camera module implementation for function shared by target
 * and host.
 
 * On the LCV-specific hardware featuring 
 * a Micron MT9V032 CMOS image sensor.
 * 
 * @author Markus Berner, Samuel Zahnd
 */
#include "framework_intern.h"
#include "cam_pub.h"
#include "cam_priv.h"

extern struct LCV_CAM cam; 

/*! @brief Determine whether an integer number is even. */
#define IS_EVEN(x)         (!((x) & 0x1))

LCV_ERR LCVCamSetShutterWidth(const uint32 usecs)
{
    uint32          nPixelClks;
    uint16          shutterWidth;
    LCV_ERR         err;
    
    /* Perform a conversion from usecs to number of rows
     * with rounding. */
    nPixelClks = (usecs*(CAM_PIX_CLK/1000000));
    /* Rounding division. */
    shutterWidth = (uint16)
        ((nPixelClks + cam.curCamRowClks/2)/cam.curCamRowClks);
    
    err = LCVCamSetRegisterValue(CAM_REG_SHUTTER_WIDTH, 
            (uint16)shutterWidth);
    if(err == SUCCESS)
    {
        cam.curExpTime = usecs;
    }
    return err;
}

LCV_ERR LCVCamGetShutterWidth(uint32 * pResult)
{
    uint16          shutterWidth;
    LCV_ERR         err;
    
    /* Input validation */
    if(unlikely(pResult == NULL))
    {
        return -EINVALID_PARAMETER;
    }
    
    err = LCVCamGetRegisterValue(CAM_REG_SHUTTER_WIDTH, 
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

LCV_ERR LCVCamGetAreaOfInterest(uint16 *pLowX,
        uint16 *pLowY,
        uint16 *pWidth,
        uint16 *pHeight)
{
    LCV_ERR err;
    
    if((err = LCVCamGetRegisterValue(CAM_REG_COL_START, pLowX)) ||
            (err = LCVCamGetRegisterValue(CAM_REG_ROW_START, pLowY)) ||
            (err = LCVCamGetRegisterValue(CAM_REG_WIN_WIDTH, pWidth)) ||
            (err = LCVCamGetRegisterValue(CAM_REG_WIN_HEIGHT, pHeight)))
    {
        LCVLog(ERROR, "%s: Error retrieving area of interest from "
                "camera!\n", __func__);
        return err;
    }
    
#ifdef LCV_TARGET
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

LCV_ERR LCVCamSetBlackLevelOffset(const uint16 offset)
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
    LCVCamSetRegisterValue( CAM_REG_ROW_NOISE_CONST , reg); 
    
    return SUCCESS;
}

LCV_ERR LCVCamGetBlackLevelOffset(uint16 *pOffset)
{
    uint16 reg;
    
    LCVCamGetRegisterValue( CAM_REG_ROW_NOISE_CONST , &reg);
    *pOffset = reg >> 2;
    
    return SUCCESS;
}

LCV_ERR LCVCamCreateMultiBuffer(const uint8 multiBufferDepth,
        const uint8 bufferIDs[])
{
    int i;
    struct frame_buffer * pFB;

    /* Input validation */
    if((multiBufferDepth < 2) ||
                    (multiBufferDepth > MAX_NR_FRAME_BUFFERS))
    {
        LCVLog(ERROR, "%s(%u, 0x%x): Invalid Parameter!\n", 
                __func__, multiBufferDepth, bufferIDs);
        return -EINVALID_PARAMETER;
    }
    for(i = 0; i < multiBufferDepth; i++)
    {
        if(bufferIDs[i] > MAX_NR_FRAME_BUFFERS)
        {
            LCVLog(ERROR, "%s(%u, 0x%x): Invalid Parameter!\n", 
                    __func__, multiBufferDepth, bufferIDs);
            return -EINVALID_PARAMETER;
        }
        pFB = &cam.fbufs[bufferIDs[i]];
        if(pFB->data == NULL)
        {
            LCVLog(ERROR, "%s: \
                    Invalid frame buffer in multibuffer.\n",
                    __func__);
            return -EINVALID_PARAMETER;
        }
    }

    /* Create the multi buffer */
    return LCVCamMultiBufferCreate(&cam.multiBuffer,
            multiBufferDepth,
            bufferIDs);
}

LCV_ERR LCVCamDeleteMultiBuffer()
{
    return LCVCamMultiBufferDestroy(&cam.multiBuffer);
}

LCV_ERR LCVCamSetupPerspective(const enum EnLcvCamPerspective perspective)
{
    uint16  reg;
    bool    rowFlip, colFlip;

    switch(perspective)
    {
    case LCV_CAM_PERSPECTIVE_DEFAULT:
        rowFlip = FALSE;
        colFlip = FALSE;
        break;
    case LCV_CAM_PERSPECTIVE_HORIZONTAL_MIRROR:
        rowFlip = FALSE;
        colFlip = TRUE;        
        break;
    case LCV_CAM_PERSPECTIVE_VERTICAL_MIRROR:
        rowFlip = TRUE;
        colFlip = FALSE;        
        break;
    case LCV_CAM_PERSPECTIVE_180DEG_ROTATE:
        rowFlip = TRUE;
        colFlip = TRUE;        
        break;
    default:
        return EINVALID_PARAMETER;
    }
    
#ifdef TARGET_TYPE_LCV_IND
    /* This board has a 180 deg rotated sensor related to the 
     * board bottom side */   
    rowFlip = !rowFlip;
    colFlip = !colFlip;
#endif /* TARGET_TYPE_LCV_IND */       
            
    /* Modify affected register fields */
    LCVCamGetRegisterValue( CAM_REG_READ_MODE, &reg);
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
    LCVCamSetRegisterValue( CAM_REG_READ_MODE, reg);        
    
    /* Store flip information in module */
    cam.flipHorizontal = colFlip;
    cam.flipVertical = rowFlip;
    
    return SUCCESS;
}

LCV_ERR PerspectiveCfgStr2Enum(const char *str, enum EnLcvCamPerspective *per )
{
    LCV_ERR err = SUCCESS;
    
    if(0 == strcmp(str, LCV_CAM_PERSPECTIVE_CFG_STR_DEFAULT) )
    {        
        *per = LCV_CAM_PERSPECTIVE_DEFAULT;
    } else {           
        if(0 == strcmp(str, LCV_CAM_PERSPECTIVE_CFG_STR_HORIZONTAL_MIRROR) )
        {
            *per = LCV_CAM_PERSPECTIVE_HORIZONTAL_MIRROR;
        } else {
            if(0 == strcmp(str, LCV_CAM_PERSPECTIVE_CFG_STR_VERTICAL_MIRROR) )
            {
                *per = LCV_CAM_PERSPECTIVE_VERTICAL_MIRROR;
            } else {
                if(0 == strcmp(str, LCV_CAM_PERSPECTIVE_CFG_STR_180DEG_ROTATE) )
                {               
                    *per = LCV_CAM_PERSPECTIVE_180DEG_ROTATE;
                } else {
                    err = EINVALID_PARAMETER;
                    *per = LCV_CAM_PERSPECTIVE_DEFAULT;
                }                
            }
        }
    }
    return err;
}

LCV_ERR LCVCamPresetRegs()
{    
    LCV_ERR err;
    /* Reset frame capture and AGC/Exposure logic.
     * Registers are _not_ set to default as done on power cycle.  */
    err = LCVCamSetRegisterValue( CAM_REG_RESET, 3);
    
    /* Snapshot mode; simultaneous readout */
    err |= LCVCamSetRegisterValue( CAM_REG_CHIP_CONTROL, 0x398);
    
    /* Define color type and operation mode */
    #ifdef TARGET_TYPE_LCV_IND  
    /* monochrone, linear */
    err |= LCVCamSetRegisterValue( CAM_REG_PIXEL_OP_MODE, 0x0011); 
    #endif /*TARGET_TYPE_LCV_IND*/    
    #ifdef TARGET_TYPE_LCV 
    /* color, linear */
    err |= LCVCamSetRegisterValue( CAM_REG_PIXEL_OP_MODE, 0x0015); 
    #endif /*TARGET_TYPE_LCV*/       
    
    /* This register is actually marked as reserved in the datasheet but a Micron 
     * representative stated that 0x3d5 is the "optimal" value for this register.
     * It seems to help brightness a bit. The default value is 0x1d1 */
    err |= LCVCamSetRegisterValue( CAM_REG_RESERVED_0x20, 0x3d5);    
    
    /* Disable AGC/AEC */
    err |= LCVCamSetRegisterValue( CAM_REG_AEC_AGC_ENA, 0x0);
   
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Unable to set camera registers! (%d)\n",
                __func__, err);
    }
    
    /* Apply row noise offset. This allows to push the black image histogram 
     * fully above zero for proper FPN correction. */ 
    err = LCVCamSetBlackLevelOffset( CAM_BLACKLEVEL);
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Unable to set black level! (%d)\n",
                __func__, err);
    }
    
    /* Apply default exposure time */
    err = LCVCamSetShutterWidth( CAM_EXPOSURE);
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Unable to set shutter width! (%d)\n",
                __func__, err);
    }
    
    /* Apply default (max) aera-of-interest */
    err = LCVCamSetAreaOfInterest(0,0,0,0);
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Unable to set area of interest! (%d)\n",
                __func__, err);
    }
        
    /* Set default camera - scene perspective relation */
    err = LCVCamSetupPerspective(LCV_CAM_PERSPECTIVE_DEFAULT);
    if(err != SUCCESS)
    {
        LCVLog(ERROR, "%s: Unable to set up camera perspective! (%d)\n",
                __func__, err);
    }
    
    return SUCCESS;
}

#ifdef TARGET_TYPE_LCV
LCV_ERR LCVCamGetBayerOrder(enum EnBayerOrder *pBayerOrderFirstRow,
							const uint16 xPos,
							const uint16 yPos)
{
	bool bFirstPixIsGreen;
	bool bFirstRowIsRed;
	
	/* Input validation */
	if(unlikely(pBayerOrderFirstRow == NULL) ||
		unlikely(xPos >= LCV_CAM_MAX_IMAGE_WIDTH) || 
		unlikely(yPos >= LCV_CAM_MAX_IMAGE_HEIGHT))
	{
		return -EINVALID_PARAMETER;
	}
	
	/* The Adaptina mt9v032 sensor will automatically adjust for column and
	 * row mirroring by means of a dummy column and read, i.e. the color order
	 * will always be RGRG by default.*/
	bFirstRowIsRed = TRUE;
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
LCV_ERR LCVCamGetBayerOrder(enum EnBayerOrder *pBayerOrderFirstRow,
							const uint16 xPos,
							const uint16 yPos)
{
	return -ENO_COLOR_SENSOR;
}
#endif /* TARGET_TYPE_LCV */
