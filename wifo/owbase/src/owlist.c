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

#include "owlist.h"

#include <stdlib.h>
#include <pthread.h>

typedef struct OWListNode OWListNode ;
struct OWListNode
{
	void * element ;
	OWListNode * next ;
} ;

struct OWList
{
	OWListNode * first ;
	OWListNode * last ;
	size_t count ;
	pthread_rwlock_t read_write_lock ;
} ;

struct OWListIterator
{
	OWList * list ;
	OWListNode * previous ;   /* provides the ability to remove the current node and link the previous one */
	OWListNode * current ;
	OWListNode * next ;       /* provides the ability to remove the current node and move next */
	OWListIteratorMode mode ;
} ;

OWList *
owlist_new
(void)
{
	OWList * list = malloc (sizeof (OWList)) ;
	if (list != NULL)
	{
		list->first = NULL ;
		list->last = NULL ;
		list->count = 0 ;
		if (pthread_rwlock_init (& list->read_write_lock, NULL))
		{
			free (list) ;
			return NULL ;
		}
	}
	return list ;
}

int
owlist_free
(
	OWList * list
)
{
	int return_code = 0 ;
	return_code |= owlist_remove_all (list) ;
	return_code |= pthread_rwlock_destroy (& list->read_write_lock) ;
	free (list) ;
	return return_code ;
}

int
owlist_free_all
(
	OWList * list,
	void (* free_element) (void *)
)
{
	int return_code = 0 ;
	OWListIterator * iterator ;
	void * element ;

	iterator = owlist_iterator_new (list, OWLIST_WRITE) ;
	if (iterator != NULL)
	{
		while (owlist_iterator_next (iterator) == 0)
		{
			element = owlist_iterator_get (iterator) ;
			return_code |= owlist_iterator_remove (iterator) ;
			free_element (element) ;
			/* if free_element has a return code, it cannot be tested */
		}
		return_code |= owlist_iterator_free (iterator) ;
	}

	return_code |= owlist_free (list) ;

	return return_code ;
}

static int
owlist_compare
(
	const void * element1,
	const void * element2,
	OWListCompare compare
)
{
	if (compare == NULL)
	{
		if (element1 < element2)
		{
			return -1 ;
		}
		else if (element1 == element2)
		{
			return 0 ;
		}
		else
		{
			return 1 ;
		}
	}

	return compare (element1, element2) ;
}

int
owlist_add
(
	OWList * list,
	const void * element,
	OWListCompare compare
)
{
	OWListIterator * iterator ;
	OWListNode * node ;

	/* write lock */
	iterator = owlist_iterator_new (list, OWLIST_WRITE) ;
	if (iterator == NULL)
	{
		return -1 ;
	}

	/* check if the element is already in the list */
	while (owlist_iterator_next (iterator) == 0)
	{
		if (owlist_compare (iterator->current->element, element, compare) == 0)
		{
			owlist_iterator_free (iterator) ;
			return -1 ;
		}
	}

	/* create a new node */
	node = malloc (sizeof (OWListNode)) ;
	if (node == NULL)
	{
		owlist_iterator_free (iterator) ;
		return -1 ;
	}
	node->element = (void *) element ;
	node->next = NULL ;

	/* add the node at the end of the list */
	if (list->first == NULL)
	{
		list->first = node ;
	}
	else
	{
		list->last->next = node ;
	}
	list->last = node ;
	list->count ++ ;

	/* write unlock */
	if (owlist_iterator_free (iterator))
	{
		return -1 ;
	}

	return 0 ;
}

void *
owlist_remove
(
	OWList * list,
	const void * element,
	OWListCompare compare
)
{
	void * found = NULL ;
	OWListIterator * iterator ;

	iterator = owlist_iterator_new (list, OWLIST_WRITE) ;
	if (iterator == NULL)
	{
		return NULL ;
	}

	while (owlist_iterator_next (iterator) == 0)
	{
		if (owlist_compare (iterator->current->element, element, compare) == 0)
		{
			found = iterator->current->element ;
			owlist_iterator_remove (iterator) ;
			break ;
		}
	}

	if (owlist_iterator_free (iterator))
	{
		return NULL ;
	}

	return found ;
}

