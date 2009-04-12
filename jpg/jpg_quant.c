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
#include "jpg_quantdata.h"

/*	This function implements 16 Step division for Q.15 format data */
uint16 DSP_Division (uint32 numer, uint32 denom)
{
	uint16 i;

	denom <<= 15;

	for (i=16; i>0; i--)
	{
		if (numer > denom)
		{
			numer -= denom;
			numer <<= 1;
			numer++;
		}
		else
			numer <<= 1;
	}

	return (uint16) numer;
}

/* Multiply Quantization table with quality factor to get LQT and CQT */
void initialize_quantization_tables (uint32 quality_factor)
{
	uint16 i, index;
	uint32 value;

	uint8 luminance_quant_table [] =
	{
		16, 11, 10, 16,  24,  40,  51,  61,
		12, 12, 14, 19,  26,  58,  60,  55,
		14, 13, 16, 24,  40,  57,  69,  56,
		14, 17, 22, 29,  51,  87,  80,  62,
		18, 22, 37, 56,  68, 109, 103,  77,
		24, 35, 55, 64,  81, 104, 113,  92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103,  99
	};

	uint8 chrominance_quant_table [] =
	{
		17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99
	};

	for (i=0; i<64; i++)
	{
		index = zigzag_table [i];

		/* luminance quantization table * quality factor */
		value = luminance_quant_table [i] * quality_factor;
		value = (value + 0x200) >> 10;

		if (value == 0)
			value = 1;
		else if (value > 255)
			value = 255;

		Lqt [index] = (uint8) value;
		ILqt [i] = DSP_Division (0x8000, value);

		/* chrominance quantization table * quality factor */
		value = chrominance_quant_table [i] * quality_factor;
		value = (value + 0x200) >> 10;

		if (value == 0)
			value = 1;
		else if (value > 255)
			value = 255;

		Cqt [index] = (uint8) value;
		ICqt [i] = DSP_Division (0x8000, value);
	}
}

/* multiply DCT Coefficients with Quantization table and store in ZigZag location */
void quantization (int16* const data, uint16* const quant_table_ptr, uint16 *Temp)
{
	int16 i;
	int32 value;

	for (i=63; i>=0; i--)
	{
		value = data [i] * quant_table_ptr [i];
		value = (value + 0x4000) >> 15;

		Temp [zigzag_table [i]] = (uint16) value;
	}
}
