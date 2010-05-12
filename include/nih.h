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

/*! @file
 * @brief NIH - "Not invented here."
 * 
 * Header file to support applications using the NIH design pattern.
 */

#ifndef OSCAR_INCLUDE_NIH_H_
#define OSCAR_INCLUDE_NIH_H_

#include <stdbool.h>
#include <stdint.h>
#include <endian.h>

/* This saves us two characters ... */
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int OSC_ERR;
typedef bool BOOL;

#define TRUE true
#define FALSE false

/* Sadly, the GNU C Library does not define any of {le,be}{16,32,64}{enc,dec}, so we're using our own functions here. The naming schema has been adopted from the FreeBSD headers (http://www.digipedia.pl/man/bswap32.9.html). */
#ifndef bswap16
#  define bswap16(v) ((uint16_t) ((uint16_t) (v) << 8 | (uint16_t) (v) >> 8))
#  define bswap32(v) ((uint32_t) ((uint32_t) bswap16(v) << 16 | (uint32_t) bswap16((v) >> 16)))
#  define bswap64(v) ((uint64_t) ((uint64_t) bswap32(v) << 32 | (uint64_t) bswap32((v) >> 32)))
#endif

#ifndef htobe16
#  if __BYTE_ORDER == __BIG_ENDIAN
#    define htobe16
#    define htole16 bswap16
#    define be16toh
#    define le16toh bswap16
     
#    define htobe32
#    define htole32 bswap32
#    define be32toh
#    define le32toh bswap32
     
#    define htobe64
#    define htole64 bswap64
#    define be64toh
#    define le64toh bswap64
#  elif __BYTE_ORDER == __LITTLE_ENDIAN
#    define htobe16 bswap16
#    define htole16
#    define be16toh bswap16
#    define le16toh
     
#    define htobe32 bswap32
#    define htole32
#    define be32toh bswap32
#    define le32toh
     
#    define htobe64 bswap64
#    define htole64
#    define be64toh bswap64
#    define le64toh
#  else
#    error "__BYTE_ORDER is neither __BIG_ENDIAN nor __LITTLE_ENDIAN."
#  endif
#endif

#ifndef be16dec
#  define be16dec(p) ((uint16_t) ((uint16_t) *((uint8_t *) (p)) << 8 | (uint16_t) *((uint8_t *) (p) + 1)))
#  define be32dec(p) ((uint32_t) ((uint32_t) be16dec(p) << 16 | (uint32_t) be16dec((uint16_t *) (p) + 1)))
#  define be64dec(p) ((uint64_t) ((uint64_t) be32dec(p) << 32 | (uint64_t) be32dec((uint32_t *) (p) + 1)))
   
#  define be16enc(p, v) { *((uint8_t *) (p)) = (uint8_t) ((v) >> 8); *((uint8_t *) (p) + 1) = (uint8_t) (v); }
#  define be32enc(p, v) { be16enc(p, (v) >> 16); be16enc((uint16_t *) (p) + 1, v); }
#  define be64enc(p, v) { be32enc(p, (v) >> 32); be32enc((uint32_t *) (p) + 1, v); }
   
#  define le16dec(p) bswap16(be16dec(p))
#  define le32dec(p) bswap32(be32dec(p))
#  define le64dec(p) bswap64(be64dec(p))
   
#  define le16enc(p, v) be16enc(p, bswap16(v))
#  define le32enc(p, v) be32enc(p, bswap32(v))
#  define le64enc(p, v) be64enc(p, bswap64(v))
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
/*! @brief Endianness of this machine. Intel uses LITTLE_ENDIAN */
#define CPU_LITTLE_ENDIAN
#endif

/* ... and these are written in ergonomical ALL_CAPS; AWESMOE!!!11!1!! */
/*! @brief Macro to swap the endianness of a 16 bit number */
#define ENDIAN_SWAP_16 bswap16
/*! @brief Macro to swap the endianness of a 32 bit number */
#define ENDIAN_SWAP_32 bswap32
/*! @brief Macro to swap the endianness of a 64 bit number */
#define ENDIAN_SWAP_64 bswap64

/*! @brief Write a 16 bit value to a non-aligned address. */
#define ST_INT16 le16enc
/*! @brief Write a 32 bit value to a non-aligned address. */
#define ST_INT32 le32enc

/*! @brief Load a 16 bit value from a non-aligned address. */
#define LD_INT16 le16dec
/*! @brief Load a 32 bit value from a non-aligned address. */
#define LD_INT32 le32dec

#endif /* OSCAR_INCLUDE_NIH_H_ */
