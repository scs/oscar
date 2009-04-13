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

#include "jpg.h"

void read_400_format (struct IMGDATA *img, JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure, uint8 *input_ptr)
{
	int32 i, j;
	int16 *Y1_Ptr = img->Y1;

	uint16 rows = jpeg_encoder_structure->rows;
	uint16 cols = jpeg_encoder_structure->cols;
	uint16 incr = jpeg_encoder_structure->incr;

	for (i=rows; i>0; i--)
	{
		for (j=cols; j>0; j--)
			*Y1_Ptr++ = *input_ptr++;

		for (j=8-cols; j>0; j--)
			{ *Y1_Ptr = *(Y1_Ptr-1); Y1_Ptr++; }

		input_ptr += incr;
	}

	for (i=8-rows; i>0; i--)
	{
		for (j=8; j>0; j--) {
			*Y1_Ptr = *(Y1_Ptr - 8);
			Y1_Ptr++;
		}
	}
}

void read_420_format (struct IMGDATA *img, JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure, uint8 *input_ptr)
{
	int32 i, j;
	uint16 Y1_rows, Y3_rows, Y1_cols, Y2_cols;

	int16 *Y1_Ptr = img->Y1;
	int16 *Y2_Ptr = img->Y2;
	int16 *Y3_Ptr = img->Y3;
	int16 *Y4_Ptr = img->Y4;
	int16 *CB_Ptr = img->CB;
	int16 *CR_Ptr = img->CR;

	int16 *Y1Ptr = img->Y1 + 8;
	int16 *Y2Ptr = img->Y2 + 8;
	int16 *Y3Ptr = img->Y3 + 8;
	int16 *Y4Ptr = img->Y4 + 8;

	uint16 rows = jpeg_encoder_structure->rows;
	uint16 cols = jpeg_encoder_structure->cols;
	uint16 incr = jpeg_encoder_structure->incr;

	if (rows <= 8)
	{
		Y1_rows = rows;
		Y3_rows = 0;
	}
	else
	{
		Y1_rows = 8;
		Y3_rows = (uint16) (rows - 8);
	}

	if (cols <= 8)
	{
		Y1_cols = cols;
		Y2_cols = 0;
	}
	else
	{
		Y1_cols = 8;
		Y2_cols = (uint16) (cols - 8);
	}

	for (i=Y1_rows>>1; i>0; i--)
	{
		for (j=Y1_cols>>1; j>0; j--)
		{
			*Y1_Ptr++ = *input_ptr++;
			*Y1_Ptr++ = *input_ptr++;
			*Y1Ptr++ = *input_ptr++;
			*Y1Ptr++ = *input_ptr++;
			*CB_Ptr++ = *input_ptr++;
			*CR_Ptr++ = *input_ptr++;
		}

		for (j=Y2_cols>>1; j>0; j--)
		{
			*Y2_Ptr++ = *input_ptr++;
			*Y2_Ptr++ = *input_ptr++;
			*Y2Ptr++ = *input_ptr++;
			*Y2Ptr++ = *input_ptr++;
			*CB_Ptr++ = *input_ptr++;
			*CR_Ptr++ = *input_ptr++;
		}

		if (cols <= 8)
		{
			for (j=8-Y1_cols; j>0; j--)
			{
				*Y1_Ptr = *(Y1_Ptr - 1);
				Y1_Ptr++;
				*Y1Ptr = *(Y1Ptr - 1);
				Y1Ptr++;
			}

			for (j=8; j>0; j--)
			{
				*Y2_Ptr = *(Y1_Ptr - 1);
				Y2_Ptr++;
				*Y2Ptr = *(Y1Ptr - 1);
				Y2Ptr++;
			}
		}
		else
		{
			for (j=8-Y2_cols; j>0; j--)
			{
				*Y2_Ptr = *(Y2_Ptr - 1);
				Y2_Ptr++;
				*Y2Ptr = *(Y2Ptr - 1);
				Y2Ptr++;
			}
		}

		for (j=(16-cols)>>1; j>0; j--)
		{
			*CB_Ptr = *(CB_Ptr-1);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr-1);
			CR_Ptr++;
		}

		Y1_Ptr += 8;
		Y2_Ptr += 8;
		Y1Ptr += 8;
		Y2Ptr += 8;

		input_ptr += incr;
	}

	for (i=Y3_rows>>1; i>0; i--)
	{
		for (j=Y1_cols>>1; j>0; j--)
		{
			*Y3_Ptr = *input_ptr++;
			Y3_Ptr++;
			*Y3_Ptr = *input_ptr++;
			Y3_Ptr++;
			*Y3Ptr = *input_ptr++;
			Y3Ptr++;
			*Y3Ptr = *input_ptr++;
			Y3Ptr++;
			*CB_Ptr = *input_ptr++;
			CB_Ptr++;
			*CR_Ptr = *input_ptr++;
			CR_Ptr++;
		}

		for (j=Y2_cols>>1; j>0; j--)
		{
			*Y4_Ptr = *input_ptr++;
			Y4_Ptr++;
			*Y4_Ptr = *input_ptr++;
			Y4_Ptr++;
			*Y4Ptr = *input_ptr++;
			Y4Ptr++;
			*Y4Ptr = *input_ptr++;
			Y4Ptr++;
			*CB_Ptr = *input_ptr++;
			CB_Ptr++;
			*CR_Ptr = *input_ptr++;
			CR_Ptr++;
		}

		if (cols <= 8)
		{
			for (j=8-Y1_cols; j>0; j--)
			{
				*Y3_Ptr = *(Y3_Ptr - 1);
				Y3_Ptr++;
				*Y3Ptr = *(Y3Ptr - 1);
				Y3Ptr++;
			}

			for (j=8; j>0; j--)
			{
				*Y4_Ptr = *(Y3_Ptr - 1);
				Y4_Ptr++;
				*Y4Ptr = *(Y3Ptr - 1);
				Y4Ptr++;
			}
		}
		else
		{
			for (j=8-Y2_cols; j>0; j--)
			{
				*Y4_Ptr = *(Y4_Ptr - 1);
				Y4_Ptr++;
				*Y4Ptr = *(Y4Ptr - 1);
				Y4Ptr++;
			}
		}

		for (j=(16-cols)>>1; j>0; j--)
		{
			*CB_Ptr = *(CB_Ptr-1);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr-1);
			CR_Ptr++;
		}

		Y3_Ptr += 8;
		Y4_Ptr += 8;
		Y3Ptr += 8;
		Y4Ptr += 8;

		input_ptr += incr;
	}

	if (rows <= 8)
	{
		for (i=8-rows; i>0; i--)
		{
			for (j=8; j>0; j--)
			{
				*Y1_Ptr = *(Y1_Ptr - 8);
				Y1_Ptr++;
				*Y2_Ptr = *(Y2_Ptr - 8);
				Y2_Ptr++;
			}
		}

		for (i=8; i>0; i--)
		{
			Y1_Ptr -= 8;
			Y2_Ptr -= 8;

			for (j=8; j>0; j--)
			{
				*Y3_Ptr = *Y1_Ptr++;
				Y3_Ptr++;
				*Y4_Ptr = *Y2_Ptr++;
				Y4_Ptr++;
			}
		}
	}
	else
	{
		for (i=(16-rows); i>0; i--)
		{
			for (j=8; j>0; j--)
			{
				*Y3_Ptr = *(Y3_Ptr - 8);
				Y3_Ptr++;
				*Y4_Ptr = *(Y4_Ptr - 8);
				Y4_Ptr++;
			}
		}
	}

	for (i=((16-rows)>>1); i>0; i--)
	{
		for (j=8; j>0; j--)
		{
			*CB_Ptr = *(CB_Ptr-8);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr-8);
			CR_Ptr++;
		}
	}
}

