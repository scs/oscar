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
#ifndef OSCAR_VIS_INCLUDE_H_
#define OSCAR_VIS_INCLUDE_H_

/*! @file
 * @brief API definition for vision library module
 * 
 */

extern struct OscModule OscModule_vis;

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
 * @param color Pointer to the greyscale output image.
 * @return SUCCESS or an appropriate error code otherwise
 */
OSC_ERR OscVisDebayerGreyscaleHalfSize(uint8 const * const pRaw, uint16 const width, uint16 const height, enum EnBayerOrder const enBayerOrderFirstRow, uint8 * const color);

/*!
 * @brief Convert a raw image captured by a camera sensor with bayer filter to a color output image with halvened dimensions.
 * 
 * The bayer pattern color order of the first row is specified with an identifier.
 * 
 * ! Only even widths and heights are supported !
 * 
 * The output picture has 24 bit color cells with halve the with and height of the original image.
 * 
 * @param pRaw Pointer to the raw input picture of size width x height.
 * @param width Width of the input and output image.
 * @param height Height of the input and output image.
 * @param enBayerOrderFirstRow The order of the bayer pattern colors
 * in the first row of the image to be debayered. Can be queried by
 * OscCamGetBayerOrder().
 * @param pOut Pointer to the color output image.
 * @return SUCCESS or an appropriate error code otherwise
 */
OSC_ERR OscVisDebayerHalfSize(uint8 const * const pRaw, uint16 const width, uint16 const height, enum EnBayerOrder const enBayerOrderFirstRow, uint8 * const pOut);

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


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to an RGB output image.
 * 
 * Very simple debayering. Makes one colour pixel out of 4 bayered pixels
 * This means that the resulting image is only width/2 by height/2 pixels
 * Image size is reduced by a factor of 4!
 * This needs about 6ms for a full 752x480 frame on leanXcam
 * The image is returned in RGB24 Format
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerRGB(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a luminance "Y" output image.
 * 
 * This function performs a very simple debayering and makes one 
 * luminance "Y" pixel out of 4 bayered pixels. This means that the
 * resulting image is only width/2 by height/2 pixels. Only even width
 * and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel greyscale format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerLumY(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);

/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a greyscale output image.
 * 
 * This function performs a very simple debayering and makes one 
 * greyscale pixel out of 4 bayered pixels. This means that the
 * resulting image is only width/2 by height/2 pixels. Image size is 
 * reduced by a factor of 4! 
 * On the target, this function is assembler-optimized for the Blackfin
 * processor and makes use of its video-pixel operations. On the host, this
 * function just calls OscVisFastDebayerGrey()
 * Because of vector operations, width must be a multiple of 4 and the
 * input and output memory for the images must be 4-byte aligned!
 * Height must be a multiple of 2!
 *
 * Time: ~3.1 ms for 752x480 raw to 376x240 grey (in SDRAM)
 *
 * @see OscVisFastDebayerGrey
 *
 * The image is returned in 8 Bit/Pixel greyscale format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisVectorDebayerGrey(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);

/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a greyscale output image.
 * 
 * This function performs a very simple debayering and makes one 
 * greyscale pixel out of 4 bayered pixels. This means that the
 * resulting image is only width/2 by height/2 pixels. Only even width
 * and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel greyscale format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerGrey(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a chrominance "U" output image.
 * 
 * This function performs a very simple debayering and makes one 
 * chrominance "U" pixel out of 4 bayered pixels. This means that the
 * resulting image is only width/2 by height/2 pixels. Only even width
 * and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel Format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerChromU(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a chrominance "V" output image.
 * 
 * This function performs a very simple debayering and makes one 
 * chrominance "V" pixel out of 4 bayered pixels. This means that the
 * resulting image is only width/2 by height/2 pixels. Only even width
 * and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel Format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerChromV(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a YUV422 output image.
 * 
 * Very simple debayering. Makes one colour pixel out of 4 bayered pixels
 * This means that the resulting image is only width/2 by height/2 pixels
 * Image size is reduced by a factor of 4!
 * And returns the image in YUV422 Format
 * The macro pixel is stored in UYVY order (equal to Y422 and UYNV and HDYC 
 * according to www.fourcc.org. The fourcc hexcode is 0x59565955
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerYUV422(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a hue "H" output image.
 * 
 * This function performs a very simple debayering and makes one 
 * hue pixel out of 4 bayered pixels according to the HSL color space.
 * This means that the resulting image is only width/2 by height/2 pixels.
 * Only even width and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel Format.
 * 
 * The HSL space color value 0 is mapped to the output value 0.
 * The HSL space color value 359 is mapped to the output value 255.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerHSL_H(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a saturation output image.
 * 
 * This function performs a very simple debayering and makes one 
 * saturation pixel out of 4 bayered pixels according to the HSL color space.
 * This means that the resulting image is only width/2 by height/2 pixels.
 * Only even width and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel Format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerHSL_S(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Convert a raw image captured by a camera sensor with bayer
 * filter to a luminance output image.
 * 
 * This function performs a very simple debayering and makes one 
 * luminance pixel out of 4 bayered pixels according to the HSL color space.
 * This means that the resulting image is only width/2 by height/2 pixels.
 * Only even width and height are supported. Image size is reduced by a factor of 4!
 * The image is returned in 8 Bit/Pixel Format.
 * 
 * @param pRaw Pointer to an OSC_PICTURE structure which contains the raw input picture of size width x height.
 * @param pOut Pointer to the result OSC_PICTURE structure of size (width/2) x (height/2).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFastDebayerHSL_L(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);


/*********************************************************************//*!
 * @brief Sobel edge filter.
 * 
 * This function calculates the sobel operator in x and y direction and combines
 * the resulting gradients to a saturated 8-Bit gradient magnitude. Instead of 
 * the usual magnitude formula G = sqrt(Gx² + Gy²) with the resource consuming
 * square root function, a simple division by a power of two is used: G = (Gx² + Gy²) * 2^(-exp).
 * 
 * This function leaves a one pixel wide border of the output image untouched.
 * Unless these pixels are treated outside this function, they might be in an undefined state.
 * 
 * @param pIn Pointer to the input picture data.
 * @param pOut Pointer to the output picture data.
 * @param width Image width.
 * @param height Image height.
 * @param exp Squared gradient magnitude division power.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisSobel(const uint8 *pIn, uint8 *pOut, const uint16 width, const uint16 height, const uint8 exp);

/*********************************************************************//*!
 * @brief Debayer an image to BGR color format using bilinear debayering.
 * 
 * Function is assembler-optimized on target. (~14 ms for 752x480)
 * Currently only the bayer orders BGBG and GRGR are supported (which are
 * the ones normally encountered when reading an image from the CMOS
 * sensor).
 * The result is an image with the format OSC_PICTURE_BGR_24.
 *
 * @param pDst The destination image (size: width x height x 3)
 * @param pSrc The source image (size: width x height)
 * @param width Width of the source image. Must be multiple of 4 and greater
 * zero.
 * @param height Height of the source image
 * @param pTmp Temporary memory for intermediate calculations
 * (size: width x 4)
 * @param enBayerOrder The order of the bayer pattern in the source image.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisDebayerBilinearBGR(uint8 *pDst, 
				 uint8 *pSrc, 
				 uint32 width, 
				 uint32 height, 
				 uint8 *pTmp, 
				 enum EnBayerOrder enBayerOrder);

#endif // #ifndef OSCAR_VIS_INCLUDE_H_
