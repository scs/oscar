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

/*! @file bmp.c
 * @brief Bitmap module implementation for target and host
 * 
 */

#include "include/bmp.h"
#include "bmp.h"
#include "oscar_intern.h"

/*! @brief The module singelton instance. */
struct OSC_BMP bmp;

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY bmp_deps[] = {
		{"log", OscLogCreate, OscLogDestroy}
};


OSC_ERR OscBmpCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
	OSC_ERR err;
	
	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->bmp.useCnt != 0)
	{
		pFw->bmp.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	/* Load the module dependencies of this module. */
	err = OscLoadDependencies(pFw,
			bmp_deps,
			sizeof(bmp_deps)/sizeof(struct OSC_DEPENDENCY));
	
	if(err != SUCCESS)
	{
		printf("%s: ERROR: Unable to load dependencies! (%d)\n",
				__func__,
				err);
		return err;
	}
	
	memset(&bmp, 0, sizeof(struct OSC_BMP));
	
	/* Increment the use count */
	pFw->bmp.hHandle = (void*)&bmp;
	pFw->bmp.useCnt++;
	
	return SUCCESS;
}

void OscBmpDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->bmp.useCnt--;
	if(pFw->bmp.useCnt > 0)
	{
		return;
	}
	
	OscUnloadDependencies(pFw,
			bmp_deps,
			sizeof(bmp_deps)/sizeof(struct OSC_DEPENDENCY));
	
	memset(&bmp, 0, sizeof(struct OSC_BMP));
}

/*********************************************************************//*!
 * @brief Extract all necessary information from a RGB BMP Header
 * 
 * Read the interesting fields from the header and swap endianness if
 * necessary.
 * 
 * @param pHdr Pointer to the BMP header in memory.
 * @param pWidth Where to store the extracted picture width.
 * @param pHeight Where to store the extracted picture height.
 * @param pDataOffset Where to store the extracted data offset.
 * @param pColorDepth Where to store the extracted color depth.
 *//*********************************************************************/
static inline void OscBmpReadHdrInfo(const uint8 *pHdr,
		int32 * pWidth, int32 * pHeight, int32 *pDataOffset,
		int16 * pColorDepth)
{
#ifdef CPU_LITTLE_ENDIAN
	*pDataOffset = LD_INT32(&pHdr[BMP_HEADER_FIELD_DATA_OFFSET]);
	*pWidth = LD_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH]);
	*pHeight = LD_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT]);
	*pColorDepth = LD_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH]);
	/* BMP uses little endian format */
#else /* CPU_LITTLE_ENDIAN */
	/* For a big endian CPU we need to swap endianness */
	*pDataOffset =
		ENDIAN_SWAP_32(LD_INT32((&pHdr[BMP_HEADER_FIELD_DATA_OFFSET])));
	*pWidth =
		ENDIAN_SWAP_32(LD_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH]));
	*pHeight =
		ENDIAN_SWAP_32(LD_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT]));
	*pColorDepth =
		ENDIAN_SWAP_16(LD_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH]));
#endif /* CPU_LITTLE_ENDIAN */
}

/*********************************************************************//*!
 * @brief Write all necessary information to a RGB BMP Header
 * 
 * Write the interesting fields to the header and swap endianness if
 * necessary.
 * 
 * @param pHdr Pointer to the BMP header in memory.
 * @param width Desired width field content.
 * @param height Desired height field content.
 * @param colorDepth Desired color depth.
 * @param headerSize Size of the bitmap header used.
 *//*********************************************************************/
static inline void OscBmpWriteHdrInfo(uint8 *pHdr,
		const int32 width, const int32 height, const int16 colorDepth,
		const int32 headerSize)
{
	int32           imageSize, dataOffset, fileSize;
	
	imageSize = (((int32)width*(colorDepth/8) + 3)/4)*4*height;
	fileSize = imageSize + headerSize;
	dataOffset = headerSize;
	
	/* BMP uses little endian format */
#ifdef CPU_LITTLE_ENDIAN
	/* Data is already lying correctly in memory */
	ST_INT32(&pHdr[BMP_HEADER_FIELD_FILE_SIZE], fileSize);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_DATA_OFFSET], dataOffset);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH], width);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT], height);
	ST_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH], colorDepth);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_IMAGE_SIZE], imageSize);
#else /* CPU_LITTLE_ENDIAN */
	/* For a big endian CPU we need to swap endianness */
	ST_INT32(&pHdr[BMP_HEADER_FIELD_FILE_SIZE],
			ENDIAN_SWAP_32(fileSize));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_DATA_OFFSET],
			ENDIAN_SWAP_32(dataOffset));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH],
			ENDIAN_SWAP_32(width));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT],
			ENDIAN_SWAP_32(height));
	ST_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH],
			ENDIAN_SWAP_16(colorDepth));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_IMAGE_SIZE],
			ENDIAN_SWAP_32(imageSize));
