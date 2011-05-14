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

#include <phapi-util/mappinglist.h>
#include <phapi-util/comparator.h>
#include <phapi-util/util.h>

#include <stdlib.h>

// ----- PRIVATE FUNCTIONS DECLARATION ----
static unsigned int mappinglist_equals(void * element_a, void * element_b, unsigned int (*equals)(void *, void *));
static mappinglist_link_t * create_mappinglist_link(void * key, void * element);
static mappinglist_link_t * create_mappinglist_link_with_int_key(int key, void * element);
static void free_mappinglist_link(mappinglist_link_t ** link);
// -----


/**
* Creates an empty mapping list
*
* @return	a pointer to the mapping list
*/
mappinglist_t * create_mappinglist(){
	mappinglist_t * list;

	if((list = (mappinglist_t *)malloc(sizeof(mappinglist_t))) == NULL){
		m_log_error("Not enough memory", "create_mappinglist");
		return NULL;
	}
	list->count = 0;
	list->first = NULL;

	return list;
}

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
void free_mappinglist(mappinglist_t ** list){
	mappinglist_link_t * link;
	mappinglist_link_t * temp_link;

	if(list != NULL && *list != NULL){
		if((*list)->count > 0){
			link = (*list)->first;
			(*list)->count = 0;
			(*list)->first = NULL;
			while(link->next != NULL){
				temp_link = link;
				link = link->next;
				free_mappinglist_link(&temp_link);
			}
			free_mappinglist_link(&link);
		}
		free(*list);
		*list = NULL;
	}
}

/**
* Creates a new mapping list link to chain an element associated to a key, in the list.
*
* @param	[in]	key : a pointer to the key
* @param	[in]	element : a pointer to the element
* @return	[out]	a mapping list link associating the element to the key
*/
static mappinglist_link_t * create_mappinglist_link(void * key, void * element){
	mappinglist_link_t * link;

	if((link = (mappinglist_link_t *)malloc(sizeof(mappinglist_link_t))) == NULL){
		m_log_error("Not enough memory", "create_mappinglist_link");
		return NULL;
	}
	link->int_key = 0;
	link->key = key;
	link->element = element;
	link->next = NULL;

	return link;
}

/**
* Creates a new mapping list link to chain an element associated to an integer key, in the list.
*
* @param	[in]	key : an integer to which associate the element
* @param	[in]	element : a pointer to the element
* @return	[out]	a mapping list link associating the element to the integer key
*/
static mappinglist_link_t * create_mappinglist_link_with_int_key(int key, void * element){
	mappinglist_link_t * link;

	if((link = (mappinglist_link_t *)malloc(sizeof(mappinglist_link_t))) == NULL){
		m_log_error("Not enough memory", "create_mappinglist_link_with_int_key");
		return NULL;
	}
	link->int_key = key;
	link->key = NULL;
	link->element = element;
	link->next = NULL;

	return link;
}


