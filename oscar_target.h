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
