/*! @file cam_host.c
 * @brief Camera module implementation for host
 *  
 * Simulation of the OSC-specific hardware featuring 
 * a Micron MT9V032 CMOS image sensor.
 * 
 */
#include "oscar_intern.h"
#include "cam_pub.h"
#include "cam_priv.h"
#include <unistd.h>
#include <stdlib.h>

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY cam_deps[] = {
        {"log", OscLogCreate, OscLogDestroy},
        {"frd", OscFrdCreate, OscFrdDestroy},
        {"bmp", OscBmpCreate, OscBmpDestroy}
};

struct OSC_CAM cam; /*!< @brief The camera module singelton instance */

/*! @brief Representation of a mt9v032 CMOS-Sensor register for 
 * simulation */
struct CAM_REGISTER
{
    uint16 addr; /*!< @brief The address of the register */
    uint16 value; /*!< @brief The value of the register */
    char name[50]; /*!< @brief String describing the register */
};

/*! @brief The default values for the mt9v032 CMOS-Sensor registers */
const struct CAM_REGISTER default_reg_values[] =
{
{ 0x00, 0x1313, "Chip Version" },
{ 0x01, 0x0001, "Column Start" },
{ 0x02, 0x0004, "Row Start" },
{ 0x03, 0x01e0, "Window Height" },
{ 0x04, 0x02f0, "Window Width" },
{ 0x05, 0x002b, "Horizontal Blanking" },
{ 0x06, 0x002d, "Vertical Blanking" },
{ 0x07, 0x0298, "Chip Control" },
{ 0x08, 0x01bb, "Shutter Width 1" },
{ 0x09, 0x01d9, "Shutter Width 2" },
{ 0x0A, 0x0164, "Shutter Width Ctrl" },
{ 0x0B, 0x05dc, "Total Shutter Width" },
{ 0x0C, 0x0000, "Reset" },
{ 0x0D, 0x0320, "Read Mode" },
{ 0x0E, 0x0000, "Monitor Mode" },
{ 0x0F, 0x0015, "Pixel Operation Mode" },
{ 0x1B, 0x0000, "LED_OUT Ctrl" },
{ 0x1C, 0x0002, "ADC Mode Control" },
{ 0x20, 0x03d5, "Reserved" },
{ 0x2C, 0x0004, "VREF_ADC Control" },
{ 0x31, 0x001d, "V1" },
{ 0x32, 0x0018, "V2" },
{ 0x33, 0x0015, "V3" },
{ 0x34, 0x0004, "V4" },
{ 0x35, 0x0010, "Analog Gain (16-64)" },
{ 0x36, 0x0040, "Max Analog Gain" },
{ 0x42, 0x0002, "Frame Dark Average" },
{ 0x46, 0x231d, "Dark Avg Thresholds" },
{ 0x47, 0x8080, "BL Calib Control" },
{ 0x48, 0x007f, "BL Calibration Value" },
{ 0x4C, 0x0002, "BL Calib Step Size" },
{ 0x70, 0x0014, "Row Noise Corr Ctrl 1" },
{ 0x72, 0x002a, "Row Noise Constant" },
{ 0x73, 0x02f7, "Row Noise Corr Ctrl 2" },
{ 0x74, 0x0000, "Pixclk, FV, LV" },
{ 0x7F, 0x0000, "Digital Test Pattern" },
/*		{0x80, 0x00f4, "Tile Weight/Gain X0_Y0"}, No access in driver
 {0x81, 0x00f4, "Tile Weight/Gain X1_Y0"},
 {0x82, 0x00f4, "Tile Weight/Gain X2_Y0"},
 {0x83, 0x00f4, "Tile Weight/Gain X3_Y0"},
 {0x84, 0x00f4, "Tile Weight/Gain X4_Y0"},
 {0x85, 0x00f4, "Tile Weight/Gain X0_Y1"},
 {0x86, 0x00f4, "Tile Weight/Gain X1_Y1"},
 {0x87, 0x00f4, "Tile Weight/Gain X2_Y1"},
 {0x88, 0x00f4, "Tile Weight/Gain X3_Y1"},
 {0x89, 0x00f4, "Tile Weight/Gain X4_Y1"},
 {0x8A, 0x00f4, "Tile Weight/Gain X0_Y2"},
 {0x8B, 0x00f4, "Tile Weight/Gain X1_Y2"},
 {0x8C, 0x00f4, "Tile Weight/Gain X2_Y2"},
 {0x8D, 0x00f4, "Tile Weight/Gain X3_Y2"},
 {0x8E, 0x00f4, "Tile Weight/Gain X4_Y2"},
 {0x8F, 0x00f4, "Tile Weight/Gain X0_Y3"},
 {0x90, 0x00f4, "Tile Weight/Gain X1_Y3"},
 {0x91, 0x00f4, "Tile Weight/Gain X2_Y3"},
 {0x92, 0x00f4, "Tile Weight/Gain X3_Y3"},
 {0x93, 0x00f4, "Tile Weight/Gain X4_Y3"},
 {0x94, 0x00f4, "Tile Weight/Gain X0_Y4"},
 {0x95, 0x00f4, "Tile Weight/Gain X1_Y4"},
 {0x96, 0x00f4, "Tile Weight/Gain X3_Y4"},
 {0x98, 0x00f4, "Tile Weight/Gain X4_Y4"},
 {0x99, 0x0000, "Tile Coord. X 0/5"},
 {0x9A, 0x0096, "Tile Coord. X 1/5"},
 {0x9B, 0x012c, "Tile Coord. X 2/5"},
 {0x9C, 0x01c2, "Tile Coord. X 3/5"},
 {0x9D, 0x0258, "Tile Coord. X 4/5"},
 {0x9E, 0x02f0, "Tile Coord. X 5/5"},
 {0x9F, 0x0000, "Tile Coord. Y 0/5"},
 {0xA0, 0x0060, "Tile Coord. Y 1/5"},
 {0xA1, 0x00c0, "Tile Coord. Y 2/5"},
 {0xA2, 0x0120, "Tile Coord. Y 3/5"},
 {0xA3, 0x0180, "Tile Coord. Y 4/5"},
 {0xA4, 0x01e0, "Tile Coord. Y 5/5"},*/
{ 0XA5, 0x003a, "AEC/AGC Desired Bin" },
{ 0xA6, 0x0002, "AEC Update Frequency" },
{ 0xA8, 0x0000, "AEC LPF" },
{ 0xA9, 0x0002, "AGC Update Frequency" },
{ 0xAB, 0x0002, "AGC LPF" },
{ 0xAF, 0x0000, "AEC/AGC Enable" },
{ 0xB0, 0xabe0, "AEC/AGC Pix Count" },
/*		{0xB1, 0x0002, "LVDS Master Ctrl"},
 {0xB2, 0x0010, "LVDS Shift Clk Ctrl"},
 {0xB3, 0x0010, "LVDS Data Ctrl"}, */
{ 0xB4, 0x0000, "Data Stream Latency" },
/*		{0xB5, 0x0000, "LVDS Internal Sync"},
 {0xB6, 0x0000, "LVDS Payload Control"},
 {0xB7, 0x0000, "Stereoscop. Error Ctrl"},
 {0xB8, 0x0000, "Stereoscop. Error Flag"},
 {0xB9, 0x0000, "LVDS Data Output"}, */
{ 0xBA, 0x0010, "AGC Gain Output" },
{ 0XBB, 0x05dc, "AEC Gain Output" },
{ 0xBC, 0x003f, "AGC/AEC Current Bin" },
{ 0xBD, 0x01e0, "Maximum Shutter Width" },
{ 0xBE, 0x0014, "AGC/AEC Bin Difference Threshold" },
{ 0xBF, 0x0016, "Field Blank" },
{ 0xC0, 0x000a, "Mon Mode Capture Ctrl" },
{ 0xC1, 0x015f, "Temperature" },
{ 0xC2, 0x0840, "Analog Controls" },
{ 0xC3, 0x0000, "NTSC FV & LV Ctrl" },
{ 0xC4, 0x4416, "NTSC Horiz Blank Ctrl" },
{ 0xC5, 0x4421, "NTSC Vert Blank Ctrl" },
{ 0xF0, 0x2100, "Bytewise Addr" },
{ 0xFE, 0xbeef, "Register Lock" },
{ 0xFF, 0x1313, "Chip Version" }, 
};


