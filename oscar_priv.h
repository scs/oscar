/*! @file oscar_priv.h
 * @brief Private framework definitions
 * 
 */
#ifndef OSCAR_PRIV_H_
#define OSCAR_PRIV_H_

#define OSC_VERSION_MAJOR		1		/*<! @brief Major version number. */
#define OSC_VERSION_MINOR		0		/*<! @brief Minor version number. */
#define OSC_VERSION_REVISION	0		/*<! @brief Bug-fix revision number. */

#include <string.h>

#ifdef OSC_HOST
    #include "oscar_types_host.h"
#endif
#ifdef OSC_TARGET
    #include "oscar_types_target.h"
#endif

#include "oscar_intern.h"


#endif /*OSCAR_PRIV_H_*/
