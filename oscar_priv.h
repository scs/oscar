/*! @file oscar_priv.h
 * @brief Private framework definitions
 * 
 */
#ifndef FRAMEWORK_PRIV_H_
#define FRAMEWORK_PRIV_H_

#include <string.h>

#ifdef OSC_HOST
    #include "oscar_types_host.h"
#endif
#ifdef OSC_TARGET
    #include "oscar_types_target.h"
#endif

#include "oscar_intern.h"


#endif /*FRAMEWORK_PRIV_H_*/
