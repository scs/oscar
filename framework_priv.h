/*! @file framework_priv.h
 * @brief Private framework definitions
 * 
 * @author Markus Berner, Samuel Zahnd
 */
#ifndef FRAMEWORK_PRIV_H_
#define FRAMEWORK_PRIV_H_

#include <string.h>

#ifdef LCV_HOST
    #include "framework_types_host.h"
#endif
#ifdef LCV_TARGET
    #include "framework_types_target.h"
#endif

#include "framework_intern.h"


#endif /*FRAMEWORK_PRIV_H_*/
