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

/*! @file morphology.c
 * @brief Mathematical morphology algorithms.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vis.h"

/* Declaration and Definition of the structure elements */
/* The kernel pixels (offset vectors) => format: {x0,y0,x1,y1,x2,y2,...,xn,yn} */
int8 aDisk8Vertical[14] = {0, -3, 0, -2, 0, -1, 0, 0, 0, 1, 0, 2, 0, 3};
int8 aDisk8Horizontal[14] = {-3, 0, -2, 0, -1, 0, 0, 0, 1, 0, 2, 0, 3, 0};	
int8 aDisk8DiagTB[10] = {-2, -2, -1, -1, 0, 0, 1, 1, 2, 2};
int8 aDisk8DiagBT[10] = {-2, 2, -1, 1, 0, 0, 1, -1, 2, -2};
int8 aDisk2[26] = {0, -2, -1, -1, 0, -1, 1, -1, -2, 0, -1, 0, 0, 0, 1, 0, 2, 0, -1, 1, 0, 1, 1, 1, 0, 2};
int8 aDisk1[10] = {0, -1, -1, 0, 0, 0, 1, 0, 0, -1};
int8 aDisk0[2] = {0, 0};

struct OSC_VIS_STREL DISK8 = {TRUE, 4, {{14, 7, 0, aDisk8Horizontal}, { 14, 0, 7, aDisk8Vertical}, {10, 5, 5, aDisk8DiagBT}, {10, 5, 5, aDisk8DiagTB}}};
struct OSC_VIS_STREL DISK2 = {FALSE, 1, {{ 26, 5, 5, aDisk2}, {0,0,0,NULL}, {0,0,0,NULL}, {0,0,0,NULL}}};
struct OSC_VIS_STREL DISK1 = {FALSE, 1, {{ 10, 3, 3, aDisk1}, {0,0,0,NULL}, {0,0,0,NULL}, {0,0,0,NULL}}};
struct OSC_VIS_STREL DISK0 = {FALSE, 1, {{ 2, 1, 1, aDisk0}, {0,0,0,NULL}, {0,0,0,NULL}, {0,0,0,NULL}}};

/* Functions */
OSC_ERR OscVisErode(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 *pTempBuffer, struct OSC_VIS_STREL *pStrEl, uint8 nRepetitions)
{
	uint16 i,j,t,r,k;
	uint16 yOffset, xOffset;
	uint8 tempPixelVal;
	uint8 nSubElements = 1;
	
	/* mapping */
	uint8 *pBinImgIn = (uint8*)picIn->data;
	uint8 *pBinImgOut = (uint8*)picOut->data;
	uint16 width = picIn->width;
	uint16 height = picIn->height;
	
	
	/* check if the structure element has decompositions */
	if (pStrEl->isDecomposed)
		nSubElements = pStrEl->numberOfSubElements;
	else nSubElements = 1;
	/* copy the input image to the temporary buffer */
	memcpy(pTempBuffer, pBinImgIn, sizeof(uint8)*width*height);	

	/* Repetition loop for multiple erosions */	
	for(k = 0; k < nRepetitions; k++)
	{
		/* Repetition loop for sub structure elements */	
		for(r = 0; r < nSubElements; r++)
		{	
			/* move the initial start pixel according to the size of the kernel (boundary condition). */			
			if (pStrEl->subStrEl[r].kernelHeight > 0)
				yOffset = (pStrEl->subStrEl[r].kernelHeight-1) >> 1;
			else yOffset = 0;
				
			if (pStrEl->subStrEl[r].kernelWidth > 0)
				xOffset = (pStrEl->subStrEl[r].kernelWidth-1) >> 1;
			else xOffset = 0;
			
			
			 /* erode on the temp buffer and write result to the output buffer */
			
			/* safe area */
			for(i = yOffset; i < (height - yOffset); i++)
			{
				for(j = xOffset; j < (width - xOffset); j++)
				{
					tempPixelVal = pTempBuffer[i * width + j];				
					for(t = 0; t < pStrEl->subStrEl[r].arrayLength; t = t+2)
					{							
						tempPixelVal = (tempPixelVal * pTempBuffer[(i + pStrEl->subStrEl[r].kernelArray[t+1])* width + (j + pStrEl->subStrEl[r].kernelArray[t])]);	
					}					
					pBinImgOut[i * width + j] = tempPixelVal;
				}
			}		
			/* if the kernel is decomposed, copy result to the temp buffer again */
			if (nSubElements > 1)		
				memcpy(pTempBuffer, pBinImgIn, sizeof(uint8)*width*height);	
			
		}
		/* if multiple erosions are demanded, copy result to the temp buffer again */
		if (nRepetitions > 1)
				memcpy(pTempBuffer, pBinImgIn, sizeof(uint8)*width*height);		
	}
	/* finalize picture */
	picOut->height = height;
	picOut->width = width;
	picOut->type = OSC_PICTURE_BINARY;
	return SUCCESS;	
}

OSC_ERR OscVisDilate(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 *pTempBuffer, struct OSC_VIS_STREL *pStrEl, uint8 nRepetitions)
{
	uint16 i,j,t,r,k;
	uint16 yOffset, xOffset;
	uint8 tempPixelVal;
	uint8 nSubElements = 1;
	
	/* mapping */
	uint8 *pBinImgIn = (uint8*)picIn->data;
	uint8 *pBinImgOut = (uint8*)picOut->data;
	uint16 width = picIn->width;
	uint16 height = picIn->height;
	
	/* check if the structure element has decompositions */
	if (pStrEl->isDecomposed)
		nSubElements = pStrEl->numberOfSubElements;
	else nSubElements = 1;
	
	/* copy the input image to the temporary buffer */
	memcpy(pTempBuffer, pBinImgIn, sizeof(uint8)*width*height); /*initial copy: input to temp buffer */
	
	/* Repetition loop for multiple erosions */	
	for(k = 0; k < nRepetitions; k++)
	{
		/* Repetition loop for sub structure elements */	
		for(r = 0; r < nSubElements; r++)
		{				
			if (pStrEl->subStrEl[r].kernelHeight > 0)
				yOffset = (pStrEl->subStrEl[r].kernelHeight-1) >> 1;
			else yOffset = 0;
				
			if (pStrEl->subStrEl[r].kernelWidth > 0)
				xOffset = (pStrEl->subStrEl[r].kernelWidth-1) >> 1;
			else xOffset = 0;
			
			for(i = yOffset; i < (height - yOffset); i++)
			{
				for(j = xOffset; j < (width - xOffset); j++)
				{

					tempPixelVal = pTempBuffer[i * width + j];
					
					for(t = 0; t < pStrEl->subStrEl[r].arrayLength; t = t+2)
					{				
						tempPixelVal = (tempPixelVal || pTempBuffer[(i + pStrEl->subStrEl[r].kernelArray[t+1]) * width + (j + pStrEl->subStrEl[r].kernelArray[t])]);	
					}
					
					pBinImgOut[i * width + j] = tempPixelVal;
				}
			}
			/* if the kernel is decomposed, copy result to the temp buffer again */
			if (nSubElements > 1)
				memcpy(pTempBuffer, pBinImgOut, sizeof(uint8)*width*height);
		}
		/* if multiple dilations are demanded, copy result to the temp buffer again */
		if (nRepetitions > 1)
				memcpy(pTempBuffer, pBinImgOut, sizeof(uint8)*width*height);
	}
	/* finalize picture */
	picOut->height = height;
	picOut->width = width;
	picOut->type = OSC_PICTURE_BINARY;
	return SUCCESS;	
}