/*********************************************************************//*!
 * @brief Host only: Initialize simulated camera registers with defaults
 *//*********************************************************************/
static void OscCamResetRegs()
{
    int i;
    uint32 len;

    /* Number of entries in the default value array */
    len = sizeof(default_reg_values)/sizeof(struct CAM_REGISTER);
    if (len > NUM_CAM_REGS)
    {
        len = NUM_CAM_REGS;
    }

    for (i = 0; i < len; i++)
    {
        cam.regs[i].addr = default_reg_values[i].addr;
        cam.regs[i].value = default_reg_values[i].value;
    }
}

/*********************************************************************//*!
 * @brief Host only: Find the register by the specified register address
 * 
 * @param addr Address of the desired register
 * @return Pointer to the structure describing the register or NULL
 *//*********************************************************************/
static struct reg_info * OscCamFindReg(const uint32 addr)
{
    int i;
    for (i = 0; i < NUM_CAM_REGS; i++)
    {
        if (cam.regs[i].addr == addr)
        {
            return &cam.regs[i];
        }
    }
    return NULL;
}

/*********************************************************************//*!
 * @brief Host only: Crop a picture to the specified window.
 * 
 * The contents of the supplied OSC_PICTURE structure are cropped and
 * written to pDstBuffer. pPic is not changed.
 * 
 * Host only.
 * 
 * @param pDstBuffer The destination buffer where the cropped image
 * is written to.
 * @param dstBufferSize Size of above destination buffer.
 * @param pPic Picture to be cropped.
 * @param pCropWin Window to crop the picture to.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscCamCropPicture(uint8* pDstBuffer,
        const uint32 dstBufferSize,
        const struct OSC_PICTURE *pPic,
        const struct capture_window *pCropWin)
{
    uint8       *pTSrc, *pTDst;
    uint32      croppedSize;
    uint16      colorDepth, bytesPerPixel;
    uint32      y;
    uint32      lowY, highY;
    
    /* Input validation */
    if((pPic == NULL) || (pPic->data == NULL) || (pCropWin == NULL) ||
            (pDstBuffer == NULL) || (dstBufferSize == 0))
    {
        OscLog(ERROR, "%s(0x%x, %u, 0x%x, 0x%x): Invalid parameter!\n",
                __func__, pDstBuffer, dstBufferSize, pPic, pCropWin);
        return -EINVALID_PARAMETER;
    }
    if((pPic->width < (pCropWin->col_off + pCropWin->width)) ||
            pPic->height < (pCropWin->row_off + pCropWin->height))
    {
        OscLog(ERROR, 
                "%s: Unable to crop image (%dx%d) to (%dx%d @ %d/%d).\n",
                __func__, pPic->width, pPic->height,
                pCropWin->width, pCropWin->height,
                pCropWin->col_off, pCropWin->row_off);
        return -EPICTURE_TOO_SMALL;
    }
    
    /* Allocate a temporary buffer for the cropped image */
    colorDepth = OSC_PICTURE_TYPE_COLOR_DEPTH(pPic->type);
    bytesPerPixel = colorDepth / 8;
    croppedSize = pCropWin->width *  pCropWin->height * bytesPerPixel;
    
    if(croppedSize > dstBufferSize)
    {
        OscLog(ERROR, "%s: Specified destination Buffer too small. \
                (%d < %d)\n", __func__, dstBufferSize, croppedSize);
        return -EBUFFER_TOO_SMALL;
    }
    
    /* The crop rectangle */
    lowY = pCropWin->row_off;
    highY = pCropWin->row_off + pCropWin->height;
    
    /* Copy loop to transfer the pixels from the supplied picture
     * to the cropped buffer */
    pTDst = pDstBuffer;
    pTSrc = (uint8*)pPic->data;
    /* X offset */
    pTSrc += pCropWin->col_off*bytesPerPixel; 
    /* Y offset */
    pTSrc += pCropWin->row_off * pPic->width * bytesPerPixel;
    for(y = lowY; y < highY; y++)
    {
        memcpy(pTDst, 
                pTSrc, 
                bytesPerPixel * pCropWin->width);
        
        pTDst += pCropWin->width * bytesPerPixel;
        pTSrc += pPic->width * bytesPerPixel;
    }
    
    return SUCCESS;
}