void read_422_format (struct IMGDATA *img, JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure, uint8 *input_ptr)
{
	int32 i, j;
	uint16 Y1_cols, Y2_cols;

	int16 *Y1_Ptr = img->Y1;
	int16 *Y2_Ptr = img->Y2;
	int16 *CB_Ptr = img->CB;
	int16 *CR_Ptr = img->CR;

	uint16 rows = jpeg_encoder_structure->rows;
	uint16 cols = jpeg_encoder_structure->cols;
	uint16 incr = jpeg_encoder_structure->incr;

	if (cols <= 8)
	{
		Y1_cols = cols;
		Y2_cols = 0;
	}
	else
	{
		Y1_cols = 8;
		Y2_cols = (uint16) (cols - 8);
	}

	for (i=rows; i>0; i--)
	{
		for (j=Y1_cols>>1; j>0; j--)
		{
			*Y1_Ptr = *input_ptr;
			Y1_Ptr++; input_ptr++;
			*CB_Ptr = *input_ptr;
			CB_Ptr++; input_ptr++;
			*Y1_Ptr = *input_ptr;
			Y1_Ptr++; input_ptr++;
			*CR_Ptr = *input_ptr;
			CR_Ptr++; input_ptr++;
		}

		for (j=Y2_cols>>1; j>0; j--)
		{
			*Y2_Ptr = *input_ptr;
			Y2_Ptr++; input_ptr++;
			*CB_Ptr = *input_ptr;
			CB_Ptr++; input_ptr++;
			*Y2_Ptr = *input_ptr;
			Y2_Ptr++; input_ptr++;
			*CR_Ptr = *input_ptr;
			CR_Ptr++; input_ptr++;
		}

		if (cols <= 8)
		{
			for (j=8-Y1_cols; j>0; j--) {
				*Y1_Ptr = *(Y1_Ptr - 1);
				Y1_Ptr++;
			}


			for (j=8-Y2_cols; j>0; j--) {
				*Y2_Ptr = *(Y1_Ptr - 1);
				Y2_Ptr++;
			}
		}
		else
		{
			for (j=8-Y2_cols; j>0; j--) {
				*Y2_Ptr = *(Y2_Ptr - 1);
				Y2_Ptr++;
			}
		}

		for (j=(16-cols)>>1; j>0; j--)
		{
			*CB_Ptr = *(CB_Ptr-1);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr-1);
			CR_Ptr++;
		}

		input_ptr += incr;
	}

	for (i=8-rows; i>0; i--)
	{
		for (j=8; j>0; j--)
		{
			*Y1_Ptr = *(Y1_Ptr - 8);
			Y1_Ptr++;
			*Y2_Ptr = *(Y2_Ptr - 8);
			Y2_Ptr++;
			*CB_Ptr = *(CB_Ptr - 8);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr - 8);
			CR_Ptr++;
		}
	}
}

