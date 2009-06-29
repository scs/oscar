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

#if 0
#define OscListType(head_t, elem_t, type) \
	typedef struct _osc_internal_list_head_ ## head_t head_t; \
	typedef struct _osc_internal_list_elem_ ## elem_t elem_t; \
	struct _osc_internal_list_head_ ## typedef_head { \
		elem_t * first, * last; \
	}; \
	struct _osc_internal_list_elem_ ## elem_t { \
		head_t * head; \
		elem_t * prev, * next; \
		type data; \
	};

#define OscListInit(head) \
	head = (typeof (head)) { }

#define OscListRemove(head, elem) \
	({
		OSC_ERR err = SUCCESS;
		if (elem->head != elem)
			err = -EPOOL;
		if (err == SUCCESS) {
			elem
		}
		err;
	})

#define ListPrepend(head, elem) \
	({
		OSC_ERR err = SUCCESS;
		if (elem->head != NULL)
			err = ListRemove(elem->head, elem)
		if (err == SUCCESS) {
			elem->head = head;
			elem->next = head->first
			head->first elem;
			if (head->last == NULL)
				head->last = elem
		}
		err;
	})

#define OscListFirst(n, e)

#define OscListIterate(n, i)
#endif

#endif // #ifndef OSCAR_INCLUDE_LIST_H_