OSC_ERR OscCamCreate(void *hFw)
{
    struct OSC_FRAMEWORK    *pFw;
    OSC_ERR                 err;
    uint16                  dummy;
    
    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->cam.useCnt != 0)
    {
        pFw->cam.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }  

    /* Load the module cam_deps of this module. */
    err = OSCLoadDependencies(pFw, 
            cam_deps, 
            sizeof(cam_deps)/sizeof(struct OSC_DEPENDENCY));
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load cam_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
        
    memset(&cam, 0, sizeof(struct OSC_CAM));
    
    /* Initialize camera registers */
    OscCamResetRegs();
    
    /* Create the file name reader if the config file exists. Otherwise
     * the application will have to set the file name reader later over
     * OscFrdSetFileNameReader(). */
    if(access(FILENAME_READER_CONFIG_FILE, F_OK) == 0)
    {
        OscFrdCreateReader(&cam.hFNReader, FILENAME_READER_CONFIG_FILE);
    }
    
    cam.lastValidID = OSC_CAM_INVALID_BUFFER_ID;
    
    /* Read the current camera register values and build a model of the
     * current state from them. */
    err = OscCamGetShutterWidth(&cam.curExpTime);
    err |= OscCamGetRegisterValue(CAM_REG_HORIZ_BLANK, 
            &cam.curHorizBlank);
    /* Read back the area of interest to implicitely update 
     * cam.curCamRowClks. */
    err |= OscCamGetAreaOfInterest(&dummy,
                &dummy,
                &dummy,
                &dummy);
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to read current settings from "
                "camera!\n",
                __func__);
    }
    
    /* Increment the use count */
    pFw->cam.hHandle = (void*)&cam;
    pFw->cam.useCnt++; 
    
    return SUCCESS;
}

void OscCamDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
            
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->cam.useCnt--;
    if(pFw->cam.useCnt > 0)
    {
        return;
    }
        
    OSCUnloadDependencies(pFw, 
            cam_deps, 
            sizeof(cam_deps)/sizeof(struct OSC_DEPENDENCY));
    
    memset(&cam, 0, sizeof(struct OSC_CAM));
}

OSC_ERR OscCamSetFileNameReader(void* hReaderHandle)
{
    /* Input validation. */
    if(hReaderHandle == NULL)
    {
        return -EINVALID_PARAMETER;
    }
    
    if(cam.hFNReader != NULL)
    {
        OscLog(WARN, "%s WARNING: Replacing file name reader already "
                "associated with camera module!\n", __func__);
    }
    
    cam.hFNReader = hReaderHandle;
    
    return SUCCESS;
}

OSC_ERR OscCamSetAreaOfInterest(const uint16 lowX,
        const uint16 lowY,
        const uint16 width,
        const uint16 height)
{
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

    if((width == 0) || (height == 0))
    {
        /* Reset the window to the default. */
        cam.capWin.col_off = 0;
        cam.capWin.row_off = 0;
        cam.capWin.width = MAX_IMAGE_WIDTH;
        cam.capWin.height = MAX_IMAGE_HEIGHT;
    } else {
        cam.capWin.col_off = lowX;
        cam.capWin.row_off = lowY;
        cam.capWin.width = width;
        cam.capWin.height = height;
    }
    
    /* The row time of the camera consists of the pixel readout time
     * for the width of the row plus the horizontal blanking. The time
     * has a lower bound. */
    cam.curCamRowClks = cam.capWin.width + cam.curHorizBlank;
    if(cam.curCamRowClks < CAM_MIN_ROW_CLKS)
    {
        cam.curCamRowClks = CAM_MIN_ROW_CLKS;
    }
        
    /* Recalculate and set the shutter width, since it changes when
     * changing the area of interest. */
    OscCamSetShutterWidth(cam.curExpTime);
        
    OscLog(DEBUG, "Area of interest set to %dx%d at %d/%d.\n",
            cam.capWin.width,
            cam.capWin.height,
            cam.capWin.col_off,
            cam.capWin.row_off);
    return SUCCESS;
}

