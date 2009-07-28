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

/*
 * On the leanXcam, encoding a CIF-Image takes about 70ms
 */

#include <stdlib.h>
#include <string.h>

#include "jpg.h"

uint8	Lqt [BLOCK_SIZE];
uint8	Cqt [BLOCK_SIZE];
uint16	ILqt [BLOCK_SIZE];
uint16	ICqt [BLOCK_SIZE];

struct IMGDATA *Image=NULL; /* Singleton */
JPEG_ENCODER_STRUCTURE *JpegStruct = NULL /* Singleton */;
uint32 lcode = 0;
uint16 bitindex = 0; 
uint16  Temp [BLOCK_SIZE];

void (*read_format) (IMGDATA *img, JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure, uint8 *input_ptr);

void initialization (JPEG_ENCODER_STRUCTURE *jpeg, uint32 image_format, uint32 image_width, uint32 image_height)
{
	uint16 mcu_width, mcu_height, bytes_per_pixel;

	/* RB there was a bug in the original code that bitindex and lcode were not initialized at
	 * the second time
 	 */
	bitindex = 0; 
	lcode=0;

	jpeg->debug_pass = 0; 

	switch (image_format) 
	{
	case OSC_PICTURE_YUV_444:
		read_format = read_444_format;
		break;
	case OSC_PICTURE_YUV_422:
		read_format = read_422_format;
		break;
	case OSC_PICTURE_YUV_420:
		read_format = read_420_format;
		break;
	case OSC_PICTURE_YUV_400:
		read_format = read_400_format;
		break;
	}

	bytes_per_pixel = OSC_PICTURE_TYPE_COLOR_DEPTH(image_format)/8;

	if (image_format == OSC_PICTURE_YUV_400|| 
	    image_format == OSC_PICTURE_YUV_444)
	{
		jpeg->mcu_width = mcu_width = 8;
		jpeg->mcu_height = mcu_height = 8;

		jpeg->horizontal_mcus = (uint16) ((image_width + mcu_width - 1) >> 3);
		jpeg->vertical_mcus = (uint16) ((image_height + mcu_height - 1) >> 3);
	}
	else
	{
		jpeg->mcu_width = mcu_width = 16;
		jpeg->horizontal_mcus = (uint16) ((image_width + mcu_width - 1) >> 4);

		if (image_format == OSC_PICTURE_YUV_420)
		{
			jpeg->mcu_height = mcu_height = 16;
			jpeg->vertical_mcus = (uint16) ((image_height + mcu_height - 1) >> 4);

		}
		else
		{
			jpeg->mcu_height = mcu_height = 8;
			jpeg->vertical_mcus = (uint16) ((image_height + mcu_height - 1) >> 3);

		}
	}

	jpeg->rows_in_bottom_mcus = (uint16) (image_height - (jpeg->vertical_mcus - 1) * mcu_height);
	jpeg->cols_in_right_mcus = (uint16) (image_width - (jpeg->horizontal_mcus - 1) * mcu_width);

	jpeg->length_minus_mcu_width = (uint16) ((image_width - mcu_width) * bytes_per_pixel);
	jpeg->length_minus_width = (uint16) ((image_width - jpeg->cols_in_right_mcus) * bytes_per_pixel);

	jpeg->mcu_width_size = (uint16) (mcu_width * bytes_per_pixel);

	if (image_format != OSC_PICTURE_YUV_420)
		jpeg->offset = (uint16) ((image_width * (mcu_height - 1) - (mcu_width - jpeg->cols_in_right_mcus)) * bytes_per_pixel);
	else
		jpeg->offset = (uint16) ((image_width * ((mcu_height >> 1) - 1) - (mcu_width - jpeg->cols_in_right_mcus)) * bytes_per_pixel);

	jpeg->ldc1 = 0;
	jpeg->ldc2 = 0;
	jpeg->ldc3 = 0;
}

