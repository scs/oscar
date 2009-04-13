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

/*! @file oscar.h
 * @brief API definition for Oscar framework
 * 
 * Must be included by the application.
 */

#ifndef OSCAR_MAIN_HEADER_FILE_WHICH_MAY_BE_INCLUDED_FROM_INSIDE_A_MODULE_H_
#define OSCAR_MAIN_HEADER_FILE_WHICH_MAY_BE_INCLUDED_FROM_INSIDE_A_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/* This saves to characters ... */
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef int OSC_ERR;
typedef bool BOOL;

#define TRUE true
#define FALSE false

#if defined(OSC_HOST)
/* Defined as stumps because it is needed in code shared by target and
 * host. */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x) (x)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x) (x)
#elif defined(OSC_TARGET)
/* Bluntly copied from linux/compiler.h from uclinux */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x) __builtin_expect(!!(x), 1)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#error "Neither OSC_HOST nor OSC_TARGET is defined as a macro."
#endif

/*! @brief Endianness of this machine. Intel uses LITTLE_ENDIAN */
#define CPU_LITTLE_ENDIAN

/*! @brief Macro to swap the endianness of a 16 bit number */
#define ENDIAN_SWAP_16(x) ((x>>8) | (x<<8))

/*! @brief Macro to swap the endianness of a 32 bit number */
#define ENDIAN_SWAP_32(x) ( \
	(x>>24) | \
	((x<<8) & 0x00FF0000) | \
	((x>>8) & 0x0000FF00) | \
	(x<<24) \
)

/*! @brief Macro to swap the endianness of a 64 bit number */
#define ENDIAN_SWAP_64(x) ( \
	(x>>56) | \
	((x<<40) & 0x00FF000000000000) | \
	((x<<24) & 0x0000FF0000000000) | \
	((x<<8)  & 0x000000FF00000000) | \
	((x>>8)  & 0x00000000FF000000) | \
	((x>>24) & 0x0000000000FF0000) | \
	((x>>40) & 0x000000000000FF00) | \
	(x<<56) \
)

/*! @brief Load a 32 bit value from a non-aligned address. */
#define LD_INT32(pData) ( \
	((int32)((pData)[0])) | (((int32)((pData)[1])) << 8) | \
	(((int32)((pData)[2])) << 16) | (((int32)((pData)[3])) << 24) \
)

/*! @brief Write a 32 bit value to a non-aligned address. */
#define ST_INT32(pData, val) { \
	((pData)[0] = (uint8)(val & 0x000000FF)); \
	((pData)[1] = (uint8)((val & 0x0000FF00) >> 8)); \
	((pData)[2] = (uint8)((val & 0x00FF0000) >> 16)); \
	((pData)[3] = (uint8)((val & 0xFF000000) >> 24)); \
}

/*! @brief Load a 16 bit value from a non-aligned address. */
#define LD_INT16(pData) (((int16)((pData)[0])) | (((int16)((pData)[1])) << 8))

/*! @brief Write a 16 bit value to a non-aligned address. */
#define ST_INT16(pData, val) { \
	((pData)[0] = (uint8)(val & 0x00FF)); \
	((pData)[1] = (uint8)((val & 0xFF00) >> 8)); \
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
	void * data;                /*!< @brief The actual image data */
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
enum EnBayerOrder {
	ROW_BGBG = 0,
	ROW_RGRG = 1,
	ROW_GBGB = 2,
	ROW_GRGR = 3
};

#include "../oscar_version.h"
#include "../oscar_error.h"
#include "../oscar_dependencies.h"

/*********************************************************************//*!
 * @brief Constructor for framework
 * 
 * @param phFw Pointer to the handle location for the framework
 * @return SUCCESS or appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCreate(void ** phFw);

/*********************************************************************//*!
 * @brief Destructor for framework
 * 
 * Fails if not all loaded modules have been destroyed.
 * 
 * @param hFw Pointer to the handle of the framework to be destroyed.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Get framework version numbers
 * 
 * Used scheme: major.minor[.revsion]
 * 
 * The major number is used for significant changes in functionality or 
 * supported plattform. Instable pre releases use a major number of 0.
 * The minor number decodes small feature changes.
 * The patch number is intended for bug fixes without changes of API.
 * 
 * @param hMajor Pointer to major version number.
 * @param hMinor Pointer to minor version number.
 * @param hPatch Pointer to patch number.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGetVersionNumber(char *hMajor, char *hMinor, char *hPatch);

/*********************************************************************//*!
 * @brief Get framework version string
 * 
 * Version string format: v<major>.<minor>[-p<patch>]  eg: v1.3  or v1.3-p1
 * The patch number is not printed if no bug-fixes are available (patch=0).
 *  
 * See @see OscGetVersionNumber for number interpretation.
 * 
 * @param hMajor Pointer to formated version string.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGetVersionString(char *hVersion);

/* Include the public header files of the different modules, which
 * contain the declarations of the API functions of the respective
 * module.
 */
#include "log.h"
#include "dspl.h"
#include "dma.h"
#include "ipc.h"
#include "bmp.h"
#include "sup.h"
#ifndef TARGET_TYPE_MESA_SR4K
#include "cam.h"
#include "cpld.h"
#include "sim.h"
#include "swr.h"
#include "srd.h"
#include "frd.h"
#include "hsm.h"
#include "cfg.h"
#include "clb.h"
#include "vis.h"
#include "gpio.h"
#include "jpg.h"
#endif
#ifdef TARGET_TYPE_INDXCAM
#include "../lgx/lgx_pub.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /*OSCAR_MAIN_HEADER_FILE_WHICH_MAY_BE_INCLUDED_FROM_INSIDE_A_MODULE_H_*/
