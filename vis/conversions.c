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

/*! @file conversions.c
 * @brief Image conversion algorithms.
 */


#include <stdio.h>
#include <stdlib.h>


#include "vis.h"

OscFunction( OscVisRGB2BGR, struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut)

  uint32 outPos = 0;
  uint32 nPixels;
  uint32 pix, cacheLine, nCacheLines, nPixPerCacheLine;
  uint8 *pImgOut = (uint8*)picOut->data;
  const uint16 width = picIn->width;
  const uint16 height = picIn->height;
  uint8 *pImgIn = (uint8*)picIn->data;
  uint8 *pIn_prefetch = pImgIn;
  uint8 *pIn = pImgIn;
  uint8 pointerOffset;
  uint8 tmp;

  OscAssert(picIn->type == OSC_PICTURE_RGB_24);
  nPixels = width*height;

  /* load cache (32 byte -> ~10 RGB pixels) */
  PREFETCH(pIn_prefetch);
  /* set pointer + 10 pixels (RGB) -> +30bytes*/
  pointerOffset = (CACHE_LINE_LEN/(3*sizeof(uint8))) * 3;
  pIn_prefetch += pointerOffset;

  /* number of necessary cache lines to process entire RGB image (10 RGB pixels per CL) */
  nCacheLines = (nPixels*3*sizeof(uint8))/pointerOffset;
  /* number of pixels per cacheline (10 RGB pixels) */
    nPixPerCacheLine = CACHE_LINE_LEN/(3*sizeof(uint8));

    for(cacheLine = 0; cacheLine < nCacheLines; cacheLine++)
    {
      /* load next 10 RGB pixels */
      PREFETCH(pIn_prefetch);
      pIn_prefetch += pointerOffset;
      for(pix = 0; pix < nPixPerCacheLine; pix++)
        {
          /* Use temporary buffer for R/B value swap,
           * in order to support in-place modification. */
          tmp = pIn[0];
          pImgOut[0] = pIn[2];
          pImgOut[1] = pIn[1];
          pImgOut[2] = tmp;
          pIn += 3;
          pImgOut += 3;
        }

    }
    picOut->width = width;
    picOut->height = height;
    picOut->type = OSC_PICTURE_BGR_24;
OscFunctionEnd()

OSC_ERR OscVisBGR2Grey(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut)
{

	uint32 outPos = 0;
	uint32 nPixels;
	uint32 pix, cacheLine, nCacheLines, nPixPerCacheLine;
	uint8 *pGrayImgOut = (uint8*)picOut->data;
	const uint16 width = picIn->width;
	const uint16 height = picIn->height;
	uint8 *pImgIn = (uint8*)picIn->data;
	uint8 *pIn_prefetch = pImgIn;
	uint8 *pIn = pImgIn;
	uint8 pointerOffset;
	
	nPixels = width*height;
	
	/* load cache (32 byte -> ~10 RGB pixels) */
	PREFETCH(pIn_prefetch);
	/* set pointer + 10 pixels (RGB) -> +30bytes*/
	pointerOffset = (CACHE_LINE_LEN/(3*sizeof(uint8))) * 3;
	pIn_prefetch += pointerOffset;
	
	/* number of necessary cache lines to process entire RGB image (10 RGB pixels per CL) */
	nCacheLines = (nPixels*3*sizeof(uint8))/pointerOffset;
	/* number of pixels per cacheline (10 RGB pixels) */
    nPixPerCacheLine = CACHE_LINE_LEN/(3*sizeof(uint8));

    for(cacheLine = 0; cacheLine < nCacheLines; cacheLine++)
    {
    	/* load next 10 RGB pixels */
    	PREFETCH(pIn_prefetch);
    	pIn_prefetch += pointerOffset;
    	for(pix = 0; pix < nPixPerCacheLine; pix++)
        {
        	pGrayImgOut[outPos++] = (15*pIn[0] + 75*pIn[1] + 38*pIn[2]) >> 7;
        	pIn += 3;
        }
        	
    }
    picOut->width = width;
    picOut->height = height;
    picOut->type = OSC_PICTURE_GREYSCALE;
	return SUCCESS;
}



