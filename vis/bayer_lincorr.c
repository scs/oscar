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

/*! @file bayer_lincorr.c
 * @brief Debayering using linear interpolation with 1st degree laplace
 * coefficients.
 */
#include "vis_pub.h"
#include "vis_priv.h"
#include "sup/sup_pub.h"
#include <stdio.h>
#include <stdlib.h>

/*! @brief Rounding division by 2. */
#define INT_DIVIDE_BY_2_ROUND(x) ((x + 1) / 2)
/*! @brief Rounding division by 3. */
#define INT_DIVIDE_BY_3_ROUND(x) ((x + 1) / 3)
/*! @brief Rounding division by 4. */
#define INT_DIVIDE_BY_4_ROUND(x) ((x + 2) / 4)
/*! @brief Rounding division by 8. */
#define INT_DIVIDE_BY_8_ROUND(x) ((x + 4) / 8)

/*! @brief Saturate an int value to an uint8. */
#define SATURATE_TO_UINT8(x) (unlikely(x < 0) ? 0 : (unlikely(x > 255) ? 255 : x))

/*! @brief Determine whether an integer number is even. */
#define IS_EVEN(x)         (!((x) & 0x1))

/*! @brief The number of bytes used to represent a pixel in the output
 * image. Usually 3 (packed mode). */
#define BYTES_PER_PIX 3
/*! @brief Offset of red color pixels in output array (sequence of R G
 * and B pixels in memory. */
#define RED_OFF 2
/*! @brief Offset of green color pixels in output array (sequence of R G
 * and B pixels in memory. */
#define GREEN_OFF 1
/*! @brief Offset of blue color pixels in output array (sequence of R G
 * and B pixels in memory. */
#define BLUE_OFF 0

/*#define BENCHMARK*/
#ifdef BENCHMARK
	#define BENCH_START(startCyc) \
		startCyc = OscSupCycGet();
#else /* BENCHMARK */
	#define BENCH_START(startCyc) \
		{};
#endif /* BENCHMARK */

#ifdef BENCHMARK
	#define BENCH_STOP(label, startCyc) \
	OscLog(CRITICAL, label ": %dus\n", \
	OscSupCycToMicroSecs(OscSupCycGet() - startCyc));
#else /* BENCHMARK */
	#define BENCH_STOP(label, startCyc) \
		{};
#endif /* BESWNCHMARK */

static inline void InterpRedAndBlue_CurGreenPixRedRow(const uint8* pRaw,
		const uint16 width,
		uint8 *const pOut)
{
	int16 red, blue;

	/* Interpolate the red color horizontally. */
	red = 2*(pRaw[-1] + pRaw[1]);
	red += 2*pRaw[0] - pOut[BYTES_PER_PIX*(-1) + GREEN_OFF]
							- pOut[BYTES_PER_PIX*(1) + GREEN_OFF];
	red = INT_DIVIDE_BY_4_ROUND(red);
	pOut[RED_OFF] = SATURATE_TO_UINT8(red);
	
	/* Interpolate the blue color vertically. */
	blue = 2*(pRaw[-width] + pRaw[width]);
	blue += 2*pRaw[0] - pOut[BYTES_PER_PIX*(-width) + GREEN_OFF]
							- pOut[BYTES_PER_PIX*(width) + GREEN_OFF];
	blue = INT_DIVIDE_BY_4_ROUND(blue);
	pOut[BLUE_OFF] = SATURATE_TO_UINT8(blue);
}
		
static inline void InterpRedAndBlue_CurGreenPixBlueRow(const uint8* pRaw,
		const uint16 width,
		uint8 *const pOut)
{
	int16 red, blue;

	/* Interpolate the blue color horizontally. */
	blue = 2*(pRaw[-1] + pRaw[1]);
	blue += 2*pRaw[0] - pOut[BYTES_PER_PIX*(-1) + GREEN_OFF]
							- pOut[BYTES_PER_PIX*(1) + GREEN_OFF];
	blue = INT_DIVIDE_BY_4_ROUND(blue);
	pOut[BLUE_OFF] = SATURATE_TO_UINT8(blue);
	
	/* Interpolate the red color vertically. */
	red = 2*(pRaw[-width] + pRaw[width]);
	red += 2*pRaw[0] - pOut[BYTES_PER_PIX*(-width) + GREEN_OFF]
							- pOut[BYTES_PER_PIX*(width) + GREEN_OFF];
	red = INT_DIVIDE_BY_4_ROUND(red);
	pOut[RED_OFF] = SATURATE_TO_UINT8(red);
}

