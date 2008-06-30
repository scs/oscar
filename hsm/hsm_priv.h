/*! @file hsm_priv.h
 * @brief Private HSM module definition
 * 
 * Original code August 2000, published in www.embedded.com
 * State-Oriented Programming by M. Samek & P.Y. Montgomery
 * 
 * @author Samuel Zahnd
 ************************************************************************/
#ifndef HSM_PRIV_H_
#define HSM_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef LCV_HOST
#include <framework_types_host.h>
#else
#include <framework_types_target.h>
#endif /* LCV_HOST */


/*!@brief object struct */
struct LCV_HSM
{
    bool dummy;                    /*!< @brief dummy variable */
    /* Wie genau fliess hier das HSM pattern ein? 
     * Im framework test braucht es auch ein HSM Test.
     * Wie ist die Unterstützung für follow up events?
     * 
     * */
};





#endif /* HSM_PRIV_H_ */