#endif /* CPU_LITTLE_ENDIAN */
}

/*********************************************************************//*!
 * @brief Reverse the row order of an OSC_PICTURE
 * 
 * @param pPic Pointer to the fully initialized OSC picture.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR OscBmpReverseRowOrder(struct OSC_PICTURE *pPic)
{
	void        *pTempRow;
	uint8       bytesPerPixel;
	uint32      rowLength;
	int         i;
	uint32      curFIndex, curBIndex;
	uint8       *pData = (uint8*)pPic->data;
	
	if(pPic->type == OSC_PICTURE_BGR_24)
	{
		bytesPerPixel = 3;
	} else if(pPic->type == OSC_PICTURE_GREYSCALE) {
		bytesPerPixel = 1;
	} else {
		return -EUNSUPPORTED_FORMAT;
	}
	
	/* Temporary buffer to store one row */
	rowLength = pPic->width * bytesPerPixel;
	pTempRow = (void*)malloc(rowLength);
	if(!pTempRow)
	{
		OscLog(CRITICAL, "%s: Memory allocation failed!\n", __func__);
		return -EOUT_OF_MEMORY;
	}
	
	for(i = 0; i < pPic->height/2; i++)
	{
		/* Swap the rows */
		curFIndex = i * rowLength;
		curBIndex = (pPic->height - i - 1) * rowLength;
		memcpy(pTempRow, &pData[curFIndex], rowLength);
		memcpy(&pData[curFIndex], &pData[curBIndex], rowLength);
		memcpy(&pData[curBIndex], pTempRow, rowLength);
	}
	
	free(pTempRow);
	return 0;
}

uint8 OSC_PICTURE_TYPE_COLOR_DEPTH(enum EnOscPictureType enType)
{
	switch (enType) 
	{
	case OSC_PICTURE_BGR_24:
		return 24;
	case OSC_PICTURE_RGB_24:
		return 24;
	case OSC_PICTURE_YUV_444:
		return 24;
	case OSC_PICTURE_YUV_422:
		return 16;
	case OSC_PICTURE_YUV_420:
		return 24;
	case OSC_PICTURE_YUV_400:
		return 8;
	default:
		return 8;
	}
}