int
owlist_remove_all
(
	OWList * list
)
{
	int return_code = 0 ;
	OWListIterator * iterator ;

	iterator = owlist_iterator_new (list, OWLIST_WRITE) ;
	if (iterator == NULL)
	{
		return -1 ;
	}

	while (owlist_iterator_next (iterator) == 0)
	{
		free (iterator->current) ;
	}

	if (owlist_iterator_free (iterator))
	{
		return_code = -1 ;
	}

	list->first = NULL ;
	list->last = NULL ;
	list->count = 0 ;

	return return_code ;
}

int
owlist_contains
(
	OWList * list,
	const void * element,
	OWListCompare compare
)
{
	int return_code = -1 ;
	OWListIterator * iterator ;

	iterator = owlist_iterator_new (list, OWLIST_READ) ;
	if (iterator == NULL)
	{
		return -1 ;
	}

	while (owlist_iterator_next (iterator) == 0)
	{
		if (owlist_compare (iterator->current->element, element, compare) == 0)
		{
			return_code = 0 ;
			break ;
		}
	}

	if (owlist_iterator_free (iterator))
	{
		return -1 ;
	}

	return return_code ;
}

OWListIterator *
owlist_iterator_new
(
	OWList * list,
	OWListIteratorMode mode
)
{
	OWListIterator * iterator = NULL ;
	if (list != NULL)
	{
		iterator = malloc (sizeof (OWListIterator)) ;
		if (iterator != NULL)
		{
			switch (mode)
			{
				case OWLIST_READ :
				{
					if (pthread_rwlock_rdlock (& list->read_write_lock))
					{
						return NULL ;
					}
					break ;
				}
				case OWLIST_WRITE :
				{
					if (pthread_rwlock_wrlock (& list->read_write_lock))
						return NULL ;
					break ;
				}
			}
			iterator->list = list ;
			iterator->mode = mode ;
			owlist_iterator_reset (iterator) ;
		}
	}
	return iterator ;
}

void
owlist_iterator_reset
(
	OWListIterator * iterator
)
{
	iterator->previous = NULL ;
	iterator->current = NULL ;
	iterator->next = iterator->list->first ;
	return ;
}

int
owlist_iterator_free
(
	OWListIterator * iterator
)
{
	int return_code = 0 ;
	if (iterator == NULL)
	{
		return -1 ;
	}
	return_code |= pthread_rwlock_unlock (& iterator->list->read_write_lock) ;
	free (iterator) ;
	return return_code ;
}

int
owlist_iterator_next
(
	OWListIterator * iterator
)
{
	if (! owlist_iterator_has_next (iterator))
	{
		return -1 ;
	}

	iterator->previous = iterator->current ;
	if (iterator->current == NULL)
	{
		iterator->current = iterator->list->first ;
	}
	else
	{
		iterator->current = iterator->next ;
	}
	iterator->next = iterator->current->next ;

	return 0 ;
}

int
owlist_iterator_has_next
(
	const OWListIterator * iterator
)
{
	return iterator->next != NULL ;
}

void *
owlist_iterator_get
(
	OWListIterator * iterator
)
{
	if (iterator->current == NULL)
	{
		return NULL ;
	}
	return iterator->current->element ;
}

int
owlist_iterator_remove
(
	OWListIterator * iterator
)
{
	if (iterator->mode != OWLIST_WRITE)
	{
		return -1 ;
	}
	if (iterator->current == NULL)
	{
		return -1 ;
	}

	/* link the previous node to the next one */
	if (iterator->previous == NULL)
	{
		iterator->list->first = iterator->next ;
	}
	else
	{
		iterator->previous->next = iterator->next ;
	}

	/* refresh the last node if needed */
	if (iterator->next == NULL)
	{
		iterator->list->last = iterator->previous ;
	}

	/* decrement count */
	iterator->list->count -- ;

	/* free node */
	free (iterator->current) ;

	/* mark current node as removed */
	iterator->current = NULL ;

	return 0 ;
}
