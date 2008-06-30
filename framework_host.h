/*! @file framework_host.h
 * @brief General definitions for the host platform
 * 
 * @author Markus Berner, Samuel Zahnd
 */
#ifndef FRAMEWORK_HOST_H_
#define FRAMEWORK_HOST_H_

/* Defined as stumps because it is needed in code shared by target and
 * host. */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x)       (x)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x)     (x)

#endif /*FRAMEWORK_HOST_H_*/
