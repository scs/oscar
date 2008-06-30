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
 * filter to an RGB output image.
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

#endif /*VIS_PUB_H_*/
