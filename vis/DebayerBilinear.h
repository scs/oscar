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
#include "oscar.h"

/*! @file Private header file for bilinear debayering. */

#ifndef DEBAYER_BILINEAR_H_
#define DEBAYER_BILINEAR_H_

/*********************************************************************//*!
 * @brief Preprocess a row of the image by calculating all possible
 * interpolations in every pixel
 * 
 * For bilinear interpolation, there are 4 possible filters:
 * - Vertical average:     01 (02) 03  One row up
 *                         11 !12! 13  Current row
 *                         21 (22) 23  One row down
 * VertAvg[12] = (02 + 22)/2
 *
 * - Horizontal average:   01  02  03  One row up
 *                        (11)!12!(13) Current row
 *                         21  22  23  One row down
 * HorizAvg[12] = (11 + 13)/2
 *
 * - X average:           (01) 02 (03) One row up
 *                         11 !12! 13  Current row
 *                        (21) 22 (23) One row down
 * XAvg[12] = ((01 + 21)/2 + (03 + 23)/2)/2
 *
 * - + average:            01 (02) 03  One row up
 *                        (11)!12!(13) Current row
 *                         21 (22) 23  One row down
 * PlusAvg[12] = ((11 + 13)/2 + (02 + 22)/2)/2
 * 
 * The supplied temporary memory area is divided into 4 sections of size
 * <width> and the calculated filters are saved there per pixel in 
 * above error.
 * @see debayerBilinearPreprocRowFirstOrLast
 *
 * @param pTmp Pointer to temporary memory of size 4 * width
 * @param width Width of the row.
 * @param pOneRowUp Pointer to the source row one line above the currently
 *        preprocessed row
 * @param pCurRow Pointer to the currently preprocessed source row.
 * @param pOneRowDown Pointer to the source row one line below the currently
 *        preprocessed row
 *
 *//*********************************************************************/
void debayerBilinearPreprocRow(uint8 *pTmp, 
			       uint32 width, 
			       const uint8* pOneRowUp,
			       const uint8* pCurRow,
			       const uint8* pOneRowDown);

/*********************************************************************//*!
 * @brief Preprocess a border row of the image by calculating all possible
 * interpolations in every pixel
 * 
 * For bilinear interpolation, there are 4 possible filters:
 * - Vertical average:     01 (02) 03  One row up
 *                         11 !12! 13  Current row
 *                         21 (22) 23  One row down
 * VertAvg[12] = (02 + 22)/2
 *
 * - Horizontal average:   01  02  03  One row up
 *                        (11)!12!(13) Current row
 *                         21  22  23  One row down
 * HorizAvg[12] = (11 + 13)/2
 *
 * - X average:           (01) 02 (03) One row up
 *                         11 !12! 13  Current row
 *                        (21) 22 (23) One row down
 * XAvg[12] = ((01 + 21)/2 + (03 + 23)/2)/2
 *
 * - + average:            01 (02) 03  One row up
 *                        (11)!12!(13) Current row
 *                         21 (22) 23  One row down
 * PlusAvg[12] = ((11 + 13)/2 + (02 + 22)/2)/2
 *
 * The supplied temporary memory area is divided into 4 sections of size
 * <width> and the calculated filters are saved there per pixel in 
 * above error.
 * Since this version of the function is adapted to the border cases
 * (first or last row), the filters are truncated.
 * @see debayerBilinearPreprocRow
 *
 * @param pTmp Pointer to temporary memory of size 4 * width
 * @param width Width of the row.
 * @param pOneRowUpOrDown Pointer to the source row one line above or below 
 *        the currently preprocessed row, depending on whether this is
 *        called for the first or the last row of an image.
 * @param pCurRow Pointer to the currently preprocessed source row.
 *
 *//*********************************************************************/
void debayerBilinearPreprocRowFirstOrLast(uint8 *pTmp, 
					  uint32 width, 
					  const uint8* pOneRowUpOrDown,
					  const uint8* pCurRow);


/*********************************************************************//*!
 * @brief Debayer an image to BGR color format using bilinear debayering.
 * 
 * Function is assembler-optimized on target.
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
OSC_ERR DebayerBilinearBGR(uint8 *pDst, 
			   uint8 *pSrc, 
			   uint32 width, 
			   uint32 height, 
			   uint8 *pTmp, 
			   enum EnBayerOrder enBayerOrder);

/*! @brief Calculate the unbiased average between two values as does the Blackfin
  if the RND_MOD bit is set in the ASTAT register. */
#define UNBIASED_AVG(A, B) ((((A+B)%2) == 0) ? ((A+B)/2): ((((A+B)/2)%2 == 0) ? ((A+B)/2) : ((A+B)/2 + 1)))
/*! @brief Calculate the biased average between two values as does the Blackfin
  if the RND_MOD bit is NOT set in the ASTAT register. */
#define BIASED_AVG(A, B) ((A+B+1)/2)



#endif /* DEBAYER_BILINEAR_H_ */
