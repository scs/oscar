/*! @file cpld_pub.h
 * @brief API definition for Cpld module.
 * 
 * Cpld provides the basic functionality to access
 * a CPLD device and its register set.
 * 
 * @author Samuel Zahnd
 ************************************************************************/
#ifndef CPLD_PUB_H_
#define CPLD_PUB_H_

#include "framework_error.h"

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvCpldErrors
{
    ENO_CPLD_DEVICE_FOUND = LCV_CPLD_ERROR_OFFSET
};

/*======================= API functions ================================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * Available on Industrial LCV platform only.
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVCpldCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * Available on Industrial LCV platform only.
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVCpldDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Write register
 *  
 * Available on Industrial LCV platform only.
 * 
 * @param regId register id
 * @param val   register value
 * @return 0 
 *//*********************************************************************/
LCV_ERR LCVCpldRset( const uint16 regId, const uint8 val);

/*********************************************************************//*!
 * @brief Write field of register
 *  
 * Available on Industrial LCV platform only.
 * 
 * @param regId register id
 * @param field field id 
 * @param val   field value
 * @return 0 
 *//*********************************************************************/
LCV_ERR LCVCpldFset( const uint16 regId, const uint8 field, const uint8 val);

/*********************************************************************//*!
 * @brief Read register
 *  
 * Available on Industrial LCV platform only.
 * 
 * @param regId   register id
 * @param val     return register value
 * @return 0 
 *//*********************************************************************/
LCV_ERR LCVCpldRget( const uint16 regId, uint8* val);

/*********************************************************************//*!
 * @brief Read field of register
 *  
 * Available on Industrial LCV platform only.
 * 
 * @param regId register id
 * @param field field id 
 * @param val   return field value
 * @return 0 
 *//*********************************************************************/
LCV_ERR LCVCpldFget( const uint16 regId, const uint8 field, uint8* val);


#endif /*CPLD_PUB_H_*/
