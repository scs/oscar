/*	Oscar, a hardware abstraction framework for the LeanXcam and IndXcam.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef OSCAR_INCLUDE_SUPPORT_H_
#define OSCAR_INCLUDE_SUPPORT_H_

#if defined(OSC_HOST)
/* Defined as stumps because it is needed in code shared by target and
 * host. */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x) (x)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x) (x)
#elif defined(OSC_TARGET)
/* Bluntly copied from linux/compiler.h from uclinux */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x) __builtin_expect(!!(x), 1)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#error "Neither OSC_HOST nor OSC_TARGET is defined as a macro."
#endif

#define containerOf(ptr, type, member) \
	({ \
		const typeof (((type *) 0)->member) * __mptr = (ptr); \
		(type *) ((char *)__mptr - offsetof(type, member)); \
	})

/*! @brief The opposite of while (condition) { block }; */
#define until(a) while (!(a))
/*! @brief An endless loop, equivalent to while (true) { block }; */
#define loop while (true)
/*! @brief Gives the length of a field (Does not work on pointers!). */
#define length(a) ((sizeof (a)) / sizeof *(a))


#endif // #ifndef OSCAR_INCLUDE_POOL_H_
