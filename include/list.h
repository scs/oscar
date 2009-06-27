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

#ifndef OSCAR_INCLUDE_LIST_H_
#define OSCAR_INCLUDE_LIST_H_

#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <stdbool.h>
#include <stddef.h>
#include "support.h"

#deinfe ListDeclareType(n, t)
	struct n ## _list_ {
		n
	}

#define ListDeclare(n, t)

#define ListInsert(n, e)

#define ListRemove(n, e)

#define ListFirst(n, e)

#define ListIterate(n, i)


#endif // #ifndef OSCAR_INCLUDE_LIST_H_