OSC_ERR OscCamSetRegisterValue(const uint32 reg, const uint16 value)
{
    struct reg_info * pReg;

    pReg = OscCamFindReg(reg);
    if(pReg == NULL)
    {
        return -EINVALID_PARAMETER;
    }
    pReg->value = value;

    return 0;
}

OSC_ERR OscCamGetRegisterValue(const uint32 reg, uint16 *pResult)
{
    struct reg_info * pReg;

    if(pResult == NULL)
    {
        return -EINVALID_PARAMETER;
    }

    pReg = OscCamFindReg(reg);
    if(pReg == NULL)
    {
        return -EINVALID_PARAMETER;
    }
    *pResult = pReg->value;

    return SUCCESS;
}

OSC_ERR OscCamSetFrameBuffer(const uint8 fbID,
        const uint32 uSize,
        const void * pData,
        const int bCached)
{
    struct frame_buffer fb;
    int i;

    /* Input validation */
    if(fbID > MAX_NR_FRAME_BUFFERS ||
            (pData == NULL && uSize != 0))
    {
        OscLog(ERROR,
                "%s(%d, %d, 0x%x, %d): \
				Invalid parameter.\n",
                __func__, fbID, uSize, pData, bCached);
        return -EINVALID_PARAMETER;
    }

    if(pData == NULL)
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
                        "%s: Deleting frame buffer %d being part of \
	                    a multi buffer!.\n", 
	                    __func__, fbID);
                return -ECANNOT_DELETE;
            }
        }
        if(cam.fbStat[fbID] == STATUS_UNITIALIZED)
        {
            OscLog(WARN, 
                    "%s: Deleting an unitialized frame buffer (%d)!\n",
                    __func__, fbID);
        }
        cam.fbStat[fbID] = STATUS_UNITIALIZED;
        memset(&cam.fbufs[fbID], 0, sizeof(struct frame_buffer));
        return 0;
    }

    fb.id = fbID;
    fb.size = uSize;
    fb.data = (void *)pData;
    fb.flags = 0;
    if(bCached)
    {
        fb.flags = FB_FLAG_CACHED;
    }

    /* Make sure we don't replace an initialized frame buffer */
    if((cam.fbufs[fbID].data != NULL) ||
            cam.fbStat[fbID] != STATUS_UNITIALIZED)
    {
        OscLog(ERROR, "%s: Unable to set frame buffer %d -> busy.\n",
                __func__, fbID);
        return -EFRAME_BUFFER_BUSY;
    }

    /* Store the info for later */
    memcpy(&cam.fbufs[fb.id], &fb, sizeof(struct frame_buffer));
    cam.fbStat[fb.id] = STATUS_READY;

    return SUCCESS;
}

