/*! @file framework.h
 * @brief API definition for LCV framework
 * 
 * Must be included by the application.
 * 
 * @author Markus Berner, Samuel Zahnd
 * @date 22.1.2008
 * @version 1.0
 */

#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

/* Include the correct type header file, depending on the target
 */
#ifdef LCV_HOST
	#include "framework_types_host.h"
    #include "framework_host.h"
#endif
#ifdef LCV_TARGET
	#include "framework_types_target.h"
    #include "framework_target.h"
#endif 

#include "framework_error.h"
#include "framework_dependencies.h"


/*********************************************************************//*!
 * @brief Constructor for framework
 * 
 * @param phFw Pointer to the handle location for the framework
 * @return SUCCESS or appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVCreate(void ** phFw);

/*********************************************************************//*!
 * @brief Destructor for framework
 * 
 * Fails if not all loaded modules have been destroyed.
 * 
 * @param hFw Pointer to the handle of the framework to be destroyed.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
LCV_ERR LCVDestroy(void *hFw);

/* Include the public header files of the different modules, which
 * contain the declarations of the API functions of the respective
 * module. 
 */
#include "log_pub.h"
#include "cam_pub.h"
#include "cpld_pub.h"
#include "lgx_pub.h"
#include "sim_pub.h"
#include "bmp_pub.h"
#include "swr_pub.h"
#include "srd_pub.h"
#include "ipc_pub.h"
#include "sup_pub.h"
#include "frd_pub.h"
#include "rtl_pub.h"
#include "dma_pub.h"
#include "hsm_pub.h"
#include "cfg_pub.h"
#include "clb_pub.h"
#include "vis_pub.h"

#endif /*FRAMEWORK_H_*/
