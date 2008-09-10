/*! @file clb_target.c
 * @brief Calibration module implementation for target
 * 
 */

#include "oscar_types_target.h"

#include "clb_pub.h"
#include "clb_priv.h"
#include "sup/sup_pub.h"
#include "cam/cam_pub.h"
#include "oscar_intern.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h> 

/*! @brief Return the result of x/y round up to the next full integer. */
#define ROUND_UP_DIVISION(x, y) ((x + (y - 1))/y)

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY clb_deps[] = {
        {"log", OscLogCreate, OscLogDestroy},
		{"cam", OscCamCreate, OscCamDestroy}
};

/*! @brief Memory for the CLB object structure plus some reserve.
 * The actual structure will be manually cache-aligned within this 
 * memory. */
uint8 unalignedMem[sizeof(struct OSC_CLB) + CACHE_LINE_LEN];

/*! @brief This stores all variables needed by the algorithm. */
struct OSC_CLB *pClb;


/*********************************************************************//*!
 * @brief Manually align the CLB structure to a cache line since
 * the linker is not capable of doing that over a directive.
 *//*********************************************************************/
static void AlignMemory()
{
    uint32 temp = ROUND_UP_DIVISION((uint32)unalignedMem, CACHE_LINE_LEN);
    
    pClb = (struct OSC_CLB*)(temp * CACHE_LINE_LEN);
}

OSC_ERR OscClbCreate(void *hFw)
{
    struct OSC_FRAMEWORK    *pFw;
    OSC_ERR                 err;
    
    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->clb.useCnt != 0)
    {
        pFw->clb.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }  
	
    /* Load the module clb_deps of this module. */
    err = OscLoadDependencies(pFw, 
            clb_deps, 
            sizeof(clb_deps)/sizeof(struct OSC_DEPENDENCY));
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load clb_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    /* First, align the memory of the CLB object structure manually.
     * Only after this call can we use the pClb pointer and the object
     * structure. */
    AlignMemory();
         
    memset(pClb, 0, sizeof(struct OSC_CLB));
    pClb->calibSlope = OSC_CLB_CALIBRATE_OFF;
    pClb->bHotpixel = FALSE;

#if 0  /* set trial calib data and write to flash */
    memset(pClb->calibData.fpn, 0x00, MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT); // no offset 
    memset(pClb->calibData.prnu, 0x80, MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT);// factor 1
    pClb->calibData.nHotpixel = 0;
    memset(pClb->calibData.hotpixels, 0, pClb->calibData.nHotpixel*sizeof(struct VEC_2D));
    
    memset(pClb->calibData.fpn, 0xff, MAX_IMAGE_WIDTH*10); // no offset 
    memset(pClb->calibData.prnu + MAX_IMAGE_WIDTH*40, 0xff, MAX_IMAGE_WIDTH*20);// factor 1    
    memset(pClb->calibData.prnu + MAX_IMAGE_WIDTH*100, 0x00, MAX_IMAGE_WIDTH*20);// factor 1
    
    StoreCalibrationData( CALIB_FILE);    
#endif    
    
	/* Load calibration data from persistent memory */
    err = LoadCalibrationData( CALIB_FILE);
    
    /* Register correction function for camera module */
    if( err == SUCCESS)
    {
        OscCamRegisterCorretionCallback( OscClbApplyCorrection);
    }
    else
    {
        OscLog(WARN, "%s: Image calibration disabled due to error!"
                ,__func__);
    }
	
	/* Increment the use count */
    pFw->clb.hHandle = (void*)pClb;
    pFw->clb.useCnt++; 
	    
	return SUCCESS;
}

void OscClbDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
                
    pFw = (struct OSC_FRAMEWORK *)hFw; 

    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->clb.useCnt--;
    if(pFw->clb.useCnt > 0)
    {
        return;
    }
  
    OscUnloadDependencies(pFw, 
            clb_deps, 
            sizeof(clb_deps)/sizeof(struct OSC_DEPENDENCY));
    
	memset(pClb, 0, sizeof(struct OSC_CLB));
}


OSC_ERR OscClbSetupCalibrate(
        enum EnOscClbCalibrateSlope calibSlope,
        bool bHotpixel)
{   
    if( (calibSlope==OSC_CLB_CALIBRATE_FPN) 
        || (calibSlope==OSC_CLB_CALIBRATE_PRNU) )
    {
        OscLog(ERROR, "%s: Selected Calibration configuration not yet supported!\n",
                __func__);
        return EINVALID_PARAMETER;
    }
    pClb->calibSlope = calibSlope;
    pClb->bHotpixel = bHotpixel;       
    
    return SUCCESS;
}

