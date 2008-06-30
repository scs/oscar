/*! @file vis_priv.h
 * @brief Private vision support module definitions with object structure
 * 
 */
#ifndef VIS_PRIV_H_
#define VIS_PRIV_H_

#ifdef OSC_HOST
    #include <oscar_types_host.h>
#else
    #include <oscar_types_target.h>
#endif /* OSC_HOST */

#include <log/log_pub.h>

/*! @brief Object struct of the bitmap module */
struct OSC_VIS {
    uint32      vis_dummy;  /*!< @brief dummy member */
};

/*======================= Private methods ==============================*/


#endif /*VIS_PRIV_H_*/
