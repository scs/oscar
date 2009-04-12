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

/*! @file bayer_fast.c
 * @brief Fast debayering without interpolation.
 */
#include "include/vis.h"
#include "vis.h"
#include <stdio.h>
#include <stdlib.h>


OSC_ERR OscVisFastDebayerRGB(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos = 0;
	char *in  = (char *)pRaw->data;
	char *out = (char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			/* Red */
			out[outPos++]=in[(y+1)*pRaw->width+x+1];
			/* Green */
			out[outPos++]=in[y*pRaw->width+x+1];
			/* Blue */
			out[outPos++]=in[y*pRaw->width+x];
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_RGB_24;
	return 0;
}


OSC_ERR OscVisFastDebayerGrey(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut)
{
	uint16 x,y;
	uint32 outPos=0;
	char *in  = (char *)pRaw->data;
	char *out = (char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			                /*     blue           +          2x green          +              red           */
			out[outPos++] = ( in[y*pRaw->width+x] + (in[y*pRaw->width+x+1]<<1) + in[(y+1)*pRaw->width+x+1] )>>2;
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_GREYSCALE;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerLumY(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut)
{
	uint16 x,y;
	uint32 outPos = 0;
	int16 pY;
	int16 R, G, B;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			R = in[(y+1)*pRaw->width+x+1];
			G = in[y*pRaw->width+x+1];
			B = in[y*pRaw->width+x];
			/* Fixed-Point: Faktoren * 128 */
			/* Y = 0.299*R+0.587*G+0.114*B) */

			pY = ((38*R + 75*G + 15*B) >> 7);
			
			out[outPos++] = pY;
		} /* for x */
	} /* for y */
	pOut->width = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type = OSC_PICTURE_GREYSCALE;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerChromU(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos = 0;
	int16 pY, pU;
	int16 R, G, B;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x = 0; x<pRaw->width; x+=2) {
			R = in[(y+1)*pRaw->width+x+1];
			G = in[y*pRaw->width+x+1];
			B = in[y*pRaw->width+x];
			/* Fixed-Point: Faktoren * 128 */
			/* Y = 0.299*R+0.587*G+0.114*B) */

			pY = ((38*R + 75*G + 15*B) >> 7);
			pU = (((B - pY)*63)>>7)+128;

			out[outPos++] = pU;
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type = OSC_PICTURE_CHROM_U;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerChromV(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos=0;
	int16 pY, pV;
	int16 R, G, B;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			R = in[(y+1)*pRaw->width+x+1];
			G = in[y*pRaw->width+x+1];
			B = in[y*pRaw->width+x];
			/* Fixed-Point: Faktoren * 128 */
			/* Y = 0.299*R+0.587*G+0.114*B) */

			pY = ((38*R + 75*G + 15*B) >> 7);
			pV = (((R - pY)*112)>>7)+128;

			out[outPos++] = pV;
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_CHROM_V;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerYUV422(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos = 0;
	int16 pY1, pY2, pU1, pV1;
	int16 R1, G1, B1, R2, G2, B2;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y < pRaw->height; y+=2) {
		for (x=0; x < pRaw->width; x+=4) {
			R1 = in[(y+1)*pRaw->width+x+1];
			G1 = in[y*pRaw->width+x+1];
			B1 = in[y*pRaw->width+x];
			R2 = in[(y+1)*pRaw->width+x+3];
			G2 = in[y*pRaw->width+x+3];
			B2 = in[y*pRaw->width+x+2];
			/* Fixed-Point: Faktoren * 128 */
			/* Y = 0.299*R+0.587*G+0.114*B) */

			pY1 = ((38*R1 + 75*G1 + 15*B1) >> 7);	
			pU1 = (((B1 - pY1)*63)>>7)+128;
			pV1 = (((R1 - pY1)*112)>>7)+128;

			pY2 =  (38*R2 + 75*G2 + 15*B2) >> 7;	

			out[outPos++]=pU1;
			out[outPos++]=pY1;
			out[outPos++]=pV1;
			out[outPos++]=pY2;

		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_YUV_422;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerHSL_H(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos = 0;
	uint8 max, min;
	uint16 hue;
	uint16 R, G, B;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			R = in[(y+1)*pRaw->width+x+1];
			G = in[y*pRaw->width+x+1];
			B = in[y*pRaw->width+x];
			
			/* get max and min */
			if( R > G ){
				max = R;
				min = G;
			}else{
				max = G;
				min = R;
			}
			if( B > max ){
				max = B;
			}
			if( B < min ){
				min = B;
			}

			/* hue */
			if(max == min){
				hue = 0;
			}else{
				if(max == R){
					hue = 10922 * (G - B) / (max - min) + 65536;
				}else if(max == G){
					hue = 10922 * (B - R) / (max - min) + 21845;
				}else{
					hue = 10922 * (R - G) / (max - min) + 43690;
				}
			}
			out[outPos++] = (uint8)(hue >> 8);
			
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_HUE;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerHSL_S(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos = 0;
	uint8 max, min;
	uint16 sat, lum;
	uint16 R, G, B;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			R = in[(y+1)*pRaw->width+x+1];
			G = in[y*pRaw->width+x+1];
			B = in[y*pRaw->width+x];
			
			/* get max and min */
			if( R > G ){
				max = R;
				min = G;
			}else{
				max = G;
				min = R;
			}
			if( B > max ){
				max = B;
			}
			if( B < min ){
				min = B;
			}
			
			/* luminance */
			lum = (max + min) >> 1;

			/* saturation */
			if(lum < 128){
				sat = ((max - min) << 8) / (lum << 1);
			}else{
				sat = ((max - min) << 8) / (512 - (lum << 1));
			}

			out[outPos++] = (uint8)sat;
			
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_HUE;
	return SUCCESS;
}


OSC_ERR OscVisFastDebayerHSL_L(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut) 
{
	uint16 x,y;
	uint32 outPos = 0;
	uint8 max, min;
	uint16 lum;
	uint16 R, G, B;
	unsigned char *in  = (unsigned char *)pRaw->data;
	unsigned char *out = (unsigned char *)pOut->data;

	for (y=0; y<pRaw->height; y+=2) {
		for (x=0; x<pRaw->width; x+=2) {
			R = in[(y+1)*pRaw->width+x+1];
			G = in[y*pRaw->width+x+1];
			B = in[y*pRaw->width+x];
			
			/* get max and min */
			if( R > G ){
				max = R;
				min = G;
			}else{
				max = G;
				min = R;
			}
			if( B > max ){
				max = B;
			}
			if( B < min ){
				min = B;
			}
			
			/* luminance */
			lum = (max + min) >> 1;

			out[outPos++] = (uint8)lum;
			
		} /* for x */
	} /* for y */
	pOut->width  = pRaw->width/2;
	pOut->height = pRaw->height/2; 
	pOut->type  = OSC_PICTURE_HUE;
	return SUCCESS;
}
