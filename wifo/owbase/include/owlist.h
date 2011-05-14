/*
 * Open Wengo Socket Library
 * Copyright (C) 2007  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _OWLIST_HEADER_
#define _OWLIST_HEADER_

#include "owcommon.h"

#ifdef OS_WINDOWS
	#if defined BUILD_OWBASE_DLL
		#define OWLIST_FUNC_DEF __declspec(dllexport)
	#elif ! defined BUILD_OWBASE_STATIC
		#define OWLIST_FUNC_DEF __declspec(dllimport)
	#endif
#endif
#ifndef OWLIST_FUNC_DEF
	#define OWLIST_FUNC_DEF
#endif /* OWLIST_FUNC_DEF */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct OWList OWList ;

typedef int (* OWListCompare) (const void *, const void *) ;

typedef struct OWListIterator OWListIterator ;

typedef enum OWListIteratorMode
{
	OWLIST_READ,
	OWLIST_WRITE
} OWListIteratorMode ;

/**
 * Create a new list.
 *
 * @return the new list
 * @return NULL if there is not enough memory
 */
OWLIST_FUNC_DEF OWList *
owlist_new
(void) ;

/**
 * Destroy a list without freeing its elements.
 */
OWLIST_FUNC_DEF int
owlist_free
(
	OWList * list
) ;

/**
 * Destroy a list and free its elements.
 */
OWLIST_FUNC_DEF int
owlist_free_all
(
	OWList * list,
	void (* free_element) (void *)
) ;

/**
 * Add an element at the end of the list.
 * An element cannot be added more than once.
 * (based on a comparator function, defaults to pointer comparison if no comparator provided)
 *
 * @warning It may cause a deadlock if an iterator is open on the list.
 *
 * @return 0 if it is added
 * @return a negative value otherwise
 */
OWLIST_FUNC_DEF int
owlist_add
(
	OWList * list,
	const void * element,
	OWListCompare compare
) ;

/**
 * Remove an element from the list.
 * (based on a comparator function, defaults to pointer comparison if no comparator provided)
 *
 * @return the pointer of the removed element
 * @return NULL otherwise
 */
OWLIST_FUNC_DEF void *
owlist_remove
(
	OWList * list,
	const void * element,
	OWListCompare compare
) ;

/**
 * Remove all elements from the list but the elements are not freed.
 *
 * @return 0 if the elements are removed
 * @return a negative value otherwise
 */
OWLIST_FUNC_DEF int
owlist_remove_all
(
	OWList * list
) ;

/**
 * Check if the list contains an element
 * (based on a comparator function, defaults to pointer comparison if no comparator provided)
 *
 * @return 0 if the element is not in the list
 * @return another value otherwise
 */
OWLIST_FUNC_DEF int
owlist_contains
(
	OWList * list,
	const void * element,
	OWListCompare compare
) ;

/**
 * Allocate an iterator that points no element (before the first one).
 * In read mode, no thread can modify the list until the iterator is closed.
 * In write mode, no thread can read or modify the list until the iterator is closed.
 *
 * @return 0 if the iterator is initialized
 * @return another value otherwise
 */
OWLIST_FUNC_DEF OWListIterator *
owlist_iterator_new
(
	OWList * list,
	OWListIteratorMode mode
) ;

/**
 * Move an iterator before the first element.
 */
OWLIST_FUNC_DEF void
owlist_iterator_reset
(
	OWListIterator * iterator
) ;

/**
 * Destroy an iterator.
 */
OWLIST_FUNC_DEF int
owlist_iterator_free
(
	OWListIterator * iterator
) ;

/**
 * Move to the next element.
 *
 * @return 0 if the next element is pointed by iterator
 * @return a negative value otherwise
 */
OWLIST_FUNC_DEF int
owlist_iterator_next
(
	OWListIterator * iterator
) ;

/**
 * Check if there is a next element.
 *
 * @return 0 if there is no next element
 * @return another value otherwise
 */
OWLIST_FUNC_DEF int
owlist_iterator_has_next
(
	const OWListIterator * iterator
) ;

/**
 * Get an element pointed by an iterator from the list.
 *
 * @return the element
 * @return NULL if there is no element at the current position
 */
OWLIST_FUNC_DEF void *
owlist_iterator_get
(
	OWListIterator * iterator
) ;

/**
 * Remove an element pointed by an iterator from the list.
 *
 * @return 0 if it is removed
 * @return a negative value otherwise
 */
OWLIST_FUNC_DEF int
owlist_iterator_remove
(
	OWListIterator * iterator
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OW_LIST_HEADER_ */
