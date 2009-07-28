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

/*! @file filters.c
 * @brief Image filter algorithms.
 */


#include <stdio.h>
#include <stdlib.h>

#include "vis.h"


/* Realization of a 3x3 Gauss filter kernel */
int8 aGauss3x3[9] = { 1,  2,  1,
				       2,  4,  2,
				       1,  2,  1 };				    
struct OSC_VIS_FILTER_KERNEL GAUSS3X3 = {3, 3, 16, aGauss3x3};

/* Realization of a 5x5 Gauss filter kernel */
int8 aGauss5x5[25] = { 1, 4, 6, 4, 1,
				       4,16,24,16, 4,
				       6,24,36,24, 6,
				       4,16,24,16, 4,
				       1, 4, 6, 4, 1 };				    
struct OSC_VIS_FILTER_KERNEL GAUSS5X5 = {5, 5, 256, aGauss5x5};

/* Realization of a 3x3 Median filter kernel */
int8 aMedian3x3[9] = { 1,  1,  1,
				       1,  1,  1,
				       1,  1,  1 };				    
struct OSC_VIS_FILTER_KERNEL MEDIAN3X3 = {3, 3, 9, aMedian3x3};

/* Realization of a 5x5 Median filter kernel */
int8 aMedian5x5[25] = { 1, 1, 1, 1, 1,
				       1, 1, 1, 1, 1,
				       1, 1, 1, 1, 1,
				       1, 1, 1, 1, 1,
				       1, 1, 1, 1, 1 };				    
struct OSC_VIS_FILTER_KERNEL MEDIAN5X5 = {5, 5, 25, aMedian5x5};



/* A generic 2D filter for grayscale images */
OSC_ERR OscVisFilter2D(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 *pTemp, struct OSC_VIS_FILTER_KERNEL *pKernel)
{
	const uint8 *pIn = (uint8*)picIn->data;
	uint8 *pOut = (uint8*)picOut->data;
	const uint16 width = picIn->width;
	const uint16 height = picIn->height;
	
	uint16 x,y,kx,ky;
	uint16 yOffset, xOffset;
	uint32 tempVal;
	
	/* move the initial start pixel according to the size of the kernel (boundary condition). */			
	if (pKernel->kernelHeight > 0)
		yOffset = (pKernel->kernelHeight-1) >> 1;
	else yOffset = 0;
		
	if (pKernel->kernelWidth > 0)
		xOffset = (pKernel->kernelWidth-1) >> 1;
	else xOffset = 0;
	
	/* copy the input image to the temporary buffer */
	memcpy_inline(pTemp, pIn, ((sizeof(uint8)*width*height) >> 2));
	/* safe area computation */
	for(y = yOffset; y < (height - yOffset); y++)
	{
		for(x = xOffset; x < (width - xOffset); x++)
		{		
			tempVal = 0;
			for(ky = 0; ky < pKernel->kernelHeight; ky++)
			{
				for(kx = 0; kx < pKernel->kernelWidth; kx++)
				{
					tempVal = tempVal + (pTemp[(y-yOffset+ky) * width + (x-xOffset+kx)] * pKernel->kernelArray[ky * pKernel->kernelWidth + kx]);
				}
			}
			pOut[y * width + x] = tempVal/pKernel->kernelWeight;	
		}
	}	
	/* finalize picture */
	picOut->width = width;
	picOut->height = height;
	picOut->type = OSC_PICTURE_GREYSCALE;
	return SUCCESS;	
}








