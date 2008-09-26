/*! @file hsm_priv.h
 * @brief Private HSM module definition
 * 
 * Original code August 2000, published in www.embedded.com
 * State-Oriented Programming by M. Samek & P.Y. Montgomery
 * 
	************************************************************************/
#ifndef HSM_PRIV_H_
#define HSM_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef OSC_HOST
#include <oscar_types_host.h>
#else
#include <oscar_types_target.h>
#endif /* OSC_HOST */


/*!@brief object struct */
struct OSC_HSM
{
	bool dummy;                    /*!< @brief dummy variable */
	/* Wie genau fliess hier das HSM pattern ein?
	 * Im framework test braucht es auch ein HSM Test.
	 * Wie ist die Unterstützung für follow up events?
	 * 
	 * */
};





#endif /* HSM_PRIV_H_ */
