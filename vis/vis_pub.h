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

/*! @file vis_pub.h
 * @brief API definition for vision library module
 * 
 */
#ifndef VIS_PUB_H_
#define VIS_PUB_H_

#include "oscar_error.h"
#ifdef OSC_HOST
	#include "oscar_types_host.h"
	#include "oscar_host.h"
#else
	#include "oscar_types_target.h"
	#include "oscar_target.h"
#endif /* OSC_HOST */

#include "bayer_pub.h"

/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscVisCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscVisDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to an BGR output image.
 * 
 * Debayering calculates the missing color values by interpolating
 * between neighboring pixels. In this case, linear interpolation with
 * LaPlace correction is used in accordance with the algorithm
 * described in
 * http://scien.stanford.edu/class/psych221/projects/99/tingchen/main.htm
 * (Interpolation with color correction I).
 * This is one of the best algorithms for quality but does require more
 * processing than a simple one.
 * 
 * The bayer pattern color order of the first row is specified with an
 * identifier.
 * 
 * ! Only even widths are supported !
 * 
 * The output picture has 24 bit color depth with color order B G R and
 * the same width and height as the input image. Color representation
 * is suboptimal at the border pixels.
 * 
 * @param pRaw Pointer to the raw input picture of size width x height.
 * @param width Width of the input and output image.
 * @param height Height of the input and output image.
 * @param enBayerOrderFirstRow The order of the bayer pattern colors
 * in the first row of the image to be debayered. Can be queried by
 * OscCamGetBayerOrder().
 * @param pOut Pointer to the BGR output image.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscVisDebayer(const uint8* pRaw,
		const uint16 width,
		const uint16 height,
		const enum EnBayerOrder enBayerOrderFirstRow,
		uint8 *const pOut);

/*!
 * @brief Convert a raw image captured by a camera sensor with bayer filter to a greyscale output image with halvened dimensions.
 * 
 * Debayering calculates the missing color values by averaging over four colored cells. 2 green and one red and blue cell are weightened averaged to form a single greyscale cell.
 * 
 * The bayer pattern color order of the first row is specified with an identifier.
 * 
 * ! Only even widths and heights are supported !
 * 
 * The output picture has 8 bit greyscale cells with halve the with and height of the original image.
 * 
 * @param pRaw Pointer to the raw input picture of size width x height.
 * @param width Width of the input and output image.
 * @param height Height of the input and output image.
 * @param enBayerOrderFirstRow The order of the bayer pattern colors
 * in the first row of the image to be debayered. Can be queried by
 * OscCamGetBayerOrder().
 * @param pOut Pointer to the greyscale output image.
 * @return SUCCESS or an appropriate error code otherwise
 */
OSC_ERR OscVisDebayerGreyscaleHalfSize(uint8 const * const pRaw, uint16 const width, uint16 const height, enum EnBayerOrder const enBayerOrderFirstRow, uint8 * const color);

/*!
 * @brief Debayers an image at one spot and gives its mean color.
 * 
 * ! Only even size is supported !
 * 
 * @param pRaw Pointer to the raw input picture of size width x height.
 * @param width Width of the input and output image.
 * @param height Height of the input and output image.
 * @param enBayerOrderFirstRow The order of the bayer pattern colors in the first row of the image to be debayered. Can be queried by OscCamGetBayerOrder().
 * @param xPos Left border of the spot.
 * @param yPos Upper boder of the spot.
 * @param size Size of the sport in pixles, this must be an even value.
 * @param color Pointer to a 3-element field where the color is written.
 * @return SUCCESS or an appropriate error code otherwise
 */
OSC_ERR OscVisDebayerSpot(uint8 const * const pRaw, uint16 const width, uint16 const height, enum EnBayerOrder enBayerOrderFirstRow, uint16 const xPos, uint16 const yPos, uint16 const size, uint8 * color);

#endif /*VIS_PUB_H_*/