static inline uint8 InterpRedOrBlue_CurBlueOrRedPix(const uint8* pRaw,
		const uint16 width,
		const uint8 *const pOut)
{
	int16 northwest, northeast, southwest, southeast;
	int16 lapN, lapP;
	int16 deltaN, deltaP;
	int16 out;

	northwest = pRaw[-width - 1];
	northeast = pRaw[-width + 1];
	southwest = pRaw[width - 1];
	southeast = pRaw[width + 1];
	
	/* Calculate the value of the color not being the raw input. */
	lapN = 2*pOut[GREEN_OFF]
					- pOut[BYTES_PER_PIX*(-width - 1) + GREEN_OFF]
					- pOut[BYTES_PER_PIX*(width + 1) + GREEN_OFF];
	lapP = 2*pOut[GREEN_OFF]
					- pOut[BYTES_PER_PIX*(-width + 1) + GREEN_OFF]
					- pOut[BYTES_PER_PIX*(width - 1) + GREEN_OFF];
	
	/* The diagonal LaPlace of the green color pixels surrounding
	 * the current pixels. */
	deltaN = abs(northwest - southeast) + abs(lapN);
	deltaP = abs(northeast - southwest) + abs(lapP);
	
	/* Interpolate in the direction with the smaller gradient to preserve
	 * edges. */
	if(deltaN < deltaP)
	{
		out = INT_DIVIDE_BY_2_ROUND(northwest + southeast + lapN);
	} else if(deltaN > deltaP) {
		out = INT_DIVIDE_BY_2_ROUND(northeast + southwest + lapP);
	} else {
		out = INT_DIVIDE_BY_4_ROUND(northwest + southeast + lapN
				+ northeast + southwest + lapP);
	}
	return SATURATE_TO_UINT8(out);
}

