/*! @file bmp_pub.h
 * @brief API definition for bitmap module
 * 
 */
#ifndef BMP_PUB_H_
#define BMP_PUB_H_

#include "oscar_error.h"
#ifdef OSC_HOST
    #include "oscar_types_host.h"
    #include "oscar_host.h"
#else
    #include "oscar_types_target.h"
    #include "oscar_target.h"
#endif /* OSC_HOST */

/*! @brief Represents the color depth of a picture */
enum EnOscPictureType {
    OSC_PICTURE_GREYSCALE,
    OSC_PICTURE_BGR_24
};

/*! @brief Macro extracting the color depth from the EnOscPictureType 
 * @param enType Enumeration of type OSC_PICTURE_TYPE.
 * @return The color depth of the specified type.*/
#define OSC_PICTURE_TYPE_COLOR_DEPTH(enType)          \
    (                                               \
            (enType == OSC_PICTURE_BGR_24) ? 24 : 8   \
    )                                         
        
/*! @brief Structure representing an 8-bit picture */
struct OSC_PICTURE {
    void* data;                 /*!< @brief The actual image data */
    unsigned short width;       /*!< @brief Width of the picture */
    unsigned short height;      /*!< @brief Height of the picture */
    enum EnOscPictureType type; /*!< @brief The type of the picture */  
};

/*! @brief Module-specific error codes.
 * 
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscBmpErrors {
    EUNSUPPORTED_FORMAT = OSC_BMP_ERROR_OFFSET,
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
OSC_ERR OscBmpCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscBmpDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Read the contents of a BMP image
 * 
 * Open the specified .bmp file and check the header for validity. Only 
 * 24 bit color depth and 8 bit greyscale with no compression or 
 * color tables are supported.
 * If the caller writes the expected image format to the OSC_PICTURE
 * struct beforehand, these assumptions are checked. Also, the caller can
 * specify the memory location to load the picture to by specifying the
 * data member of the OSC_PICTURE. Otherwise the required memory is 
 * allocated. When specifying the destination address, the expected
 * image format must also be specified to avoid overflow.
 * If the picture in the file has the row order reversed, this is 
 * corrected. The returned row order is top to bottom.
 * 
 * The data in a RGB color BMP is stored with the pixel order BGR, so
 * that is the format in which the data is returned.
 * 
 * @see OscBmpReverseRowOrder
 * 
 * @param pPic Pointer to an uninitialized or fully initialized OSC 
 * picture (Pixel order BGR).
 * @param strFileName The file name of the picture to read.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscBmpRead(struct OSC_PICTURE *pPic, const char *strFileName);

/*********************************************************************//*!
 * @brief Write a picture as a RGB BMP file
 * 
 * Open the specified .bmp file and initialize the header. Only 
 * 24 bit color depth and 8 bit greyscale with no compression or 
 * color tables are supported.
 * Pictures are expected in the top to bottom row order and with the
 * color order BGR. The supplied OSC_PICTURE remains unchanged and no
 * memory is freed.
 * 
 * @param pPic Pointer to a fully initialized OSC picture.
 * @param strFileName The file name of the picture to write.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscBmpWrite(const struct OSC_PICTURE *pPic, 
        const char *strFileName);

#endif /*BMP_PUB_H_*/