void read_444_format (struct IMGDATA *img, JPEG_ENCODER_STRUCTURE *jpeg_encoder_structure, uint8 *input_ptr)
{
	int32 i, j;
	int16 *Y1_Ptr = img->Y1;
	int16 *CB_Ptr = img->CB;
	int16 *CR_Ptr = img->CR;

	uint16 rows = jpeg_encoder_structure->rows;
	uint16 cols = jpeg_encoder_structure->cols;
	uint16 incr = jpeg_encoder_structure->incr;

	for (i=rows; i>0; i--)
	{
		for (j=cols; j>0; j--)
		{
			*Y1_Ptr = *input_ptr++;
			Y1_Ptr++;
			*CB_Ptr = *input_ptr++;
			CB_Ptr++;
			*CR_Ptr = *input_ptr++;
			CR_Ptr++;
		}

		for (j=8-cols; j>0; j--)
		{
			*Y1_Ptr = *(Y1_Ptr-1);
			Y1_Ptr++;
			*CB_Ptr = *(CB_Ptr-1);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr-1);
			CR_Ptr++;
		}

		input_ptr += incr;
	}

	for (i=8-rows; i>0; i--)
	{
		for (j=8; j>0; j--)
		{
			*Y1_Ptr = *(Y1_Ptr - 8);
			Y1_Ptr++;
			*CB_Ptr = *(CB_Ptr - 8);
			CB_Ptr++;
			*CR_Ptr = *(CR_Ptr - 8);
			CR_Ptr++;
		}
	}
}

void BGR_2_444 (uint8 *input_ptr, uint8 *output_ptr, uint32 image_width, uint32 image_height)
{
	uint32 i, size;
	uint8 R, G, B;
	int32 Y, Cb, Cr;

	size = image_width * image_height;

	for (i=size; i>0; i--)
	{
		B = *input_ptr++;
		G = *input_ptr++;
		R = *input_ptr++;

		Y = ((77 * R + 150 * G + 29 * B) >> 8);
		Cb = ((-43 * R - 85 * G + 128 * B) >> 8) + 128;
		Cr = ((128 * R - 107 * G - 21 * B) >> 8) + 128;

		if (Y < 0)
			Y = 0;
		else if (Y > 255)
			Y = 255;

		if (Cb < 0)
			Cb = 0;
		else if (Cb > 255)
			Cb = 255;

		if (Cr < 0)
			Cr = 0;
		else if (Cr > 255)
			Cr = 255;

		*output_ptr++ = (uint8) Y;
		*output_ptr++ = (uint8) Cb;
		*output_ptr++ = (uint8) Cr;
	}
}
