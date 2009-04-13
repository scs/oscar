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

#ifndef OSCAR_INCLUDE_OSCAR_H_
#define OSCAR_INCLUDE_OSCAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Support file for the NIH design pattern. */
#include "nih.h"

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

#endif /* OSCAR_INCLUDE_OSCAR_H_ */