static void InterpGreen_FirstTwoRows(const uint8* pRaw,
		const uint16 width,
		const bool bTopLeftIsGreen,
		uint8 *const pOut)
{
	uint16 col, startCol;
	const uint8 *pRawPix;
	uint8 *pOutPix;
	int16 out;
	
	/* ------------------ First row ---------------- */
	/* Top left corner. */
	if(!bTopLeftIsGreen)
	{
		pOut[GREEN_OFF] = INT_DIVIDE_BY_2_ROUND(pRaw[1] + pRaw[width]);
		startCol = 1;
		pRawPix = &pRaw[1];
		pOutPix = &pOut[BYTES_PER_PIX + GREEN_OFF];
	} else {
		startCol = 0;
		pRawPix = &pRaw[0];
		pOutPix = &pOut[GREEN_OFF];
	}
	
	/* First line. */
	PREFETCH(&pRawPix[width]);
	for(col = startCol; col < width - 2; col+=2)
	{
		/* Always process two pixels. Copy the first, native green
		 * pixel and interpolate the other. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;
		
		out = pRawPix[-1] + pRawPix[1] + pRawPix[width];
		*pOutPix = INT_DIVIDE_BY_3_ROUND(out);
		pRawPix++;
		pOutPix += BYTES_PER_PIX;
		
		PREFETCH(&pRawPix[width]);
	}
	
	/* Top right corner. */
	if(bTopLeftIsGreen)
	{
		/* Fill in the last two pixels of the first row. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;
		
		/* Last pixel is not green. */
		*pOutPix = INT_DIVIDE_BY_2_ROUND(pRaw[-1] + pRaw[width]);
		pRawPix++;
	} else {
		/* Last pixel is green. */
		*pOutPix = *pRawPix;
		pRawPix++;
	}
	PREFETCH(&pRawPix[width]);
	pOutPix += BYTES_PER_PIX;
	
	/* ------------------ Second row ---------------- */
	/* First pixel. */
	if(bTopLeftIsGreen)
	{
		/* First pixel is not green. */
		out = pRawPix[-width] + pRawPix[1] + pRawPix[width];
		*pOutPix = INT_DIVIDE_BY_3_ROUND(out);
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		startCol = 1;
	} else {
		/* First pixel is green. */
		startCol = 0;
	}
	
	/* Second line. */
	for(col = startCol; col < width - 2; col+=2)
	{
		/* Always process two pixels. Copy the first, native green
		 * pixel and interpolate the other. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		out = pRawPix[-1] + pRawPix[1] + pRawPix[width] + pRawPix[-width];
		*pOutPix = INT_DIVIDE_BY_4_ROUND(out);
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		PREFETCH(&pRawPix[width]);
	}
	/* Last pixel. */
	if(!bTopLeftIsGreen)
	{
		/* Fill in the last two pixels of the second row. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		/* Last pixel is not green. */
		*pOutPix = INT_DIVIDE_BY_3_ROUND(pRawPix[-1] + pRawPix[width] +
					pRawPix[-width]);
	} else {
		/* Last pixel is green. */
		*pOutPix = *pRawPix;
		pRawPix++;
	}
}

static void InterpGreen_LastTwoRows(const uint8* pRaw,
		const uint16 width,
		const bool bFirstIsGreen,
		uint8 *const pOut)
{
	uint16 col, startCol;
	const uint8 *pRawPix;
	uint8 *pOutPix;
	int16 out;
	
	/* -------------- Second last row -----------------*/
	/* Left-most pixel. */
	if(!bFirstIsGreen)
	{
		out = pRaw[-width] + pRaw[1] + pRaw[width];
		pOut[GREEN_OFF] = INT_DIVIDE_BY_3_ROUND(out);
		startCol = 1;
		pRawPix = &pRaw[1];
		pOutPix = &pOut[BYTES_PER_PIX + GREEN_OFF];
	} else {
		startCol = 0;
		pRawPix = &pRaw[0];
		pOutPix = &pOut[GREEN_OFF];
	}
	
	/* Second last line. */
	PREFETCH(&pRawPix[width]);
	for(col = startCol; col < width - 2; col+=2)
	{
		/* Always process two pixels. Copy the first, native green
		 * pixel and interpolate the other. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		out = pRawPix[-1] + pRawPix[1] + pRawPix[-width] + pRawPix[width];
		*pOutPix = INT_DIVIDE_BY_4_ROUND(out);
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		PREFETCH(&pRawPix[width]);
	}
	
	/* Right-most pixel. */
	if(bFirstIsGreen)
	{
		/* Fill in the last two pixels of the first row. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		/* Last pixel is not green. */
		*pOutPix =
			INT_DIVIDE_BY_3_ROUND(pRaw[-1] + pRaw[-width] + pRaw[width]);
	} else {
		/* Last pixel is green. */
		*pOutPix = *pRawPix;
		pRawPix++;
	}
	
	/* --------------- Last row --------------------- */
	/* Left-most pixel. */
	if(bFirstIsGreen)
	{
		/* First pixel is not green. */
		out = pRawPix[-width] + pRawPix[1];
		*pOutPix = INT_DIVIDE_BY_2_ROUND(out);
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		startCol = 1;
	} else {
		/* First pixel is green. */
		startCol = 0;
	}
	
	/* Last line. */
	for(col = startCol; col < width - 2; col+=2)
	{
		/* Always process two pixels. Copy the first, native green
		 * pixel and interpolate the other. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		out = pRawPix[-1] + pRawPix[1] + pRawPix[-width];
		*pOutPix = INT_DIVIDE_BY_3_ROUND(out);
		pRawPix++;
		pOutPix += BYTES_PER_PIX;
	}
	
	/* Right-most pixel. */
	if(!bFirstIsGreen)
	{
		/* Fill in the last two pixels of the second row. */
		*pOutPix = *pRawPix;
		pRawPix++;
		pOutPix += BYTES_PER_PIX;

		/* Last pixel is not green. */
		*pOutPix =
			INT_DIVIDE_BY_2_ROUND(pRawPix[-1] + pRawPix[-width]);
	} else {
		/* Last pixel is green. */
		*pOutPix = *pRawPix;
		pRawPix++;
	}
}

static inline void InterpGreen_FirstTwoCols(const uint8 *pRaw,
		const uint16 width,
		const bool bFirstIsGreen,
		uint8 *const pOut)
{
	int16 out;
	
	/* Interpolate the first pixel and copy the second or vice versa. */
	if(bFirstIsGreen)
	{
		pOut[GREEN_OFF] = *pRaw;
		
		out = pRaw[0] + pRaw[2] + pRaw[1 + width] + pRaw[1 - width];
		pOut[BYTES_PER_PIX + GREEN_OFF] = INT_DIVIDE_BY_4_ROUND(out);
	} else {
		out = pRaw[1] + pRaw[width] + pRaw[-width];
		pOut[GREEN_OFF] = INT_DIVIDE_BY_3_ROUND(out);
		
		pOut[BYTES_PER_PIX + GREEN_OFF] = pRaw[1];
	}
}

static inline void InterpGreen_LastTwoCols(const uint8 *pRaw,
		const uint16 width,
		const bool bFirstIsGreen,
		uint8 *const pOut)
{
	int16 out;
	
	/* Interpolate the first pixel and copy the second or vice versa. */
	if(bFirstIsGreen)
	{
		pOut[GREEN_OFF] = *pRaw;

		out = pRaw[0] + pRaw[1 + width] + pRaw[1 - width];
		pOut[BYTES_PER_PIX + GREEN_OFF] = INT_DIVIDE_BY_3_ROUND(out);
	} else {
		out = pRaw[-1] + pRaw[1] + pRaw[width] + pRaw[-width];
		pOut[GREEN_OFF] = INT_DIVIDE_BY_4_ROUND(out);

		pOut[BYTES_PER_PIX + GREEN_OFF] = pRaw[1];
	}
}

static inline uint8 InterpGreen_CurRedOrBluePix(const uint8* pRaw,
		uint16 width)
{
	int16 west, east, north, south;
	int16 lapH, lapV;
	int16 deltaH, deltaV;
	int16 out;
	
	/* The four neighboring green pixels. */
	west = pRaw[-1];
	east = pRaw[1];
	north = pRaw[-width];
	south = pRaw[width];
	
	/* The horizontal and vertical LaPlace of the current pixel.
	 * Depending on the actual position, these are blue or red pixels. */
	lapH = 2*pRaw[0] - pRaw[-2] - pRaw[2];
	lapV = 2*pRaw[0] - pRaw[-2*width] - pRaw[2*width];
	
	deltaH = abs(west - east) + abs(lapH);
	deltaV = abs(north - south) + abs(lapV);
	
	/* Interpolate in the direction with the smallest gradient to
	 * preserve edges. */
	if(deltaH < deltaV)
	{
		out = INT_DIVIDE_BY_4_ROUND(2 * (west + east) + deltaH);
	} else if(deltaH > deltaV) {
		out = INT_DIVIDE_BY_4_ROUND(2 * (north + south) + deltaV);
	} else {
		out = INT_DIVIDE_BY_8_ROUND(2 * (west + east + north + south) +
				deltaH + deltaV);
	}
	return SATURATE_TO_UINT8(out);
}

inline static void InterpRedAndBlue_FirstOrLastRow(
		const uint8* pRawRow,
		uint16 width,
		bool bOutRowIsRed,
		bool bOutRowFirstPixIsGreen,
		uint8* pCopyRow,
		uint8* pOutRow)
{
	uint8 *pOutPix = pOutRow;
	uint8 *pCopyPix = pCopyRow;
	const uint8* pRawPix = pRawRow;
	uint16 col;
	
	if(bOutRowFirstPixIsGreen)
	{
		if(bOutRowIsRed)
		{
			/* Red row (G R G R G R) */
			for(col = 0; col < width; col += 2)
			{
				pOutPix[RED_OFF] = pCopyPix[RED_OFF];
				pOutPix[BLUE_OFF] = pCopyPix[BLUE_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				pRawPix++;
					
				pOutPix[RED_OFF] = *pRawPix;
				pOutPix[BLUE_OFF] = pCopyPix[BLUE_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				
				pRawPix++;
			}
		} else {
			/* Blue row (G B G B G B). */
			for(col = 0; col < width; col += 2)
			{
				pOutPix[RED_OFF] = pCopyPix[RED_OFF];
				pOutPix[BLUE_OFF] = pCopyPix[BLUE_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				pRawPix++;
								
				pOutPix[BLUE_OFF] = *pRawPix;
				pOutPix[RED_OFF] = pCopyPix[RED_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				
				pRawPix++;
			}
		}
	} else {
		if(bOutRowIsRed)
		{
			/* Red row (R G R G R G) */
			for(col = 0; col < width; col += 2)
			{
				pOutPix[RED_OFF] = *pRawPix;
				pOutPix[BLUE_OFF] = pCopyPix[BLUE_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				
				pOutPix[RED_OFF] = pCopyPix[RED_OFF];
				pOutPix[BLUE_OFF] = pCopyPix[BLUE_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				
				pRawPix += 2;
			}
		} else {
			/* Blue row (B G B G B G). */
			for(col = 0; col < width; col += 2)
			{
				pOutPix[BLUE_OFF] = *pRawPix;
				pOutPix[RED_OFF] = pCopyPix[RED_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				
				pOutPix[RED_OFF] = pCopyPix[RED_OFF];
				pOutPix[BLUE_OFF] = pCopyPix[BLUE_OFF];
				pOutPix += BYTES_PER_PIX;
				pCopyPix += BYTES_PER_PIX;
				
				pRawPix += 2;
			}
		}
	}
}

OSC_ERR OscVisDebayer(const uint8* pRaw,
		const uint16 width,
		const uint16 height,
		enum EnBayerOrder enBayerOrderFirstRow,
		uint8 *const pOut)
{
	bool        bTopLeftIsGreen, bTopRowIsRed, bFirstPixIsGreen, bRowIsRed;
	uint8       *pOutPix, *pOutRow, *pOutPrefetch;
	const uint8 *pRawPix, *pRawRow, *pRawPrefetch;
	uint16      row, col;
#ifdef BENCHMARK
	uint32      startCyc;
#endif
	
	/*---------------------- Input validation. -------------------- */
	if((pRaw == NULL) || (pOut == NULL) || (width == 0) || (height == 0))
	{
		OscLog(ERROR, "%s(0x%x, %d, %d, %d 0x%x): Invalid arguments!",
				__func__, pRaw, width, height, enBayerOrderFirstRow, pOut);
		return -EINVALID_PARAMETER;
	}
	
	if((!IS_EVEN(width)) || (width < 4) || (height < 4))
	{
		OscLog(ERROR, "%s: Invalid parameter! Width: %d Height: %d\n"
				"Width must be even and >=4 and height must be >=4.\n",
				__func__, width, height);
		return -EINVALID_PARAMETER;
	}
	
	bTopLeftIsGreen = (enBayerOrderFirstRow == ROW_GBGB) ||
						(enBayerOrderFirstRow == ROW_GRGR);
	bTopRowIsRed = (enBayerOrderFirstRow == ROW_RGRG) ||
						(enBayerOrderFirstRow == ROW_GRGR);
	
	/* -------------- Interpolate all green pixels. -----------------*/
	
	BENCH_START(startCyc);
	/* The first and last two rows must be treated specially. */
	InterpGreen_FirstTwoRows(pRaw,
		width,
		bTopLeftIsGreen,
		pOut);
	BENCH_STOP("Green_FirstTwoRows", startCyc);
	
	pRawPix = &pRaw[2*width];
	pRawPrefetch = &pRawPix[2*width];
	pOutPix = &pOut[2*BYTES_PER_PIX*width];
	pOutPrefetch = &pOutPix[2*width];
	bFirstPixIsGreen = bTopLeftIsGreen;
	
	BENCH_START(startCyc);
	PREFETCH(pRawPrefetch);
	PREFETCH(pOutPrefetch);
	for(row = 2; row < height - 2; row++)
	{
		/* Interpolate a single row. Copy the pixels that natively
		 * represent green from the raw data and interpolate the
		 * others. We must treat the first and the last two columns
		 * specially. */
		InterpGreen_FirstTwoCols(pRawPix,
				width,
				bFirstPixIsGreen,
				pOutPix);
		pRawPix += 2;
		pOutPix += 2*BYTES_PER_PIX;
		
		if(bFirstPixIsGreen)
		{
			for(col = 2; col < width - 2; col += 2)
			{
				pOutPix[GREEN_OFF] = *pRawPix;
				pRawPix++;
				
				pOutPix[BYTES_PER_PIX + GREEN_OFF] =
					InterpGreen_CurRedOrBluePix(pRawPix, width);
				
				pOutPix += 2*BYTES_PER_PIX;
				pRawPix++;
			}
		} else {
			/* First pixel is red or blue. */
			for(col = 2; col < width - 2; col += 2)
			{
				pOutPix[GREEN_OFF] =
					InterpGreen_CurRedOrBluePix(pRawPix, width);
				pRawPix++;
				
				pOutPix[BYTES_PER_PIX + GREEN_OFF] = *pRawPix;
				pRawPix++;
				
				pOutPix += 2*BYTES_PER_PIX;
			}
		}
		
		InterpGreen_LastTwoCols(pRawPix,
				width,
				bFirstPixIsGreen,
				pOutPix);
		pRawPix += 2;
		pOutPix += 2*BYTES_PER_PIX;
		
		/* The color of the first pixel in a row alternates between green
		 * and either blue or red. */
		bFirstPixIsGreen = !bFirstPixIsGreen;
	}
	BENCH_STOP("Green_Center", startCyc);
	
	BENCH_START(startCyc);
	bFirstPixIsGreen = IS_EVEN(height) ? bTopLeftIsGreen : !bTopLeftIsGreen;
	InterpGreen_LastTwoRows(pRawPix,
			width,
			bFirstPixIsGreen,
			pOutPix);
	BENCH_STOP("Green_LastTwoRows", startCyc);
	
	/* ------------- Interpolate red and blue pixels. ---------------*/
	
	BENCH_START(startCyc);
	bFirstPixIsGreen = !bTopLeftIsGreen;
	bRowIsRed = !bTopRowIsRed;
	pRawPix = &pRaw[width];
	pOutPix = &pOut[width*BYTES_PER_PIX];
	pOutRow = pOutPix;
	pRawRow = pRawPix;
	for(row = 1; row < height - 1; row++)
	{
		/* Interpolate a single row. Copy the pixels that natively
		 * represent green from the raw data and interpolate the
		 * others. We must treat the first and the last columns
		 * specially. */
		
		/* For first column. */
		pRawPix++;
		pOutPix += BYTES_PER_PIX;
		
		if(!bFirstPixIsGreen)
		{
			/* First pixel is not green. */
			if(bRowIsRed)
			{
				for(col = 1; col < width - 1; col += 2)
				{
					InterpRedAndBlue_CurGreenPixRedRow(
								pRawPix,
								width,
								pOutPix);
					
					pOutPix += BYTES_PER_PIX;
					pRawPix++;
					
					pOutPix[RED_OFF] = *pRawPix;
					pOutPix[BLUE_OFF] = InterpRedOrBlue_CurBlueOrRedPix(
							pRawPix,
							width,
							pOutPix);
								
					pOutPix += BYTES_PER_PIX;
					pRawPix++;
				}
				/* First and last column. Just copy from the neighboring
				 * pixels. */
				pOutRow[RED_OFF] = *pRawRow;
				pOutRow[BLUE_OFF] = pOutRow[BYTES_PER_PIX + BLUE_OFF];
				
				pOutRow[(width - 1)*BYTES_PER_PIX + RED_OFF] =
					pOutRow[(width - 2)*BYTES_PER_PIX + RED_OFF];
				pOutRow[(width - 1)*BYTES_PER_PIX + BLUE_OFF] =
					pOutRow[(width - 2)*BYTES_PER_PIX + BLUE_OFF];
			} else {
								
				for(col = 1; col < width - 1; col += 2)
				{
					InterpRedAndBlue_CurGreenPixBlueRow(
							pRawPix,
							width,
							pOutPix);

					pOutPix += BYTES_PER_PIX;
					pRawPix++;

					pOutPix[BLUE_OFF] = *pRawPix;
					pOutPix[RED_OFF] = InterpRedOrBlue_CurBlueOrRedPix(
							pRawPix,
							width,
							pOutPix);

					pOutPix += BYTES_PER_PIX;
					pRawPix++;
				}
				/* First and last column. Just copy from the neighboring
				 * pixels. */
				pOutRow[RED_OFF] = pOutRow[BYTES_PER_PIX + RED_OFF];
				pOutRow[BLUE_OFF] = *pRawRow;

				pOutRow[(width - 1)*BYTES_PER_PIX + RED_OFF] =
					pOutRow[(width - 2)*BYTES_PER_PIX + RED_OFF];
				pOutRow[(width - 1)*BYTES_PER_PIX + BLUE_OFF] =
					pOutRow[(width - 2)*BYTES_PER_PIX + BLUE_OFF];
			}
		}
		else
		{
			/* First pixel is green. */
			if(bRowIsRed)
			{
				for(col = 1; col < width - 1; col += 2)
				{
					pOutPix[RED_OFF] = *pRawPix;
					pOutPix[BLUE_OFF] = InterpRedOrBlue_CurBlueOrRedPix(
							pRawPix,
							width,
							pOutPix);

					pOutPix += BYTES_PER_PIX;
					pRawPix++;
										
					InterpRedAndBlue_CurGreenPixRedRow(
							pRawPix,
							width,
							pOutPix);

					pOutPix += BYTES_PER_PIX;
					pRawPix++;
				}
				/* First and last column. Just copy from the neighboring
				 * pixels. */
				pOutRow[RED_OFF] = pOutRow[BYTES_PER_PIX + RED_OFF];
				pOutRow[BLUE_OFF] = pOutRow[BYTES_PER_PIX + BLUE_OFF];

				pOutRow[(width - 1)*BYTES_PER_PIX + RED_OFF] =
					*pRawPix;
				pOutRow[(width - 1)*BYTES_PER_PIX + BLUE_OFF] =
					pOutRow[(width - 2)*BYTES_PER_PIX + BLUE_OFF];
			}
			else
			{
				for(col = 1; col < width - 1; col += 2)
				{
					pOutPix[BLUE_OFF] = *pRawPix;
					pOutPix[RED_OFF] = InterpRedOrBlue_CurBlueOrRedPix(
							pRawPix,
							width,
							pOutPix);
					
					pRawPix++;
					pOutPix += BYTES_PER_PIX;
										
					InterpRedAndBlue_CurGreenPixBlueRow(
							pRawPix,
							width,
							pOutPix);

					pOutPix += BYTES_PER_PIX;
					pRawPix++;
				}
				/* First and last column. Just copy from the neighboring
				 * pixels. */
				pOutRow[RED_OFF] = pOutRow[BYTES_PER_PIX + RED_OFF];
				pOutRow[BLUE_OFF] = pOutRow[BYTES_PER_PIX + BLUE_OFF];

				pOutRow[(width - 1)*BYTES_PER_PIX + RED_OFF] =
					pOutRow[(width - 2)*BYTES_PER_PIX + RED_OFF];
				pOutRow[(width - 1)*BYTES_PER_PIX + BLUE_OFF] =
					*pRawPix;
			}

		}

		/* For last column */
		pOutPix += BYTES_PER_PIX;
		pRawPix++;
		
		/* The color of the first pixel in a row alternates between green
		 * and either blue or red. */
		bFirstPixIsGreen = !bFirstPixIsGreen;
		bRowIsRed = !bRowIsRed;
		pOutRow += BYTES_PER_PIX*width;
		pRawRow += width;
	}
	BENCH_STOP("RedBlue_Center", startCyc);
	
	/* Fill in the first and the last row as well as the corners.
	 * For this, use the color information of the neighboring pixels.
	 * Start with the last row because it is probably still in cache.*/
	
	BENCH_START(startCyc);
	/* Last Row. */
	bFirstPixIsGreen = IS_EVEN(height) ? !bTopLeftIsGreen : bTopLeftIsGreen;
	InterpRedAndBlue_FirstOrLastRow(
			&pRaw[(height - 1)*width],
			width,
			!bFirstPixIsGreen,
			bFirstPixIsGreen,
			&pOut[(height - 2)*width*BYTES_PER_PIX],
			&pOut[(height - 1)*width*BYTES_PER_PIX]);
	
	/* First row.*/
	InterpRedAndBlue_FirstOrLastRow(
			pRaw,
			width,
			!bTopLeftIsGreen,
			bTopLeftIsGreen,
			&pOut[width*BYTES_PER_PIX],
			pOut);
	BENCH_STOP("RedBlue_Border", startCyc);
	return SUCCESS;
}

OSC_ERR OscVisDebayerGreyscaleHalfSize(uint8 const * const pRaw, uint16 const width, uint16 const height, enum EnBayerOrder const enBayerOrderFirstRow, uint8 * const pOut)
{
	bool bTopLeftIsGreen;
	uint16 ix, iy, outWidth = width / 2, outHeight = height / 2;
	
	/*---------------------- Input validation. -------------------- */
	if((pRaw == NULL) || (pOut == NULL))
	{
		OscLog(ERROR, "%s(0x%x, %d, %d, %d 0x%x): Invalid arguments!",
				__func__, pRaw, width, height, enBayerOrderFirstRow, pOut);
		return -EINVALID_PARAMETER;
	}
	
	if((!IS_EVEN(width)) || (!IS_EVEN(height)) || (width < 2) || (height < 2))
	{
		OscLog(ERROR, "%s: Invalid parameter! Width: %d Height: %d\n"
				"Width must be even and >=2 and height must be >=2.\n",
				__func__, width, height);
		return -EINVALID_PARAMETER;
	}
	
	bTopLeftIsGreen = (enBayerOrderFirstRow == ROW_GBGB) || (enBayerOrderFirstRow == ROW_GRGR);
	
	/* -------------- Interpolate all green pixels. -----------------*/
	
	/* The first and last two rows must be treated specially. */
	
/*	pRawPix = &pRaw[2*width];
	pRawPrefetch = &pRawPix[2*width];
	pOutPix = &pOut[2*BYTES_PER_PIX*width];
	pOutPrefetch = &pOutPix[2*width];
	bFirstPixIsGreen = bTopLeftIsGreen;
	
	BENCH_START(startCyc);
	PREFETCH(pRawPrefetch);
	PREFETCH(pOutPrefetch);
*/

	if (bTopLeftIsGreen)
		for(iy = 0; iy < outHeight; iy += 1)
		{
			uint16 iyRaw = iy * 2;
			for(ix = 0; ix < outWidth; ix += 1)
			{
				uint8 cellRed, cellGreen1, cellGreen2, cellBlue;
				uint16 ixRaw = ix * 2;
				uint16 grey;
				
				cellGreen1 = pRaw[iyRaw * width + ixRaw];
				cellRed = pRaw[iyRaw * width + ixRaw + 1];
				cellBlue = pRaw[(iyRaw + 1) * width + ixRaw];
				cellGreen2 = pRaw[(iyRaw + 1) * width + ixRaw + 1];
				
				grey = (uint16)cellRed * 2 + (uint16)cellGreen1 + (uint16)cellGreen2 + (uint16)cellBlue * 2;
				pOut[iy * outWidth + ix] = grey / 6;
			}
		}
	else
		for(iy = 0; iy < outHeight; iy += 1)
		{
			uint16 iyRaw = iy * 2;
			for(ix = 0; ix < outWidth; ix += 1)
			{
				uint8 cellRed, cellGreen1, cellGreen2, cellBlue;
				uint16 ixRaw = ix * 2;
				uint16 grey;;
				
				cellRed = pRaw[iyRaw * width + ixRaw];
				cellGreen1 = pRaw[iyRaw * width + ixRaw + 1];
				cellGreen2 = pRaw[(iyRaw + 1) * width + ixRaw];
				cellBlue = pRaw[(iyRaw + 1) * width + ixRaw + 1];
				
				grey = (uint16)cellRed * 2 + (uint16)cellGreen1 + (uint16)cellGreen2 + (uint16)cellBlue * 2;
				pOut[iy * outWidth + ix] = grey / 6;
			}
		}
	
	return SUCCESS;
}

OSC_ERR OscVisDebayerSpot(uint8 const * const pRaw, uint16 const width, uint16 const height, enum EnBayerOrder const enBayerOrderFirstRow, uint16 const xPos, uint16 const yPos, uint16 const size, uint8 * color)
{
	bool bTopLeftIsGreen, bTopRowIsRed;
	uint16 ix, iy;
	uint32 sumRed = 0, sumGreen = 0, sumBlue = 0;
	uint32 const sizeSq = size * size;
	
	/*---------------------- Input validation. -------------------- */
	if(pRaw == NULL)
	{
		OscLog(ERROR, "%s(0x%x, %d, %d, %d): Invalid arguments!",
				__func__, pRaw, width, height, enBayerOrderFirstRow);
		return -EINVALID_PARAMETER;
	}
	
	if((!IS_EVEN(width)) || (!IS_EVEN(height)) || (width < 2) || (height < 2))
	{
		OscLog(ERROR, "%s: Invalid parameter! Width: %d Height: %d\n"
				"Width must be even and >=2 and height must be >=2.\n",
				__func__, width, height);
		return -EINVALID_PARAMETER;
	}
	
	
	/* Adjust pattern for a unaligned spot */
	
	bTopLeftIsGreen = (enBayerOrderFirstRow == ROW_GBGB || enBayerOrderFirstRow == ROW_GRGR) == IS_EVEN(xPos) == IS_EVEN(yPos);
	bTopRowIsRed = (enBayerOrderFirstRow == ROW_RGRG || enBayerOrderFirstRow == ROW_GRGR) == IS_EVEN(yPos);
	
	if (bTopLeftIsGreen)
		if (bTopRowIsRed)
			for(iy = yPos; iy < yPos + size; iy += 2)
				for(ix = xPos; ix < xPos + size; ix += 2)
				{
					sumGreen += pRaw[iy * width + ix];
					sumRed += pRaw[iy * width + ix + 1];
					sumBlue += pRaw[(iy + 1) * width + ix];
					sumGreen += pRaw[(iy + 1) * width + ix + 1];
				}
		else
			for(iy = yPos; iy < yPos + size; iy += 2)
				for(ix = xPos; ix < xPos + size; ix += 2)
				{
					sumGreen += pRaw[iy * width + ix];
					sumBlue += pRaw[iy * width + ix + 1];
					sumRed += pRaw[(iy + 1) * width + ix];
					sumGreen += pRaw[(iy + 1) * width + ix + 1];
				}
	else
		if (bTopRowIsRed)
			for(iy = yPos; iy < yPos + size; iy += 2)
				for(ix = xPos; ix < xPos + size; ix += 2)
				{
					sumRed += pRaw[iy * width + ix];
					sumGreen += pRaw[iy * width + ix + 1];
					sumGreen += pRaw[(iy + 1) * width + ix];
					sumBlue += pRaw[(iy + 1) * width + ix + 1];
				}
		else
			for(iy = yPos; iy < yPos + size; iy += 2)
				for(ix = xPos; ix < xPos + size; ix += 2)
				{
					sumBlue += pRaw[iy * width + ix];
					sumGreen += pRaw[iy * width + ix + 1];
					sumGreen += pRaw[(iy + 1) * width + ix];
					sumRed += pRaw[(iy + 1) * width + ix + 1];
				}
	
	color[0] = sumRed * 4 / sizeSq;
	color[1] = sumGreen * 2 / sizeSq;
	color[2] = sumBlue * 4 / sizeSq;
		
	return SUCCESS;
}
