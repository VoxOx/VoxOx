/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef __MAPPINGLIST_H__
#define __MAPPINGLIST_H__

#include <phapi-util/phapiutildll.h>

#ifndef TRUE
	#define TRUE	1
#endif /* TRUE */
#ifndef FALSE
	#define FALSE	0
#endif /* FALSE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef struct mappinglist_link mappinglist_link_t;
	/**
	* Structure of a link of the mapping list
	*/
	struct mappinglist_link {
		int int_key;
		void * key;
		void * element;
		mappinglist_link_t * next;
	};

	typedef struct mappinglist mappinglist_t;
	/**
	* Structure of the mapping list
	*/
	struct mappinglist{
		unsigned int count;
		mappinglist_link_t * first;
	};


	/**
	* Creates an empty mapping list
	*
	* @return	a pointer to the mapping list
	*/
	PHAPIUTIL_API mappinglist_t * create_mappinglist();

	/**
	* Frees a mapping list. All contained elements and keys are not freed.
	*
	* Example:
	* <pre>
	*	mappinglist_t * list;
	*	free_mappinglist(&list);
	* </pre>
	*
	* @param	[in-out]	list : a pointer to the list pointer to free and turn into NULL
	*/
	PHAPIUTIL_API void free_mappinglist(mappinglist_t ** list);

	/**
	* Puts an element in the mapping list associated to the given key.
	* If the key is already used, the adding will be unsuccesful
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in-out]	list : a pointer to the list receiving the new element
	* @param	[in]	key : a pointer to the key
	* @param	[in]	element : a pointer to the element
	* @param	[in]	equals : a pointer to the comparator function used to check the key equality
	* @return	TRUE if the add was successful; FALSE else
	*/
	PHAPIUTIL_API unsigned int mappinglist_put(mappinglist_t * list, void * key, void * element, unsigned int (*equals)(void *, void *));

	/**
	* Puts an element in the mapping list associated to the given integer key
	* If the key is already used, the adding will be unsuccesful
	*
	* @param	[in-out]	list : a pointer to the list receiving the new element
	* @param	[in]	key : an integer key
	* @param	[in]	element : a pointer to the element
	* @return	TRUE if the add was successful; FALSE else
	*/
	PHAPIUTIL_API unsigned int mappinglist_put_with_int_key(mappinglist_t * list, int key, void * element);

	/**
	* Removes the element associated to the key from the mapping list.
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in-out]	list : a pointer to the list from which the element associated to the key is removed
	* @param	[in]	key : a pointer to the key
	* @param	[in]	equals : a pointer to the comparator function used to check the key equality
	* @return	the removed element if the suppression was successful; NULL else
	*/
	PHAPIUTIL_API void * mappinglist_remove(mappinglist_t * list, void * key, unsigned int (*equals)(void *, void *));

	/**
	* Removes the element associated to the integer key from the mapping list
	*
	* @param	[in-out]	list : a pointer to the list from which the element associated to the key is removed
	* @param	[in]	key : an integer key
	* @return	the removed element if the suppression was successful; NULL else
	*/
	PHAPIUTIL_API void * mappinglist_remove_with_int_key(mappinglist_t * list, int key);

	/**
	* Retrieves an element associated with the given key from the mapping list
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in]	list : a pointer to the list from which the element associated to the key is retrieved
	* @param	[in]	key : a pointer to a key
	* @param	[in]	equals : a pointer to the comparator function used to check the key equality
	* @return	the removed element if the suppression was successful; NULL else
	*/
	PHAPIUTIL_API void * mappinglist_get(mappinglist_t * list, void * key, unsigned int (*equals)(void *, void *));

	/**
	* Retrieves an element associated with the given key from the mapping list
	*
	* @param	[in]	list : a pointer to the list from which the element associated to the integer key is retrieved
	* @param	[in]	key : an integer key
	* @return	the removed element if the suppression was successful; NULL else
	*/
	PHAPIUTIL_API void * mappinglist_get_with_int_key(mappinglist_t * list, int key);

	/**
	* Checks if a key is already contained in the mapping list
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in]	list : a pointer to the list
	* @param	[in]	key : a pointer to a key
	* @param	[in]	equals : a pointer to the comparator function used to check the key equality
	* @return	TRUE if the key is already used in the list; FALSE else
	*/
	PHAPIUTIL_API unsigned int mappinglist_contains_key(mappinglist_t * list, void * key, unsigned int (*equals)(void *, void *));

	/**
	* Checks if an integer key is already contained in the mapping list
	*
	* @param	[in]	list : a pointer to the list
	* @param	[in]	key : an integer key
	* @return	TRUE if the key is already used in the list; FALSE else
	*/
	PHAPIUTIL_API unsigned int mappinglist_contains_int_key(mappinglist_t * list, int key);

	/**
	* Checks if an element is already contained in the mapping list
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in]	list : a pointer to the list
	* @param	[in]	element : a pointer to an element
	* @param	[in]	equals : a pointer to the comparator function used to check the element equality
	* @return	TRUE if the element is already used in the list; FALSE else
	*/
	PHAPIUTIL_API unsigned int mappinglist_contains_element(mappinglist_t * list, void * element, unsigned int (*equals)(void *, void *));

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __MAPPINGLIST_H__ */