OSC_ERR OscVisGrey2BW(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 threshold, bool bDarkIsForeground)
{	
	uint8 foregroundValue, backgroundValue;
	
	uint8 *pGrayImgIn = (uint8*)picIn->data;
	uint8 *pBWImgOut = (uint8*)picOut->data;
	const uint16 width = picIn->width;
	const uint16 height = picIn->height;
	
	uint32 outPos = 0;
	uint32 nPixels;
	uint32 pix, cacheLine, nCacheLines, nPixPerCacheLine;
	uint8 *pIn_prefetch = pGrayImgIn;
	uint8 *pIn = pGrayImgIn;
	nPixels = width*height;
	
	if(bDarkIsForeground)
	{
		foregroundValue = 0;
		backgroundValue = 1;
	}
	else
	{
		foregroundValue = 1;
		backgroundValue = 0;
	}			
	PREFETCH(pIn_prefetch);
	/* set pointer + 32 pixels (mono u8)*/
	pIn_prefetch += (CACHE_LINE_LEN);
	
	/* number of necessary cache lines */
	nCacheLines = nPixels/CACHE_LINE_LEN;
	/* number of pixels per cacheline (10 RGB pixels) */
    nPixPerCacheLine = CACHE_LINE_LEN;
    for(cacheLine = 0; cacheLine < nCacheLines; cacheLine++)
    {
    	/* load next 10 RGB pixels */
    	PREFETCH(pIn_prefetch);
    	pIn_prefetch += (CACHE_LINE_LEN);
    	for(pix = 0; pix < nPixPerCacheLine; pix++)
        {
        	if (pIn[0] > threshold)
				pBWImgOut[outPos++] = foregroundValue;
			else
				pBWImgOut[outPos++] = backgroundValue; 		
        	pIn++;
        }
    }
    picOut->height = height;
    picOut->width = width;
    picOut->type = OSC_PICTURE_BINARY;
	return SUCCESS;
}

OSC_ERR OscVisBGR2BW(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 threshold, bool bDarkIsForeground)
{
	uint8 *pImgIn = (uint8*)picIn->data;
	uint8 *pBWImgOut = (uint8*)picOut->data;
	const uint16 width = picIn->width;
	const uint16 height = picIn->height;
	
	uint8 foregroundValue, backgroundValue;
	uint32 outPos = 0;
	uint32 nPixels;
	uint32 pix, cacheLine, nCacheLines, nPixPerCacheLine;
	uint8 *pIn_prefetch = pImgIn;
	uint8 *pIn = pImgIn;
	uint8 pointerOffset;
	uint8 tempGrayVal;
	
	nPixels = width*height;
	
	if(bDarkIsForeground )
	{
		foregroundValue = 0;
		backgroundValue = 1;
	}
	else
	{
		foregroundValue = 1;
		backgroundValue = 0;
	}
	/* load cache (32 byte -> ~10 RGB pixels) */
	PREFETCH(pIn_prefetch);
	/* set pointer + 10 pixels (RGB) -> +30bytes*/
	pointerOffset = (CACHE_LINE_LEN/(3*sizeof(uint8))) * 3;
	pIn_prefetch += pointerOffset;
	
	/* number of necessary cache lines to process entire RGB image (10 RGB pixels per CL) */
	nCacheLines = (nPixels*3*sizeof(uint8))/pointerOffset;
	/* number of pixels per cacheline (10 RGB pixels) */
    nPixPerCacheLine = CACHE_LINE_LEN/(3*sizeof(uint8));

    for(cacheLine = 0; cacheLine < nCacheLines; cacheLine++)
    {
    	/* load next 10 RGB pixels */
    	PREFETCH(pIn_prefetch);
    	pIn_prefetch += pointerOffset;
    	for(pix = 0; pix < nPixPerCacheLine; pix++)
        {
        	tempGrayVal = (15*pIn[0] + 75*pIn[1] + 38*pIn[2]) >> 7;
        	if (tempGrayVal > threshold)
				pBWImgOut[outPos++] = foregroundValue;
			else
				pBWImgOut[outPos++] = backgroundValue; 		
        	pIn += 3;
        }
        	
    }
    picOut->height = height;
    picOut->width = width;
    picOut->type = OSC_PICTURE_BINARY;
	return SUCCESS;
}


/* Old, un-optimized functions... */

/* Convert RGB image to Grayscale image */
/* at the moment, the values are just passed over... */
/* Fixed-Point: Faktoren * 128 */
/* Gray = 0.299*R+0.587*G+0.114*B) */
/*
OSC_ERR OscVisRGB2Gray_OLD(uint8 *pImgIn, uint8 *pGrayImgOut, const uint16 width, const uint16 height)	
{
	uint16 i,j;
	uint32 outPos = 0;
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){	
			pGrayImgOut[outPos++] = (15*pImgIn[(i*width + j)*3] + 75*pImgIn[(i*width+j)*3 + 1] + 38*pImgIn[(i*width+j)*3] + 2) >> 7;		
		}
	}
	return SUCCESS;
}
*/


/* Threshold to convert into binary image */	
/*
OSC_ERR OscVisGray2BW_OLD(uint8 *pGrayImgIn, uint8 *pBWImgOut, const uint16 width, const uint16 height, uint8 threshold, bool bDetectDarkObjects)
{	
	uint16 i,j;
	uint8 foregroundValue, backgroundValue;
	if(bDetectDarkObjects)
	{
		foregroundValue = 0;
		backgroundValue = 1;
	}
	else
	{
		foregroundValue = 1;
		backgroundValue = 0;
	}			
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++){			
			if (pGrayImgIn[i*width + j] > threshold)
				pBWImgOut[i*width + j] = foregroundValue;
			else
				pBWImgOut[i*width + j] = backgroundValue; 				
		}
	}
	
	
	return SUCCESS;
}
*/