OSC_ERR OscClbApplyCorrection( uint8 *pImg,
        const uint16 lowX, const uint16 lowY,
        const uint16 width, const uint16 height) 
{
    pClb->capWin.col_off = lowX;
    pClb->capWin.row_off = lowY;
    pClb->capWin.width = width;
    pClb->capWin.height = height;
    
    if( pClb->calibSlope == OSC_CLB_CALIBRATE_FPN_PRNU)
    {
        OscClbCorrectFpnPrnu( pImg);
    }
    if( pClb->bHotpixel)
    {
        OscClbCorrectHotpixel( pImg);
    }
    return SUCCESS;
}

OSC_ERR LoadCalibrationData(const char strCalibFN[])
{
    FILE    *pCalibF;
    uint32  n, idx, nElements;
    uint16  nHotpixel;
    struct  OSC_CLB_CALIBRATION_DATA *pCalib = &pClb->calibData;
    uint32  magicnumber;
    uint16  pack;
    uint16  size;
   
    pCalibF = fopen(strCalibFN, "rb");
    if(pCalibF == NULL)
    {
        OscLog(ERROR, "%s: Unable to open calibration data file (%s)! "
                "%s.\n", __func__, strCalibFN, strerror(errno));
        return -EUNABLE_TO_OPEN_FILE;
    }    
    
    /* Read in the magic word.*/
    n = fread(&magicnumber, sizeof(uint32), 1, pCalibF);
    if(n == 1)
    {
        if(magicnumber != CALIB_MAGIC)
        {
            goto fread_err;            
        }
    } else {
        goto fread_err;
    }
    
    /* Read width/height.*/
    n = fread(&size, sizeof(uint16), 1, pCalibF);
    if(n == 1)
    {
        if(size != MAX_IMAGE_WIDTH)
        {
            goto fread_err;            
        }
    } else {
        goto fread_err;
    }    
    n = fread(&size, sizeof(uint16), 1, pCalibF);
    if(n == 1)
    {
        if(size != MAX_IMAGE_HEIGHT)
        {
            goto fread_err;            
        }
    } else {
        goto fread_err;
    }     

    /* Read in FPN/PRNU section.
     *
     * Prnu memory is also used as temporary buffer for unpacking. The 
     * packed data junk is filled in by fread. Beginning with the first 
     * packed element this information is translated and stored to Fpn/Prnu.
     */ 
    nElements = (MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT);   
    n = fread(pCalib->prnu,
            sizeof(uint16),
            nElements,
            pCalibF);   
    if(n != nElements)
    {
        goto fread_err;
    }  
    
    for(idx=0; idx<nElements; idx++)
    {
        pack = pCalib->prnu[idx];
        pCalib->fpn[idx] = (pack & 0xf800) >> 11;
        pCalib->prnu[idx] = pack & 0x3ff;           
    }         
    
    /* Read in the hotpixel section. First extract the provided
     * number of pairs. */
    n = fread(&nHotpixel, sizeof(uint16), 1, pCalibF);
    if(n == 1)
    {
        if(nHotpixel > MAX_NR_HOTPIXEL)
        {
            goto fread_err;            
        }
        pCalib->nHotpixel = nHotpixel;
    } else {
        goto fread_err;
    }    
     
    n = fread(pCalib->hotpixels,
            sizeof(struct VEC_2D),
            nHotpixel,
            pCalibF);
    
    if(n != nHotpixel)
    {
        goto fread_err;
    }      
    
    fclose(pCalibF);
    OscLog(DEBUG, "%s: done.\n", __func__);
    return SUCCESS;    
    
fread_err:
    OscLog(WARN, "%s: File read error from file %s!\n",
            __func__, strCalibFN);
    return -EFILE_ERROR;    
}

