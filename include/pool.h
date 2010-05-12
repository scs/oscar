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

#ifndef OSCAR_INCLUDE_POOL_H_
#define OSCAR_INCLUDE_POOL_H_

#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <stdbool.h>
#include <stddef.h>
#include "support.h"

#define PoolDeclare(n, t, c) \
	struct { \
		union block * free; \
		union block { \
			union block * next; \
			t value; \
		} data[c]; \
	} (n);

#define PoolInit(n) \
	({ \
		typeof ((n).free) p = (n).data; \
		while (p - (n).data < ARR_LENGTH((n).data) - 1) { \
			p->next = p + 1; \
			p += 1; \
		} \
		p->next = NULL; \
		(n).free = (n).data; \
		SUCCESS; \
	})

#define PoolGet(n, p) \
	({ \
		OSC_ERR err = EPOOL; \
		typeof ((n).free) res = (n).free; \
		if (res != NULL) { \
			(n).free = res->next; \
			*(p) = &res->value; \
			err = SUCCESS; \
		} \
		err; \
	})

#define PoolPut(n, p) \
	({ \
		typeof ((n).free) b = containerOf((p), typeof (*(n).free), value); \
		if (b != NULL) { \
			b->next = (n).free; \
			(n).free = b; \
		} \
		SUCCESS; \
	})

#endif // #ifndef OSCAR_INCLUDE_POOL_H_
