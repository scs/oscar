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
#include "jpg_markdata.h"

/* Header for JPEG Encoder */

uint8* write_markers (uint8 *output_ptr, uint32 image_format, uint32 image_width, uint32 image_height)
{
	uint16 i, header_length;
	uint8 number_of_components;

	/* Start of image marker */
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xD8;

	/* Quantization table marker */
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xDB;

	/* Quantization table length */
	*output_ptr++ = 0x00;
	*output_ptr++ = 0x84;

	/* Pq, Tq */
	*output_ptr++ = 0x00;

	/* Lqt table */
	for (i=0; i<64; i++)
		*output_ptr++ = Lqt [i];

	/* Pq, Tq */
	*output_ptr++ = 0x01;

	/* Cqt table */
	for (i=0; i<64; i++)
		*output_ptr++ = Cqt [i];

	/* huffman table(DHT) */
	for (i=0; i<210; i++)
	{
		*output_ptr++ = (uint8) (markerdata [i] >> 8);
		*output_ptr++ = (uint8) markerdata [i];
	}

	if (image_format == OSC_PICTURE_YUV_400)
		number_of_components = 1;
	else
		number_of_components = 3;

	/* Frame header(SOF) */

	/* Start of frame marker */
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xC0;

	header_length = (uint16) (8 + 3 * number_of_components);

	/* Frame header length	 */
	*output_ptr++ = (uint8) (header_length >> 8);
	*output_ptr++ = (uint8) header_length;

	/* Precision (P) */
	*output_ptr++ = 0x08;

	/* image height */
	*output_ptr++ = (uint8) (image_height >> 8);
	*output_ptr++ = (uint8) image_height;

	/* image width */
	*output_ptr++ = (uint8) (image_width >> 8);
	*output_ptr++ = (uint8) image_width;

	/* Nf */
	*output_ptr++ = number_of_components;

	if (image_format == OSC_PICTURE_YUV_400)
	{
		*output_ptr++ = 0x01;
		*output_ptr++ = 0x11;
		*output_ptr++ = 0x00;
	}
	else
	{
		*output_ptr++ = 0x01;

		if (image_format == OSC_PICTURE_YUV_420)
			*output_ptr++ = 0x22;
		else if (image_format == OSC_PICTURE_YUV_422)
			*output_ptr++ = 0x21;
		else
			*output_ptr++ = 0x11;

		*output_ptr++ = 0x00;

		*output_ptr++ = 0x02;
		*output_ptr++ = 0x11;
		*output_ptr++ = 0x01;

		*output_ptr++ = 0x03;
		*output_ptr++ = 0x11;
		*output_ptr++ = 0x01;
	}

	/* Scan header(SOF) */

	/* Start of scan marker */
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xDA;

	header_length = (uint16) (6 + (number_of_components << 1));

	/* Scan header length */
	*output_ptr++ = (uint8) (header_length >> 8);
	*output_ptr++ = (uint8) header_length;

	/* Ns */
	*output_ptr++ = number_of_components;

	if (image_format == OSC_PICTURE_YUV_400)
	{
		*output_ptr++ = 0x01;
		*output_ptr++ = 0x00;
	}
	else
	{
		*output_ptr++ = 0x01;
		*output_ptr++ = 0x00;

		*output_ptr++ = 0x02;
		*output_ptr++ = 0x11;

		*output_ptr++ = 0x03;
		*output_ptr++ = 0x11;
	}

	*output_ptr++ = 0x00;
	*output_ptr++ = 0x3F;
	*output_ptr++ = 0x00;
	return output_ptr;
}