#if 0
OSC_ERR StoreCalibrationData(const char strCalibFN[])
{
    FILE    *pCalibF;
    uint32  n, idx, nElements;
    struct  OSC_CLB_CALIBRATION_DATA *pCalib = &pClb->calibData;
    uint32  magicnumber;
    uint16  pack, dummy;
    uint16  size;
   
    pCalibF = fopen(strCalibFN, "wb");
    if(pCalibF == NULL)
    {
        OscLog(ERROR, "%s: Unable to open calibration data file (%s)! "
                "%s.\n", __func__, strCalibFN, strerror(errno));
        return -EUNABLE_TO_OPEN_FILE;
    }    
    
    /* Write the magic word.*/
    magicnumber = CALIB_MAGIC;
    n = fwrite(&magicnumber, sizeof(uint32), 1, pCalibF);
    if(n != 1)
    {
        goto fwrite_err;
    }
    
    /* Write width/height */
    size = MAX_IMAGE_WIDTH;
    n = fwrite(&size, sizeof(uint16), 1, pCalibF);
    if(n != 1)
    {
        goto fwrite_err;
    }    
    size = MAX_IMAGE_HEIGHT;    
    n = fwrite(&size, sizeof(uint16), 1, pCalibF);
    if(n != 1)
    {
        goto fwrite_err;
    }      
    
    /* Write out FPN/PRNU section 
     * 
     * Use Prnu as temporary buffer for packed format generation 
     * */    
    nElements = (MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT);
    for(idx=0; idx < nElements; idx++)
    {
        pack  = pCalib->fpn[idx] << 11;
        pCalib->fpn[idx] |= pack;    
    }
    
    n = fwrite(pCalib->prnu,
            sizeof(uint16),
            nElements,
            pCalibF);
    
    if(n != nElements)
    {
        goto fwrite_err;
    }      
    
    /* Write out the hotpixel section.  */
    nElements = pCalib->nHotpixel;
    n = fwrite(&nElements, sizeof(uint16), 1, pCalibF);
    if(n != 1)
    {
        goto fwrite_err;
    }    
     
    n = fwrite(pCalib->hotpixels,
            sizeof(struct VEC_2D),
            MAX_NR_HOTPIXEL,
            pCalibF);
    
    if(n != MAX_NR_HOTPIXEL)
    {
        goto fwrite_err;
    }         
    
    dummy = EOF;
    fwrite(&dummy, sizeof(uint8), 1, pCalibF);
    fclose(pCalibF);
    OscLog(DEBUG, "%s: done.\n", __func__);
    return SUCCESS;    
    
fwrite_err:
    OscLog(ERROR, "%s: File write error for file %s!\n",
            __func__, strCalibFN);
    return -EFILE_ERROR;    
}
#endif /*0*/


OSC_ERR OscClbCorrectFpnPrnu( uint8* pImg)
{
    uint8 pix;
    uint8 offset;
    uint16 gain;
       
    uint8  u8Tmp;
    int16  i16Tmp;
    uint16 u16Tmp;
    int16  row = pClb->capWin.row_off; 
    int16  col = pClb->capWin.col_off;
    uint8  *pFpn = &pClb->calibData.fpn[col +row*MAX_IMAGE_WIDTH];
    uint16 *pPrnu = &pClb->calibData.prnu[col +row*MAX_IMAGE_WIDTH];
    uint8  *pFpn_prefetch = pFpn;
    uint16 *pPrnu_prefetch = pPrnu;    
    uint8  *pImg_prefetch = pImg;
    
    uint16 idx, nCacheLines;
    uint16 cacheLine = 0;
    
    /* Element count based on the smalest data: Img, FPN */
    uint16 remainColPix;
    uint16 nPixPerCacheLine = CACHE_LINE_LEN/sizeof(uint8);    
    
    
    /* Determine number of affected cache-lines. Since data might be
     * unaligned two more cache-lines may be used in worst case. 
     * 
     * Cache-line increments are dominated by largest elemnt: PRNU. */
    nCacheLines = sizeof(uint16) * pClb->capWin.width/CACHE_LINE_LEN + 2;

    for( row = pClb->capWin.row_off; row<(pClb->capWin.row_off + pClb->capWin.height); row++)
    {
        pFpn = &pClb->calibData.fpn[pClb->capWin.col_off +row*MAX_IMAGE_WIDTH];
        pPrnu = &pClb->calibData.prnu[pClb->capWin.col_off +row*MAX_IMAGE_WIDTH];
        pFpn_prefetch = pFpn;
        pPrnu_prefetch = pPrnu;
        remainColPix = pClb->capWin.width;
        
        /* initial prefetch */
        PREFETCH(pFpn_prefetch);
        PREFETCH(pPrnu_prefetch);
        PREFETCH(pImg_prefetch);
        pFpn_prefetch += CACHE_LINE_LEN/sizeof(uint8) /2;
        pPrnu_prefetch += CACHE_LINE_LEN/sizeof(uint16);
        pImg_prefetch += CACHE_LINE_LEN/sizeof(uint8) /2;  
        
        /* prefetch also next line to be always two lines ahead */
        PREFETCH_NEXT(pFpn_prefetch);
        PREFETCH_NEXT(pPrnu_prefetch);
        PREFETCH_NEXT(pImg_prefetch);        
        pFpn_prefetch += CACHE_LINE_LEN/sizeof(uint8) /2;
        pPrnu_prefetch += CACHE_LINE_LEN/sizeof(uint16);    
        pImg_prefetch += CACHE_LINE_LEN/sizeof(uint8) /2;        
        
        for( cacheLine = 0; cacheLine < nCacheLines; cacheLine++)
        {
            PREFETCH(pFpn_prefetch);
            PREFETCH(pPrnu_prefetch);
            PREFETCH(pImg_prefetch);
            pFpn_prefetch += CACHE_LINE_LEN/sizeof(uint8) /2;
            pPrnu_prefetch += CACHE_LINE_LEN/sizeof(uint16);
            pImg_prefetch += CACHE_LINE_LEN/sizeof(uint8) /2;             
            
            for(idx = 0; idx < (remainColPix%nPixPerCacheLine); idx++)
            {
                pix = *pImg;
                offset = *pFpn;
                gain = *pPrnu;
                                      
                /* add FPN offset */
                i16Tmp = pix - offset;
                u8Tmp = (i16Tmp < 0) ? 0 : (i16Tmp & 0xff); 
                
                /* multiply with PRNU scale */
                u16Tmp = (u8Tmp * gain) >> 8;                
                u8Tmp = (u16Tmp > 255) ? 255 : (u16Tmp & 0xff);
                
                *pImg = u8Tmp;                                
                
                pImg++;
                pFpn++;
                pPrnu++;
                remainColPix--;
                
            } /* accross pixels inside a cache-line */
        } /* accross cache-lines inside a row */        
    } /* accross rows */
    
    return SUCCESS;
}


