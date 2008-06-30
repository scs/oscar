/*! @file oscar_target.h
 * @brief General framework definitions for the target
 * 
 */
#ifndef OSCAR_TARGET_H_
#define OSCAR_TARGET_H_

/* Bluntly copied from linux/compiler.h from uclinux */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x)       __builtin_expect(!!(x), 1)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x)     __builtin_expect(!!(x), 0)

#endif /*OSCAR_TARGET_H_*/
