/*! @file oscar.h
 * @brief API definition for Oscar framework
 * 
 * Must be included by the application.
 * 
 */

#ifndef OSCAR_H_
#define OSCAR_H_

/* Include the correct type header file, depending on the target
 */
#ifdef OSC_HOST
	#include "oscar_types_host.h"
    #include "oscar_host.h"
#endif
#ifdef OSC_TARGET
	#include "oscar_types_target.h"
    #include "oscar_target.h"
#endif 

#include "oscar_error.h"
#include "oscar_dependencies.h"


/*********************************************************************//*!
 * @brief Constructor for framework
 * 
 * @param phFw Pointer to the handle location for the framework
 * @return SUCCESS or appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCreate(void ** phFw);

/*********************************************************************//*!
 * @brief Destructor for framework
 * 
 * Fails if not all loaded modules have been destroyed.
 * 
 * @param hFw Pointer to the handle of the framework to be destroyed.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDestroy(void *hFw);

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
#include "dspl_pub.h"
#include "dma_pub.h"
#include "hsm_pub.h"
#include "cfg_pub.h"
#include "clb_pub.h"
#include "vis_pub.h"

#endif /*OSCAR_H_*/