/* ToDo for more efficient processing 
 * - Select the subset of hotpixels for the selected AOI once it changes 
 * - Try to reduce conditional tests for neighbour location. Eg. dedicated list.*/
OSC_ERR OscClbCorrectHotpixel(uint8* pImg)
{
    uint16 n;
    struct VEC_2D centerPos;
    uint8 center, left, right, top, bottom;
       
    for(n = 0; n < (pClb->calibData.nHotpixel); n++)
    {
        centerPos = pClb->calibData.hotpixels[n];
        
        /* hotpixel x-coordinate within image? */
        if( (centerPos.x >= pClb->capWin.col_off) && 
                (centerPos.x < (pClb->capWin.col_off+ pClb->capWin.width)) )
        {
            centerPos.x -=  pClb->capWin.col_off;
        
            /* hotpixel y-coordinate within image? */
            if( (centerPos.y >= pClb->capWin.row_off) && 
                    (centerPos.y < (pClb->capWin.row_off+ pClb->capWin.height)) )
            {
                centerPos.y -=  pClb->capWin.row_off;                               
                
                /* extract left pixel. Use right if outside image */
                if(centerPos.x > 0) {
                    left = pImg[pClb->capWin.width * centerPos.y + centerPos.x-1];
                } else {
                    left = pImg[pClb->capWin.width * centerPos.y + centerPos.x+1];
                }
                /* extract right pixel. Use left if outside image */
                if(centerPos.x+1 < pClb->capWin.width) {
                    right = pImg[pClb->capWin.width * centerPos.y + centerPos.x+1];
                } else {
                    right = pImg[pClb->capWin.width * centerPos.y + centerPos.x-1];
                }            
                /* extract top pixel. Use bottom if outside image */                
                if(centerPos.y > 0) {
                    top = pImg[pClb->capWin.width * (centerPos.y-1) + centerPos.x];
                } else {
                    top = pImg[pClb->capWin.width * (centerPos.y+1) + centerPos.x];
                }            
                /* extract bottom pixel. Use left if outside image */                
                if(centerPos.y+1 < pClb->capWin.height) {
                    bottom = pImg[pClb->capWin.width * (centerPos.y-1) + centerPos.x];
                } else {
                    bottom = pImg[pClb->capWin.width * (centerPos.y+1) + centerPos.x];
                }                  
                /* compute center replacemnt */
                center = (left+right+top+bottom) >> 2;
                pImg[pClb->capWin.width * centerPos.y + centerPos.x] = center;
            }
            
        }                 
    }
    return SUCCESS;
}



