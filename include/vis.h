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



/* Datatypes needed by morphology.c */

/*! @brief Structure representing a part of a structuring element used in binary mathematical morphology. Used mainly for decomposed structuring elements. */
struct OSC_VIS_SUBSTREL {
	uint16 arrayLength;			/*!< @brief Length of the offset vector array */
	uint8 kernelWidth;			/*!< @brief Width in pixels of the structring element kernel */
	uint8 kernelHeight;			/*!< @brief Height in pixels of the structring element kernel */
	int8 *kernelArray;			/*!< @brief Structring element kernel array. The array holds the offset vectors. */
};

/*! @brief Structure representing a structuring element used in binary mathematical morphology. */
struct OSC_VIS_STREL {
	bool isDecomposed;			/*!< @brief Decomposition flag */
	uint8 numberOfSubElements;	/*!< @brief Number of sub-structuring elements if decomposed, else set this to 1. */
	struct OSC_VIS_SUBSTREL subStrEl[4]; /*!< @brief The actual structuring element. It can be composed of subelements or just a single element. */
};


/* Realizations of several structuring elements used in binary mathematical morphology (defined in 'morphology.c') */
extern struct OSC_VIS_STREL DISK2;
extern struct OSC_VIS_STREL DISK8;
extern struct OSC_VIS_STREL DISK1;
extern struct OSC_VIS_STREL DISK0;


/* Datatypes needed by segmentation.c */
#define MAX_NO_OF_RUNS 8192
#define MAX_NO_OF_OBJECTS 1024

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

/*! @brief Structure representing a run used in connected components labeling based on run-length-encoding (RLE). */
struct OSC_VIS_REGIONS_RUN {
	uint16 row;								/*!< @brief The row in which the run was detected */
	uint16 startColumn;						/*!< @brief The start column of the detected run */
	uint16 endColumn;						/*!< @brief The end column of the detected run */
	struct OSC_VIS_REGIONS_RUN *parent;		/*!< @brief Pointer to the parent run */
	struct OSC_VIS_REGIONS_RUN *next;		/*!< @brief Pointer to the next run in the linked list of runs */
	uint16 label;							/*!< @brief Label number of the run */
};

/*! @brief Structure representing an object (=region) in the binary image. Used in connected components labeling based on run-length-encoding (RLE). */
struct OSC_VIS_REGIONS_OBJECT {
	struct OSC_VIS_REGIONS_RUN *root;		/*!< @brief Pointer to the root run representing the object */
	uint16 area;							/*!< @brief Property entry for object area */
	uint16 perimeter;						/*!< @brief Property entry for object perimter (not implemented yet) */
	uint16 centroidX, centroidY;			/*!< @brief Property entry for object centroid pixel coordinates */
	uint16 bboxTop, bboxBottom, bboxLeft, bboxRight; /*!< @brief Property entry for object bounding box coordinates */
};

/*! @brief Structure representing a binary image as connected sets of runs grouped into objects (=regions). It is the result/output
 * of the connected components labeling algorithm based on run-length-encoding (RLE). */
struct OSC_VIS_REGIONS {
	uint16 noOfRuns;											/*!< @brief Number of detected runs */
	uint16 noOfObjects;											/*!< @brief Number of detected objects/regions */
	struct OSC_VIS_REGIONS_RUN runs[MAX_NO_OF_RUNS];			/*!< @brief The array of all detected runs */
	struct OSC_VIS_REGIONS_OBJECT objects[MAX_NO_OF_OBJECTS];	/*!< @brief Array of the detected objects */
};

/* Datatypes needed by filters.c */
/*! @brief Structure representing a filter kernel used in the generic 2D filter. */
struct OSC_VIS_FILTER_KERNEL {	
	uint8 kernelWidth;			/*!< @brief Width in pixels of the filter kernel */
	uint8 kernelHeight;			/*!< @brief Height in pixels of the filter kernel */
	uint16 kernelWeight;		/*!< @brief The sum of all kernel pixel values (Weight). */
	int8 *kernelArray;			/*!< @brief The actual array holding the kernel pixel values */
};

/* Realizations of several filter kernels used by the generic 2D filter (defined in 'filters.c') */
extern struct OSC_VIS_FILTER_KERNEL GAUSS3X3;
extern struct OSC_VIS_FILTER_KERNEL GAUSS5X5;
extern struct OSC_VIS_FILTER_KERNEL MEDIAN3X3;
extern struct OSC_VIS_FILTER_KERNEL MEDIAN5X5;



extern struct OscModule OscModule_vis;

/*====================== API functions =================================*/


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
OSC_ERR OscVisFastDebayerBGR(const struct OSC_PICTURE *pRaw, struct OSC_PICTURE *pOut);

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
 * the usual magnitude formula G = sqrt(GxB2 + GyB2) with the resource consuming
 * square root function, a simple division by a power of two is used: G = (GxB2 + GyB2) * 2^(-exp).
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
 * @brief Binary Erosion.
 * 
 * This function performs an erosion of a binary image (black-white image).
 * It does not treat border pixels, so it leaves a border on the output image untouched,
 * The size of this border depends on the structuring element's width and height.
 * 
 * This function can operate in-place, i.e picIn = picOut. It also supports 
 * multiple runs.
 * 
 * @param picIn Pointer to the input picture struct (type must be OSC_PICTURE_BINARY).
 * @param picOut Pointer to the output picture struct. For in-place operation, set this to the input picture.
 * @param pTempBuffer Pointer to the temporary data buffer.
 * @param pStrEl Pointer to structuring element.
 * @param nRepetitions Number of operation repetitions.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisErode(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 *pTempBuffer, struct OSC_VIS_STREL *pStrEl, uint8 nRepetitions);