OSC_ERR OscCamSetupCapture(uint8 fbID,
        const enum EnOscCamTriggerMode tMode)
{
    uint8           fb;
    int             i;
  
    /* If the caller is using automatic multibuffer management,
     * get the correct frame buffer. */
    fb = fbID;
    if(fbID == OSC_CAM_MULTI_BUFFER)
    {
        /* Get the buffer ID to write to next */
        fb = OscCamMultiBufferGetCapBuf(&cam.multiBuffer);
    }
    
    if(fb > MAX_NR_FRAME_BUFFERS)
    {
        OscLog(ERROR, "%s(%u, %d): Invalid parameter!\n",
                __func__, fbID, tMode);
        return -EINVALID_PARAMETER;
    }

    if(unlikely(cam.capWin.width == 0 || cam.capWin.height == 0))
    {
        OscLog(ERROR, "%s: No area of interest set!\n",
                __func__);
        return -ENO_AREA_OF_INTEREST_SET;
    }
    
    for(i = 0; i < MAX_NR_FRAME_BUFFERS; i++)
    {
        if((cam.fbStat[i] == STATUS_CAPTURING_EXT_TRIGGER) ||
                (cam.fbStat[i] == STATUS_CAPTURING_MAN_TRIGGER))
        {
            /* Can only capture one picture at a time but we don't
             * know how 'long ago' the last capture was. */
            OscLog(WARN, 
                    "%s(%d, %d):" \
                    "Already capturing to different frame buffer (%d). "\
                    "This may not be possible on the target\n",
                    __func__, fb, tMode, i);
        }
    }
    
    /* Distinguish the trigger modes */
    switch(tMode)
    {
    case OSC_CAM_TRIGGER_MODE_EXTERNAL:
        cam.fbStat[fb] = STATUS_CAPTURING_EXT_TRIGGER;
        break;
    case OSC_CAM_TRIGGER_MODE_MANUAL:
        cam.fbStat[fb] = STATUS_CAPTURING_MAN_TRIGGER;
        break;
    default:
        return -EINVALID_PARAMETER;
    }
     
    OscLog(DEBUG, 
            "%s: Setting up capture of %ux%d picture " \
            "on frame buffer %d.\n",
            __func__, cam.capWin.width, cam.capWin.height, fb);
    
    /* This is a void-operation in the host implementation, since we
     * can read a picture from file at any time. */
    
    if(fbID == OSC_CAM_MULTI_BUFFER)
    {
        /* Allow the multi buffer to update is status according to this
         * successful capture. */
        OscCamMultiBufferCapture(&cam.multiBuffer);
    }
    
    /* Save the capture window currently configured since that will
     * be the one used later when really reading the image */
    memcpy(&cam.lastCapWin, &cam.capWin, sizeof(struct capture_window));

    
    return SUCCESS;
}

OSC_ERR OscCamCancelCapture()
{
    int i;
    for(i = 0; i < MAX_NR_FRAME_BUFFERS; i++)
    {
        if((cam.fbStat[i] == STATUS_CAPTURING_EXT_TRIGGER) ||
                (cam.fbStat[i] == STATUS_CAPTURING_MAN_TRIGGER))
        {
            /* Assume worst case => corrupted */
            cam.fbStat[i] = STATUS_CORRUPTED;
            return SUCCESS;
        }
    }
    
    OscLog(WARN, "%s: Cancel request when no picture \
            transfer to cancel.\n",
            __func__);
    return -ENOTHING_TO_ABORT;
}

