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

/*! @file oscar_types_target.h
 * @brief Type definitions for the target.
 * 
 */
#ifndef TYPES_H_
#define TYPES_H_

#define int8    char
#define uint8   unsigned char
#define int16   short
#define uint16  unsigned short
#define int32   long
#define uint32  unsigned long
#define int64   long long
#define uint64  unsigned long long

#define TRUE    (1==1)
#define FALSE   (!TRUE)

#define OSC_ERR int

#define bool    short

/*! @brief Endianness of this machine. Blackfin uses LITTLE_ENDIAN */
#define CPU_LITTLE_ENDIAN

/*! @brief Macro to swap the endianness of a 16 bit number */
#define ENDIAN_SWAP_16(x)       \
	(                           \
			(x>>8) | (x<<8)     \
	)
/*! @brief Macro to swap the endianness of a 32 bit number */
#define ENDIAN_SWAP_32(x)               \
	(                                   \
			(x>>24) |                   \
			((x<<8) & 0x00FF0000) |     \
			((x>>8) & 0x0000FF00) |     \
			(x<<24)                     \
	)
/*! @brief Macro to swap the endianness of a 64 bit number */
#define ENDIAN_SWAP_64(x)                           \
	(                                               \
			(x>>56) |                               \
			((x<<40) & 0x00FF000000000000) |        \
			((x<<24) & 0x0000FF0000000000) |        \
			((x<<8)  & 0x000000FF00000000) |        \
			((x>>8)  & 0x00000000FF000000) |        \
			((x>>24) & 0x0000000000FF0000) |        \
			((x>>40) & 0x000000000000FF00) |        \
			(x<<56)                                 \
	)

/*! @brief Load a 32 bit value from a non-aligned address. */
#define LD_INT32(pData) \
	( \
			((int32)((pData)[0])) | (((int32)((pData)[1])) << 8) | \
			(((int32)((pData)[2])) << 16) | (((int32)((pData)[3])) << 24)\
	)

/*! @brief Write a 32 bit value to a non-aligned address. */
#define ST_INT32(pData, val) \
{ \
	((pData)[0] = (uint8)(val & 0x000000FF));   \
	((pData)[1] = (uint8)((val & 0x0000FF00) >> 8));   \
	((pData)[2] = (uint8)((val & 0x00FF0000) >> 16));   \
	((pData)[3] = (uint8)((val & 0xFF000000) >> 24));     \
}

/*! @brief Load a 16 bit value from a non-aligned address. */
#define LD_INT16(pData) \
	( \
			((int16)((pData)[0])) | (((int16)((pData)[1])) << 8) \
	)

/*! @brief Write a 16 bit value to a non-aligned address. */
#define ST_INT16(pData, val) \
{ \
	((pData)[0] = (uint8)(val & 0x00FF));      \
	((pData)[1] = (uint8)((val & 0xFF00) >> 8));      \
}

/*! @brief Represents the color depth of a picture */
enum EnOscPictureType {
	OSC_PICTURE_GREYSCALE,
	OSC_PICTURE_YUV_444,
	OSC_PICTURE_YUV_422,
	OSC_PICTURE_YUV_420,
	OSC_PICTURE_YUV_400,
	OSC_PICTURE_CHROM_U,
	OSC_PICTURE_CHROM_V,
	OSC_PICTURE_HUE,
	OSC_PICTURE_BGR_24,
	OSC_PICTURE_RGB_24
};

/*! @brief Structure representing an 8-bit picture */
struct OSC_PICTURE {
	void* data;                 /*!< @brief The actual image data */
	unsigned short width;       /*!< @brief Width of the picture */
	unsigned short height;      /*!< @brief Height of the picture */
	enum EnOscPictureType type; /*!< @brief The type of the picture */
};

/*! @brief The order in which the colored pixels of a bayer pattern
 * appear in a row.
 * 
 * The colors are abbreviated as follows:
 * - G: Green
 * - R: Red
 * - B: Blue
 * 
 * The enum is constructed from two bools; one saying whether the first
 * pixel in the row is green and the other whether it is a red or blue
 * row.
 *          firstGreen      firstOther
 * red          11              01
 * blue         10              00
 * */
enum EnBayerOrder
{
	ROW_BGBG = 0,
	ROW_RGRG = 1,
	ROW_GBGB = 2,
	ROW_GRGR = 3
};

#endif /*TYPES_H_*/
