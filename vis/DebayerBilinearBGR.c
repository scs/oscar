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

/*! @file Jump-in point for debayering to BGR (target and host). */
#include "oscar.h"

OSC_ERR OscVisDebayerBilinearBGR(uint8 *pDst, 
				 uint8 *pSrc, 
				 uint32 width, 
				 uint32 height, 
				 uint8 *pTmp, 
				 enum EnBayerOrder enBayerOrder)
{
  /* Input validation. */
  if(pDst == NULL || pSrc == NULL || (width % 4) || pTmp == NULL)
    {
      return -EINVALID_PARAMETER;
    }
  if(enBayerOrder != ROW_BGBG && enBayerOrder != ROW_GRGR)
    {
      /* Not supported. */
      OscLog(ERROR, "%s: Invalid bayer order supplied (%d)\n", __func__, enBayerOrder);
      return -EINVALID_PARAMETER;
    }
  return DebayerBilinearBGR(pDst,
			    pSrc,
			    width,
			    height,
			    pTmp,
			    enBayerOrder);
}