OSC_ERR OscCamReadPicture(const uint8 fbID,
        void ** ppPic,
        const uint16 maxAge,
        const uint16 timeout)
{
    OSC_ERR             err = SUCCESS;
    uint8               fb;
    struct OSC_PICTURE  pic;
    char                strPicFileName[256];
    

    if(unlikely(cam.hFNReader == NULL))
    {
        OscLog(ERROR, "%s: No filename reader set!\n", __func__);
        return -EDEVICE;
    }
    /* If the caller is using automatic multibuffer management,
     * get the correct frame buffer. */
    fb = fbID;
    if(fb == OSC_CAM_MULTI_BUFFER)
    {
        /* Get the correct buffer ID */
        fb = OscCamMultiBufferGetSyncBuf(&cam.multiBuffer);
        if(fb == OSC_CAM_INVALID_BUFFER_ID)
        {
            OscLog(ERROR, "%s: No capture started!\n",
                    __func__);
            return -ENO_CAPTURE_STARTED;
        }
    }
    
    /* Input validation */
    if((ppPic == NULL) || (fb > MAX_NR_FRAME_BUFFERS) ||
            (cam.fbufs[fb].data == NULL))
    {
        OscLog(ERROR, "%s(%u, 0x%x, %u, %u): Invalid parameter!\n",
                __func__, fbID, ppPic, maxAge, timeout);
        return -EINVALID_PARAMETER;
    }
    
    *ppPic = NULL; /* Precaution */

    if((cam.fbStat[fb] != STATUS_CAPTURING_EXT_TRIGGER) &&
            (cam.fbStat[fb] != STATUS_CAPTURING_MAN_TRIGGER))
    {
        /* There is no scheduled capture */
        OscLog(ERROR, "%s: No capture started on frame buffer %d!\n",
                __func__, fb);
        return -ENO_CAPTURE_STARTED;
    }
    
    OscLog(DEBUG, 
            "%s(%u, 0x%x, %u, %u): Syncing capture on frame buffer %d.\n",
            __func__, fbID, ppPic, maxAge, timeout, fb);
    
    /* Get the current test image file name from the file name reader 
     * module */
    OscFrdGetCurrentFileName(cam.hFNReader, 
            strPicFileName);
    
    /* We have no assumptions about the picture format but let 
     * everything be filled and allocated by the loader routine */
    memset(&pic, 0, sizeof(struct OSC_PICTURE));
    
    /* Read the file */
    err = OscBmpRead(&pic, strPicFileName);
    if(err != 0)
    {
        OscLog(ERROR, "%s: Unable to read test image (%s). Err: %d.\n", 
                __func__, strPicFileName, err);
        return -EDEVICE;
    }
    
    /* Crop the picture to the window set by the application.
     * We use the window at the time of the call to OscCamSetupCapture()
     * to emulate the behavior of the target implementation. */
    err = OscCamCropPicture(cam.fbufs[fb].data, 
            cam.fbufs[fb].size,
            &pic, 
            &cam.lastCapWin);    
    if(err != 0)
    {
        OscLog(ERROR, "%s: Unable to crop test image (%s). Err: %d.\n", 
                __func__, strPicFileName, err);
        return -EDEVICE;
    }
    
    /* Free the picture structure data again; it was allocated in the 
     * OscBmpRead routine. */
    free(pic.data);

    *ppPic = cam.fbufs[fb].data;
    cam.fbStat[fb] = STATUS_VALID;
    
    /* The operation was successful */
    
    if(fbID == OSC_CAM_MULTI_BUFFER)
    {
        /* Allow the multi buffer to update is status according to this
         * successful read. */
        OscCamMultiBufferSync(&cam.multiBuffer);
    }
    if(memcmp(&cam.capWin, &cam.lastCapWin, sizeof(struct capture_window)))
    {
        /* Reset the field storing the ID of the frame buffer with the 
         * lastvalid picture since now the format has changed and the 
         * application expects the new format. */
        cam.lastValidID = OSC_CAM_INVALID_BUFFER_ID;      
    } else {   
        /* Mark this frame buffer as the one containing the 
         * latest picture. */
        cam.lastValidID = fb;
    }
    
    return err;
}

OSC_ERR OscCamReadLatestPicture(uint8 ** ppPic)
{    
    /* Input Validation */
    if(ppPic == NULL)
    {
        return -EINVALID_PARAMETER;
    }
    
    *ppPic = NULL;  /* Precaution */
    
    if(cam.lastValidID == OSC_CAM_INVALID_BUFFER_ID)
    {
        return -ENO_MATCHING_PICTURE;
    }
    
    if(cam.fbufs[cam.lastValidID].data == NULL)
    {
        cam.lastValidID = OSC_CAM_INVALID_BUFFER_ID;
        return -ENO_MATCHING_PICTURE;
    }
    
    OscLog(DEBUG, 
            "%s(0x%x): Getting latest picture from frame" \
            "buffer %d.\n",
            __func__, ppPic,cam.lastValidID);
    
    *ppPic = cam.fbufs[cam.lastValidID].data;
    return SUCCESS;
}

OSC_ERR OscCamRegisterCorretionCallback( 
        int (*pCallback)(
                uint8 *pImg, 
                const uint16 lowX, 
                const uint16 lowY,
                const uint16 width, 
                const uint16 height))
{
    return SUCCESS;   
}

