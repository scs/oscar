/*	JPEG encoder library
	Original Implementation by Nitin Gupta
	Adapted to leanXcam by Reto Baettig
	
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

/*! @file
 * @brief Private JPG module definition
 */

#ifndef JPG_PRIV_H_
#define JPG_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "oscar.h"

#define		BLOCK_SIZE				64

typedef struct JPEG_ENCODER_STRUCTURE
{
	uint16	mcu_width;
	uint16	mcu_height;
	uint16	horizontal_mcus;
	uint16	vertical_mcus;
	uint16	cols_in_right_mcus;
	uint16	rows_in_bottom_mcus;

	uint16	rows;
	uint16	cols;

	uint16	length_minus_mcu_width;
	uint16	length_minus_width;
	uint16	incr;
	uint16	mcu_width_size;
	uint16	offset;

	int16 ldc1;
	int16 ldc2;
	int16 ldc3;

	int16 debug_pass;

} JPEG_ENCODER_STRUCTURE;

extern uint8	Lqt [BLOCK_SIZE];
extern uint8	Cqt [BLOCK_SIZE];
extern uint16	ILqt [BLOCK_SIZE];
extern uint16	ICqt [BLOCK_SIZE];

typedef struct IMGDATA {
	int16	Y1 [BLOCK_SIZE];
	int16	Y2 [BLOCK_SIZE];
	int16	Y3 [BLOCK_SIZE];
	int16	Y4 [BLOCK_SIZE];
	int16	CB [BLOCK_SIZE];
	int16	CR [BLOCK_SIZE];
/*	int16	Temp [BLOCK_SIZE];*/
} IMGDATA;

extern uint32 lcode;
extern uint16 bitindex;

/*======================= Private methods ==============================*/
void initialization (JPEG_ENCODER_STRUCTURE *, uint32, uint32, uint32);

uint16 DSP_Division (uint32, uint32);
void initialize_quantization_tables (uint32);

uint8* write_markers (uint8 *, uint32, uint32, uint32);

void read_400_format (IMGDATA *img, JPEG_ENCODER_STRUCTURE *, uint8 *);
void read_420_format (IMGDATA *img, JPEG_ENCODER_STRUCTURE *, uint8 *);
void read_422_format (IMGDATA *img, JPEG_ENCODER_STRUCTURE *, uint8 *);
void read_444_format (IMGDATA *img, JPEG_ENCODER_STRUCTURE *, uint8 *);
void BGR_2_444 (uint8 *, uint8 *, uint32, uint32);

uint8* encodeMCU (JPEG_ENCODER_STRUCTURE *, uint32, uint8 *);

void levelshift (int16 *);
void DCT (int16 *);
void quantization (int16 *, uint16 *, uint16 *);
uint8* huffman (JPEG_ENCODER_STRUCTURE *, uint16, uint8 *, uint16 *);

uint8* close_bitstream (uint8 *);
#
#endif /*JPG_PRIV_H_*/
