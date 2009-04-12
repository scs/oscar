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

/*! @file transform.c
 * @brief Image transformation algorithms.
 */
#include "include/vis.h"
#include "vis.h"
#include <stdio.h>
#include <stdlib.h>


OSC_ERR OscVisSobel(const uint8 *pIn, uint8 *pOut, const uint16 width, const uint16 height, const uint8 exp)
{
	uint16 x,y;
	int16 gradX, gradY, pix;
	for (y = 1; y < height-1; y++){
		for (x = 1; x < width-1; x++){
			/* x direction operator */
			gradX = pIn[(y-1) * width + (x-1)]       -  pIn[(y-1) * width + (x+1)]       + 
				   (pIn[y * width + (x-1)    ] << 1) - (pIn[y * width + (x+1)    ] << 1) + 
				    pIn[(y+1) * width + (x-1)]       -  pIn[(y+1) * width + (x+1)];
			
			/* y direction operator */
			gradY = pIn[(y-1) * width + (x-1)] + (pIn[(y-1) * width + x] << 1) + pIn[(y-1) * width + (x+1)] -
				    pIn[(y+1) * width + (x-1)] - (pIn[(y+1) * width + x] << 1) - pIn[(y+1) * width + (x+1)];
			
			/* output (instead of calculating the square root and normalizing the result we simply shift by exp and saturate) */
			pix = (gradY * gradY + gradX * gradX) >> exp;
			if(pix > 255) pix = 255;
			pOut[y * width + x] = pix; 
		}
	}
	return SUCCESS;	
}

