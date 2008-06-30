/*! @file vis_priv.h
 * @brief Private vision support module definitions with object structure
 * 
 * @author Markus Berner
 */
#ifndef VIS_PRIV_H_
#define VIS_PRIV_H_

#ifdef LCV_HOST
    #include <framework_types_host.h>
#else
    #include <framework_types_target.h>
#endif /* LCV_HOST */

#include <log/log_pub.h>

/*! @brief Object struct of the bitmap module */
struct LCV_VIS {
    uint32      vis_dummy;  /*!< @brief dummy member */
};

/*======================= Private methods ==============================*/


#endif /*VIS_PRIV_H_*/