/*********************************************************************//*!
 * @brief Binary Dilation.
 * 
 * This function performs an dilation of a binary image (black-white image).
 * It does not treat border pixels, so it leaves a border on the output image untouched,
 * The size of this border depends on the structuring element's width and height.
 * 
 * This function can operate in-place, i.e picIn = picOut. It also supports 
 * multiple runs.
 * 
 * @param picIn Pointer to the input picture struct (type must be OSC_PICTURE_BINARY).
 * @param picOut Pointer to the output picture struct. For in-place operation, set this to the input picture.
 * @param pTempBuffer Pointer to the temporary data buffer.
 * @param pStrEl Pointer to structuring element.
 * @param nRepetitions Number of operation repetitions.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisDilate(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 *pTempBuffer, struct OSC_VIS_STREL *pStrEl, uint8 nRepetitions);

/*********************************************************************//*!
 * @brief Label Binary Image * 
 * 
 * This function labels the binary image by checking for connected-components based on
 * run-length encoding. This function outputs a representation of the binary image based on connected
 * sets of runs. The sets of runs are refered to as 'objects' (=regions of foreground pixels).
 * 
 * @param picIn Pointer to the input picture struct (type must be OSC_PICTURE_BINARY).
 * @param regions Pointer to the regions struct
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisLabelBinary(struct OSC_PICTURE *picIn, struct OSC_VIS_REGIONS *regions);

/*********************************************************************//*!
 * @brief Extract Properties of the Regions (=labeled binary Image) * 
 * 
 * This function fills out the internal properties data fields (area,
 * centroid, bounding box) of the regions.
 * 
 * @param regions Pointer to the regions struct
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisGetRegionProperties(struct OSC_VIS_REGIONS *regions);

/*********************************************************************//*!
 * @brief Draw Centroid Markers * 
 * 
 * This helper function draws the centroids of the Regions as small red crosses. 
 * 
 * @param picIn Pointer to the input color picture struct (type must be OSC_PICTURE_BGR).
 * @param regions Pointer to the regions struct
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisDrawCentroidMarkers(struct OSC_PICTURE *picIn, struct OSC_VIS_REGIONS *regions);

/*********************************************************************//*!
 * @brief Draw Bounding Boxes * 
 * 
 * This helper function draws the bounding boxes of the Regions as rectangles in magenta. 
 * 
 * @param picIn Pointer to the input color picture struct (type must be OSC_PICTURE_BGR).
 * @param regions Pointer to the regions struct
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisDrawBoundingBox(struct OSC_PICTURE *picIn, struct OSC_VIS_REGIONS *regions);

/*********************************************************************//*!
 * @brief RGB to Grayscale Conversion * 
 * 
 * This function converts a RGB color image to a grayscale image using the same R,G,B pixel scalars
 * used in MATLAB.
 * 
 * @param picIn Pointer to the input color picture struct (type must be OSC_PICTURE_BGR).
 * @param picOut Pointer to the output picture struct. (type will be OSC_PICTURE_GREYSCALE).
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisRGB2Gray(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut);

/*********************************************************************//*!
 * @brief Grayscale to Binary Conversion * 
 * 
 * This function converts a grayscale image to a 'binary' image. In fact, the pixel values still are of type uint8,
 * but only values of 0 (background) resp. 1 (foreground) are generated.
 * 
 * @param picIn Pointer to the input color picture struct (type must be OSC_PICTURE_GREYSCALE).
 * @param picOut Pointer to the output picture struct. (type will be OSC_PICTURE_BINARY).
 * @param threshold The threshold value for differentiation of foreground and background pixels.
 * @param bDarkIsForeground Flag to enable detection of dark foreground objects on a bright background.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisGray2BW(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 threshold, bool bDarkIsForeground);

/*********************************************************************//*!
 * @brief Grayscale to Binary Conversion * 
 * 
 * This function converts  a RGB color image directly to a 'binary' image using a MATLAB style RGB-to-grayscale conversion first
 * before the brightness thresholding.
 * 
 * @param picIn Pointer to the input color picture struct (type must be OSC_PICTURE_BGR).
 * @param picOut Pointer to the output picture struct. (type will be OSC_PICTURE_BINARY).
 * @param threshold The threshold value for differentiation of foreground and background pixels.
 * @param bDarkIsForeground Flag to enable detection of dark foreground objects on a bright background.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisRGB2BW(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 threshold, bool bDarkIsForeground);



/*********************************************************************//*!
 * @brief Generic 2D Filter for Grayscale Images * 
 * 
 * A generic 2D filter for grayscale images only. A filter kernel must be provided with this function.
 * 
 * @param picIn Pointer to the input grayscale picture struct (type must be OSC_PICTURE_GREYSCALE).
 * @param picOut Pointer to the output picture struct. 
 * @param pTemp Pointer to the temporary data buffer.
 * @param pKernel Pointer to the filter kernel struct.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscVisFilter2D(struct OSC_PICTURE *picIn, struct OSC_PICTURE *picOut, uint8 *pTemp, struct OSC_VIS_FILTER_KERNEL *pKernel);


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

