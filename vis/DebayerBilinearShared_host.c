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

/*! @file Shared helper functions for bilinear debayering on the host. */

#include "DebayerBilinear.h"

void debayerBilinearPreprocRow(uint8 *pTmp, 
			       uint32 width, 
			       const uint8* pOneRowUp,
			       const uint8* pCurRow,
			       const uint8* pOneRowDown)
{
  uint32 pix;
  uint8* pVertAvg = pTmp;
  uint8* pHorizAvg = &pTmp[width];
  uint8* pXAvg = &pTmp[2*width];
  uint8* pPlusAvg = &pTmp[3*width];

  // Vertical average
  for(pix = 0; pix < width; pix++)
    {
      pVertAvg[pix] = BIASED_AVG(pOneRowUp[pix], pOneRowDown[pix]);
    }
  // Horizontal average
  for(pix = 1; pix < width - 1; pix++)
    {
      pHorizAvg[pix] = BIASED_AVG(pCurRow[pix - 1], pCurRow[pix + 1]);
    }
  // X average
  for(pix = 1; pix < width - 1; pix++)
    {
      pXAvg[pix] = BIASED_AVG(pVertAvg[pix - 1], pVertAvg[pix + 1]);
    }
  // + average
  for(pix = 1; pix < width - 1; pix++)
    {
      pPlusAvg[pix] = BIASED_AVG(pHorizAvg[pix], pVertAvg[pix]);
    }
}

void debayerBilinearPreprocRowFirstOrLast(uint8 *pTmp, 
					  uint32 width, 
					  const uint8* pOneRowUpOrDown,
					  const uint8* pCurRow)
{
  uint32 pix;
  uint8* pVertAvg = pTmp;
  uint8* pHorizAvg = &pTmp[width];
  uint8* pXAvg = &pTmp[2*width];
  uint8* pPlusAvg = &pTmp[3*width];

  // Vertical average
  for(pix = 0; pix < width; pix++)
    {
      pVertAvg[pix] = pOneRowUpOrDown[pix];
    }
  // Horizontal average
  for(pix = 1; pix < width - 1; pix++)
    {
      pHorizAvg[pix] = BIASED_AVG(pCurRow[pix - 1], pCurRow[pix + 1]);
    }
  // X average
  for(pix = 1; pix < width - 1; pix++)
    {
      pXAvg[pix] = BIASED_AVG(pVertAvg[pix - 1], pVertAvg[pix + 1]);
    }
  // + average
  for(pix = 1; pix < width - 1; pix++)
    {
      pPlusAvg[pix] = (pHorizAvg[pix]*2 + pVertAvg[pix])/3;
    }
}