OSC_ERR OscBmpRead(struct OSC_PICTURE *pPic, const char *strFileName)
{
	FILE            *pPicFile;
	/* Temporary buffer to store the header */
	unsigned char   *pHeader[sizeof(aryBmpHeadRGB)];
	int32           dataOffset, width, height;
	int16           colorDepth;
	uint32          imgSize;
	int             bIsReversed = FALSE;
	uint16          row, rowLen;
	uint8           *pData;
	
	if(pPic == NULL || strFileName == NULL || strFileName[0] == '\0')
	{
		OscLog(ERROR, "%s(0x%x, %s): Invalid parameter.\n",
				__func__, pPic, strFileName);
		return -EINVALID_PARAMETER;
	}
	
	pPicFile = fopen(strFileName, "rb");
	if(pPicFile == NULL)
	{
		OscLog(ERROR, "%s: Unable to open picture %s!\n",
				__func__, strFileName);
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	/* Read in the header and extract the interesting fields */
	fread(pHeader, 1, sizeof(aryBmpHeadRGB), pPicFile);
	OscBmpReadHdrInfo((uint8*)pHeader,
			&width,
			&height,
			&dataOffset,
			&colorDepth);
	if(height > 0)
	{
		/* The row order is reversed (mirrored on y axis). This is
		 * the default way that bitmaps are stored (reversed, so to
		 * speak). */
		bIsReversed = TRUE;
	} else {
		height = height * (-1);
	}
	
	/* Check the header for validity */
	if(unlikely(colorDepth != 24 && colorDepth != 8))
	{
		OscLog(ERROR, "%s: Unsupported color depth: %d.\n",
				__func__, colorDepth);
		fclose(pPicFile);
		return -EUNSUPPORTED_FORMAT;
	}
	if(unlikely(dataOffset != sizeof(aryBmpHeadRGB) &&
			dataOffset != sizeof(aryBmpHeadGrey)))
	{
		/* Only supported uncompressed headers without color table */
		OscLog(ERROR, "%s: Unsupported BMP header size: %d.\n",
				__func__, dataOffset);
		fclose(pPicFile);
		return -EUNSUPPORTED_FORMAT;
	}
	
	imgSize = width*height*(colorDepth/8);
	/* If the caller has specified the desired image format check it
	 * against the values from the header. */
	if(unlikely((pPic->width != 0) && ((pPic->width != (uint32)width) ||
					(pPic->height != (uint32)height))))
	{
		OscLog(ERROR, "%s: Wrong image format. %dx%d instead of %dx%d.\n",
				__func__, width, height, pPic->width, pPic->height);
		fclose(pPicFile);
		return -EWRONG_IMAGE_FORMAT;
	}
	if(pPic->data != NULL)
	{
		/* Memory is already allocated by the caller, he needs to know
		 * and supply the expected image format. */
		if(pPic->width == 0)
		{
			fclose(pPicFile);
			OscLog(ERROR,
					"%s: Unable to verify image format assumptions.\n",
					__func__);
			return -EUNABLE_TO_VERIFY_IMAGE_FORMAT;
		}
	} else {
		/* We allocate the memory for the picture */
		pPic->data = (void*)malloc(imgSize);
		if(pPic->data == NULL)
		{
			OscLog(ERROR, "%s: Memory allocation error!\n", __func__);
			return -EOUT_OF_MEMORY;
		}
	}
	
	pPic->width = (uint32)width;
	pPic->height = (uint32)height;
	if(colorDepth == 24)
	{
		pPic->type = OSC_PICTURE_BGR_24;
	} else { /* colorDepth == 8 */
		pPic->type = OSC_PICTURE_GREYSCALE;
	}
		
	/* Seek the pixel data portion of the file and read it in */
	fseek(pPicFile, dataOffset, SEEK_SET);
	
	/* Padded to 4 bytes. */
	rowLen = ((((uint32)pPic->width*(colorDepth/8)) + 3)/4)*4;
	pData = (uint8*)pPic->data;
	for(row = 0; row < pPic->height; row++)
	{
		fread(&pData[row*pPic->width*colorDepth/8], colorDepth/8,
			rowLen, pPicFile);
	}
	
	fclose(pPicFile);
	
	if(bIsReversed)
	{
		/* The row order is reversed */
		return OscBmpReverseRowOrder(pPic);
	}
	return SUCCESS;
}

OSC_ERR OscBmpWrite(const struct OSC_PICTURE *pPic,
		const char *strFileName)
{
	FILE            *pPicFile;
	int16           colorDepth;
	uint8           *aryBmpHead;
	uint32          bmpHeadSize;
	uint32          zero = 0;
	int32          row, rowLen, padLen;
	uint8           *pData;
	
	/* Input validation */
	if(pPic == NULL || pPic->data == NULL ||
			strFileName == NULL || strFileName[0] == '\0' ||
			pPic->width == 0 || pPic->height == 0)
	{
		OscLog(ERROR, "%s(0x%x, %s): Invalid parameter!\n",
				__func__, pPic, strFileName);
		return -EINVALID_PARAMETER;
	}
	if(pPic->type == OSC_PICTURE_BGR_24)
	{
		aryBmpHead = aryBmpHeadRGB;
		bmpHeadSize = sizeof(aryBmpHeadRGB);
		colorDepth = 24;
	}
	else if(pPic->type == OSC_PICTURE_GREYSCALE)
	{
		aryBmpHead = aryBmpHeadGrey;
		bmpHeadSize = sizeof(aryBmpHeadGrey);
		colorDepth = 8;
	}
	else
	{
		OscLog(ERROR, "%s: Unsupported image type (%d).\n",
				__func__, pPic->type);
		return -EUNSUPPORTED_FORMAT;
	}
			
	/* Initialize the header */
	OscBmpWriteHdrInfo(aryBmpHead,
			(int32)pPic->width,
			(int32)pPic->height,
			colorDepth,
			bmpHeadSize);
	
	pPicFile = fopen(strFileName, "wb");
	if(pPicFile == NULL)
	{
		OscLog(ERROR, "%s: Unable to open picture %s!\n",
				__func__, strFileName);
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	/* Write header and data to file */
	fwrite(aryBmpHead, 1, bmpHeadSize, pPicFile);
	
	/* Pad to 4 bytes. Write 'reversed'. Bmps are stored 'reversed'
	 * in the file. */
	rowLen = (int32)pPic->width*colorDepth/8;
	padLen = ((rowLen + 3)/4)*4 - rowLen;
	pData = (uint8*)pPic->data;
	for(row =  pPic->height - 1; row >= 0; row--)
	{
		/* Row data. */
		fwrite(&pData[row*pPic->width*colorDepth/8], 1,
			rowLen, pPicFile);
		if(padLen != 0)
		{
			/* Row padding. */
			fwrite(&zero, 1, padLen, pPicFile);
		}
	}
	fflush(pPicFile);
	fclose(pPicFile);
	
	return SUCCESS;
}
