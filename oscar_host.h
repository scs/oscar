/*! @file oscar_host.h
 * @brief General definitions for the host platform
 * 
 */
#ifndef OSCAR_HOST_H_
#define OSCAR_HOST_H_

/* Defined as stumps because it is needed in code shared by target and
 * host. */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x)       (x)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x)     (x)

#endif /*OSCAR_HOST_H_*/
