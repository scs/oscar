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

/*! @file Bilinear debayering to BGR format on host. */
/* Helper functions */
#include "DebayerBilinear.h"


/*********************************************************************//*!
 * @brief Assemble a row from a BGBG bayer order source for the output 
 *        BGR image.
 * 
 * Special treatment has to be applied to the border cases (left and right
 * border).
 * @param pDstRow Output row (BGR format)
 * @param pVertAvg Preprocessed vertical average
 * @param pHorizAvg Preprocessed horizontal average
 * @param pXAvg Preprocessed X average
 * @param pPlusAvg Preprocessed + average
 * @param pSrcRow Current source row in BGBG bayer order.
 * @param width Width of the row.
 *
 *//*********************************************************************/
static void bgbgToBgr(uint8 *pDstRow,
		      const uint8 *pVertAvg,
		      const uint8 *pHorizAvg,
		      const uint8 *pXAvg,
		      const uint8 *pPlusAvg,
		      const uint8 *pSrcRow,
		      uint32 width)
{
  uint32 pix;

  /******* First Blue pixel *******/
  // Blue color of first blue pixel
  *pDstRow++ = *pSrcRow++;

  // Green color of first blue pixel
  *pDstRow++ = ((2 * *pVertAvg++) + *pSrcRow)/3;

  // Red color of first blue pixel
  *pDstRow++ = *pVertAvg;

  pHorizAvg++;
  pXAvg++;
  pPlusAvg++;

  for(pix = 0; pix < width - 2; )
  {
    /********** Green pixel ***********/
    // Blue color of green pixel
    *pDstRow++ = *pHorizAvg++;

    // Green color of green pixel
    *pDstRow++ = *pSrcRow++;

    // Red color of green pixel
    *pDstRow++ = *pVertAvg++;

    pXAvg++;
    pPlusAvg++;
    pix++;

    /********** Blue pixel ***********/
    // Blue color of blue pixel
    *pDstRow++ = *pSrcRow++;

    // Green color of blue pixel
    *pDstRow++ = *pPlusAvg++;

    // Red color of blue pixel
    *pDstRow++ = *pXAvg++;

    pVertAvg++;
    pHorizAvg++;
    pix++;
  }

  /******* Last green pixel *********/  
  // Blue color of last green pixel
  *pDstRow++ = pSrcRow[-1];

  // Green color of last green pixel
  *pDstRow++ = *pSrcRow++;

  // Red color of last green pixel
  *pDstRow++ = *pVertAvg++;
}

/*********************************************************************//*!
 * @brief Assemble a row from a GRGR bayer order source for the output 
 *        BGR image.
 * 
 * Special treatment has to be applied to the border cases (left and right
 * border).
 * @param pDstRow Output row (BGR format)
 * @param pVertAvg Preprocessed vertical average
 * @param pHorizAvg Preprocessed horizontal average
 * @param pXAvg Preprocessed X average
 * @param pPlusAvg Preprocessed + average
 * @param pSrcRow Current source row in GRGR bayer order.
 * @param width Width of the row.
 *
 *//*********************************************************************/
static void grgrToBgr(uint8 *pDstRow,
		      const uint8 *pVertAvg,
		      const uint8 *pHorizAvg,
		      const uint8 *pXAvg,
		      const uint8 *pPlusAvg,
		      const uint8 *pSrcRow,
		      uint32 width)
{
  uint32 pix;

  /******* First Green pixel *******/
  // Blue color of first green pixel
  *pDstRow++ = *pVertAvg++;

  // Green color of first green pixel
  *pDstRow++ = *pSrcRow++;

  // Red color of first green pixel
  *pDstRow++ = *pSrcRow;

  pHorizAvg++;
  pXAvg++;
  pPlusAvg++;

  for(pix = 0; pix < width - 2; )
  {
    /********** Red pixel ***********/
    // Blue color of red pixel
    *pDstRow++ = *pXAvg++;

    // Green color of red pixel
    *pDstRow++ = *pPlusAvg++;

    // Red color of red pixel
    *pDstRow++ = *pSrcRow++;

    pVertAvg++;
    pHorizAvg++;
    pix++;

    /********** Green pixel ***********/
    // Blue color of green pixel
    *pDstRow++ = *pVertAvg++;

    // Green color of green pixel
    *pDstRow++ = *pSrcRow++;

    // Red color of green pixel
    *pDstRow++ = *pHorizAvg++;

    pXAvg++;
    pPlusAvg++;
    pix++;
  }

  /******* Last red pixel *********/  
  // Blue color of last red pixel
  *pDstRow++ = pVertAvg[-1];

  // Green color of last red pixel
  *pDstRow++ = ((*pVertAvg * 2) + pSrcRow[-1])/3;

  // Red color of last red pixel
  *pDstRow++ = *pSrcRow++;
}

OSC_ERR DebayerBilinearBGR(uint8 *pDst, 
			   uint8 *pSrc, 
			   uint32 width, 
			   uint32 height, 
			   uint8 *pTmp, 
			   enum EnBayerOrder enBayerOrder)
{
  uint32 row;
  uint8* pCurRow = pSrc;
  uint8* pOneRowUp = &pSrc[-width];
  uint8* pOneRowDown = &pSrc[width];
  enum EnBayerOrder enRowBayerOrder = enBayerOrder;
  
  for(row = 0; row < height; row++)
    {
      // First, do all the calculations (filters) for the current row
      // The first and the last row require special treatment.
      if(row == 0)
	{
	  debayerBilinearPreprocRowFirstOrLast(pTmp,
					       width,
					       pOneRowDown,
					       pCurRow);
	} else if(row == (height - 1)) {
	  debayerBilinearPreprocRowFirstOrLast(pTmp,
					       width,
					       pOneRowUp,
					       pCurRow);	
      } else {
	  debayerBilinearPreprocRow(pTmp,
				    width,
				    pOneRowUp,
				    pCurRow,
				    pOneRowDown);
      }

      // Then, pick out the value we need to the format we want.
      switch(enRowBayerOrder)
	{
	case ROW_BGBG:
	  bgbgToBgr(&pDst[row*width*3],
		    pTmp,
		    &pTmp[width],
		    &pTmp[2*width],
		    &pTmp[3*width],
		    pCurRow,
		    width);
	  enRowBayerOrder = ROW_GRGR;
	  break;
	case ROW_GRGR:
	  grgrToBgr(&pDst[row*width*3],
		    pTmp,
		    &pTmp[width],
		    &pTmp[2*width],
		    &pTmp[3*width],
		    pCurRow,
		    width);
	  enRowBayerOrder = ROW_BGBG;
	  break;
	default:
	  OscLog(ERROR, "%s: Unsupported bayer order encountered! (%d)\n",
		 __func__, enBayerOrder);
	  return;
	}
      pCurRow += width;
      pOneRowUp += width;
      pOneRowDown += width;
    }

}
