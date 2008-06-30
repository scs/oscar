/*! @file bmp_pub.h
 * @brief API definition for bitmap module
 * 
 * @author Markus Berner
 */
#ifndef BMP_PUB_H_
#define BMP_PUB_H_

#include "framework_error.h"
#ifdef LCV_HOST
    #include "framework_types_host.h"
    #include "framework_host.h"
#else
    #include "framework_types_target.h"
    #include "framework_target.h"
#endif /* LCV_HOST */

/*! @brief Represents the color depth of a picture */
enum EnLcvPictureType {
    LCV_PICTURE_GREYSCALE,
    LCV_PICTURE_RGB_24
};

/*! @brief Macro extracting the color depth from the EnLcvPictureType 
 * @param enType Enumeration of type LCV_PICTURE_TYPE.
 * @return The color depth of the specified type.*/
#define LCV_PICTURE_TYPE_COLOR_DEPTH(enType)          \
    (                                               \
            (enType == LCV_PICTURE_RGB_24) ? 24 : 8   \
    )                                         
        
/*! @brief Structure representing an 8-bit picture */
struct LCV_PICTURE {
    void* data;                 /*!< @brief The actual image data */
    unsigned short width;       /*!< @brief Width of the picture */
    unsigned short height;      /*!< @brief Height of the picture */
    enum EnLcvPictureType type; /*!< @brief The type of the picture */  
};

/*! @brief Module-specific error codes.
 * 
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvBmpErrors {
    EUNSUPPORTED_FORMAT = LCV_BMP_ERROR_OFFSET,
    EWRONG_IMAGE_FORMAT,
    EUNABLE_TO_VERIFY_IMAGE_FORMAT
};

/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVBmpCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVBmpDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Read the contents of a BMP image
 * 
 * Open the specified .bmp file and check the header for validity. Only 
 * 24 bit color depth and 8 bit greyscale with no compression or 
 * color tables are supported.
 * If the caller writes the expected image format to the LCV_PICTURE
 * struct beforehand, these assumptions are checked. Also, the caller can
 * specify the memory location to load the picture to by specifying the
 * data member of the LCV_PICTURE. Otherwise the required memory is 
 * allocated. When specifying the destination address, the expected
 * image format must also be specified to avoid overflow.
 * If the picture in the file has the row order reversed, this is 
 * corrected. The returned row order is top to bottom.
 * @see LCVBmpReverseRowOrder
 * 
 * @param pPic Pointer to an uninitialized or fully initialized LCV 
 * picture.
 * @param strFileName The file name of the picture to read.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVBmpRead(struct LCV_PICTURE *pPic, const char *strFileName);

/*********************************************************************//*!
 * @brief Write a picture as a RGB BMP file
 * 
 * Open the specified .bmp file and initialize the header. Only 
 * 24 bit color depth and 8 bit greyscale with no compression or 
 * color tables are supported.
 * Pictures are expected in the top to bottom row order. The supplied
 * LCV_PICTURE remains unchanged and no memory is freed.
 * 
 * @param pPic Pointer to a fully initialized LCV picture.
 * @param strFileName The file name of the picture to write.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVBmpWrite(const struct LCV_PICTURE *pPic, 
        const char *strFileName);

#endif /*BMP_PUB_H_*/