uint8* OscJpgEncode(struct OSC_PICTURE *pic, uint8 *output_ptr, uint32 quality_factor)
{
	uint16 i, j;
	uint8 *input_ptr = (uint8 *)pic->data;

	JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure;

	if (Image==NULL) {
		Image = malloc(sizeof(struct IMGDATA));	
		if (Image == NULL)
			OscFatalErr("Could not allocate memory\n");
	}

	if (JpegStruct==NULL) {
		JpegStruct= malloc(sizeof(JPEG_ENCODER_STRUCTURE));	
		if (JpegStruct == NULL)
			OscFatalErr("Could not allocate memory\n");
	}

	jpeg_encoder_structure = JpegStruct;
	
	if (pic->type == OSC_PICTURE_BGR_24)
	{
		pic->type = OSC_PICTURE_YUV_444;
		BGR_2_444 (input_ptr, output_ptr, pic->width, pic->height);
		memcpy(input_ptr, output_ptr, pic->width*pic->height*3);
	}

	if (pic->type != OSC_PICTURE_YUV_444)
	{
		/* unsupported or untested image format */
		OscFatalErr("Unsupported Image Format in OscJpgEncode\n");
	} 

	/* Initialization of JPEG control structure */
	initialization (jpeg_encoder_structure, pic->type, pic->width, pic->height);
	
	/* Quantization Table Initialization */
	initialize_quantization_tables (quality_factor);

	/* Writing Marker Data */

	output_ptr = write_markers (output_ptr, pic->type, pic->width, pic->height);

	for (i=1; i<=jpeg_encoder_structure->vertical_mcus; i++)
	{
		if (i < jpeg_encoder_structure->vertical_mcus)
			jpeg_encoder_structure->rows = jpeg_encoder_structure->mcu_height;
		else
			jpeg_encoder_structure->rows = jpeg_encoder_structure->rows_in_bottom_mcus;

		for (j=1; j<=jpeg_encoder_structure->horizontal_mcus; j++)
		{
			if (j < jpeg_encoder_structure->horizontal_mcus)
			{
				jpeg_encoder_structure->cols = jpeg_encoder_structure->mcu_width;
				jpeg_encoder_structure->incr = jpeg_encoder_structure->length_minus_mcu_width;
			}
			else
			{
				jpeg_encoder_structure->cols = jpeg_encoder_structure->cols_in_right_mcus;
				jpeg_encoder_structure->incr = jpeg_encoder_structure->length_minus_width;
			}

			read_format (Image, jpeg_encoder_structure, input_ptr);
			/* Encode the data in MCU */
			output_ptr = encodeMCU (jpeg_encoder_structure, pic->type, output_ptr);

			input_ptr += jpeg_encoder_structure->mcu_width_size;
		}

		input_ptr += jpeg_encoder_structure->offset;
	}

	/* Close Routine */
	close_bitstream (output_ptr);
	return output_ptr;
}

uint8* encodeMCU (JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure, uint32 image_format, uint8 *output_ptr)
{
	levelshift (Image->Y1);
	DCT ((int16 *)Image->Y1);
	quantization (Image->Y1, ILqt, Temp);

	output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr, Temp);

	if (image_format == OSC_PICTURE_YUV_420 || 
	    image_format == OSC_PICTURE_YUV_422)
	{
		levelshift (Image->Y2);
		DCT (Image->Y2);
		quantization (Image->Y2, ILqt, Temp);
		output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr, Temp);

		if (image_format == OSC_PICTURE_YUV_420)
		{
			levelshift (Image->Y3);
			DCT (Image->Y3);
			quantization (Image->Y3, ILqt, Temp);
			output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr, Temp);

			levelshift (Image->Y4);
			DCT (Image->Y4);
			quantization (Image->Y4, ILqt, Temp);
			output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr, Temp);
		}
	}

	if (image_format != OSC_PICTURE_YUV_400)
	{
		levelshift (Image->CB);
		DCT (Image->CB);
		quantization (Image->CB, ICqt, Temp);
		output_ptr = huffman (jpeg_encoder_structure, 2, output_ptr, Temp);

		levelshift (Image->CR);
		DCT (Image->CR);
		quantization (Image->CR, ICqt, Temp);
		output_ptr = huffman (jpeg_encoder_structure, 3, output_ptr, Temp);
	}

	jpeg_encoder_structure->debug_pass++;
	return output_ptr;
}