/**
* Frees the memory associated to a mapping list link.
* The contained element and key are not freed.
*
* Example:
* <pre>
*	mappinglist_link_t * link;
*	free_mappinglist_link(&link);
* </pre>
*
* @param	[in-out]	link : a pointer to the the link pointer to free and turn into NULL
*/
static void free_mappinglist_link(mappinglist_link_t ** link){
	(*link)->int_key = 0;
	(*link)->key = NULL;
	(*link)->element = NULL;
	(*link)->next = NULL;
	free(*link);
	*link = NULL;
}

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
unsigned int mappinglist_put(mappinglist_t * list, void * key, void * element, unsigned int (*equals)(void *, void *)){
	mappinglist_link_t * new_link;
	mappinglist_link_t * link;

	if(list != NULL && key != NULL && element != NULL){
		if(!mappinglist_contains_key(list, key, equals)){
			new_link = create_mappinglist_link(key, element);

			if(list->count == 0){
				list->first = new_link;
				list->count++;
				return TRUE;
			}else{ // count >= 1
				link = list->first;
				while(link->next != NULL){
					link = link->next;
				} // link->next == NULL
				link->next = new_link;
				list->count++;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
* Puts an element in the mapping list associated to the given integer key
* If the key is already used, the adding will be unsuccesful
*
* @param	[in-out]	list : a pointer to the list receiving the new element
* @param	[in]	key : an integer key
* @param	[in]	element : a pointer to the element
* @return	TRUE if the add was successful; FALSE else
*/
unsigned int mappinglist_put_with_int_key(mappinglist_t * list, int key, void * element){
	mappinglist_link_t * new_link;
	mappinglist_link_t * link;

	if(list != NULL && element != NULL){
		if(!mappinglist_contains_int_key(list, key)){
			new_link = create_mappinglist_link_with_int_key(key, element);

			if(list->count == 0){
				list->first = new_link;
				list->count++;
				return TRUE;
			}else{ // count >= 1
				link = list->first;
				while(link->next != NULL){
					link = link->next;
				} // link->next == NULL
				link->next = new_link;
				list->count++;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
* Removes the element associated to the key from the mapping list.
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in-out]	list : a pointer to the list from which the element associated to the key is removed
* @param	[in]	key : a pointer to the key
* @param	[in]	equals : a pointer to the comparator function used to check the key equality
* @return	the removed element if the suppression was successful; NULL else
*/
void * mappinglist_remove(mappinglist_t * list, void * key, unsigned int (*equals)(void *, void *)){
	mappinglist_link_t * link;
	mappinglist_link_t * previous_link;
	void * element;

	if(list != NULL && key != NULL){
		if(list->count > 0){
			link = list->first;
			if(mappinglist_equals(link->key, key, equals)){
				// first receives next, or NULL if no more
				element = link->element;
				list->first = link->next;
				free_mappinglist_link(&link);
				list->count--;
				return element;
			}
			while(link->next != NULL){
				previous_link = link;
				link = link->next;
				if(mappinglist_equals(link->key, key, equals)){
					// previous receives next, or NULL if no more
					element = link->element;
					previous_link->next = link->next;
					free_mappinglist_link(&link);
					list->count--;
					return element;
				}
			}
		}
	}
	return NULL;
}

/**
* Removes the element associated to the integer key from the mapping list
*
* @param	[in-out]	list : a pointer to the list from which the element associated to the key is removed
* @param	[in]	key : an integer key
* @return	the removed element if the suppression was successful; NULL else
*/
void * mappinglist_remove_with_int_key(mappinglist_t * list, int key){
	mappinglist_link_t * link;
	mappinglist_link_t * previous_link;
	void * element;

	if(list != NULL){
		if(list->count > 0){
			link = list->first;
			if(link->int_key == key){
				// first receives next, or NULL if no more
				element = link->element;
				list->first = link->next;
				free_mappinglist_link(&link);
				list->count--;
				return element;
			}
			while(link->next != NULL){
				previous_link = link;
				link = link->next;
				if(link->int_key == key){
					// previous receives next, or NULL if no more
					element = link->element;
					previous_link->next = link->next;
					free_mappinglist_link(&link);
					list->count--;
					return element;
				}
			}
		}
	}
	return NULL;
}

/**
* Retrieves an element associated with the given key from the mapping list
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in]	list : a pointer to the list from which the element associated to the key is retrieved
* @param	[in]	key : a pointer to a key
* @param	[in]	equals : a pointer to the comparator function used to check the key equality
* @return	the removed element if the suppression was successful; NULL else
*/
void * mappinglist_get(mappinglist_t * list, void * key, unsigned int (*equals)(void *, void *)){
	mappinglist_link_t * link;
	mappinglist_link_t * previous_link;

	if(list != NULL && key != NULL){
		if(list->count > 0){
			link = list->first;
			if(mappinglist_equals(link->key, key, equals)){
				return link->element;
			}
			while(link->next != NULL){
				previous_link = link;
				link = link->next;
				if(mappinglist_equals(link->key, key, equals)){
					return link->element;
				}
			}
		}
	}
	return NULL;
}

/**
* Retrieves an element associated with the given key from the mapping list
*
* @param	[in]	list : a pointer to the list from which the element associated to the integer key is retrieved
* @param	[in]	key : an integer key
* @return	the removed element if the suppression was successful; NULL else
*/
void * mappinglist_get_with_int_key(mappinglist_t * list, int key){
	mappinglist_link_t * link;
	mappinglist_link_t * previous_link;

	if(list != NULL){
		if(list->count > 0){
			link = list->first;
			if(link->int_key == key){
				return link->element;
			}
			while(link->next != NULL){
				previous_link = link;
				link = link->next;
				if(link->int_key == key){
					return link->element;
				}
			}
		}
	}
	return NULL;
}
/**
* Checks if a key is already contained in the mapping list
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in]	list : a pointer to the list
* @param	[in]	key : a pointer to a key
* @param	[in]	equals : a pointer to the comparator function used to check the key equality
* @return	TRUE if the key is already used in the list; FALSE else
*/
unsigned int mappinglist_contains_key(mappinglist_t * list, void * key, unsigned int (*equals)(void *, void *)){
	mappinglist_link_t * link;

	if(list != NULL && key != NULL){
		if(list->count > 0){
			link = list->first;
			if(mappinglist_equals(link->key, key, equals)){
				return TRUE;
			}
			while(link->next != NULL){
				link = link->next;
				if(mappinglist_equals(link->key, key, equals)){
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/**
* Checks if an integer key is already contained in the mapping list
*
* @param	[in]	list : a pointer to the list
* @param	[in]	key : an integer key
* @return	TRUE if the key is already used in the list; FALSE else
*/
unsigned int mappinglist_contains_int_key(mappinglist_t * list, int key){
	mappinglist_link_t * link;

	if(list != NULL){
		if(list->count > 0){
			link = list->first;
			if(link->int_key == key){
				return TRUE;
			}
			while(link->next != NULL){
				link = link->next;
				if(link->int_key == key){
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/**
* Checks if an element is already contained in the mapping list
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in]	list : a pointer to the list
* @param	[in]	element : a pointer to an element
* @param	[in]	equals : a pointer to the comparator function used to check the element equality
* @return	TRUE if the element is already used in the list; FALSE else
*/
unsigned int mappinglist_contains_element(mappinglist_t * list, void * element, unsigned int (*equals)(void *, void *)){
	mappinglist_link_t * link;

	if(list != NULL && element != NULL){
		if(list->count > 0){
			link = list->first;
			if(mappinglist_equals(link->element, element, equals)){
				return TRUE;
			}
			while(link->next != NULL){
				link = link->next;
				if(mappinglist_equals(link->element, element, equals)){
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
/**
* Selector function to choose the right comparator and compare two elements
* with that provided comparator.
*
* @param	[in]	element_a : a pointer
* @param	[in]	element_b : another pointer
* @param	[in]	equals : a pointer to a function that checks
* @return	TRUE if element_a and element_b are equals considering the provided equality tester function equals; FALSE else
*/
static unsigned int mappinglist_equals(void * element_a, void * element_b, unsigned int (*equals)(void *, void *)){
	if(equals == NULL){
		equals = &pointer_equals;
	}
	return equals(element_a, element_b);
}
