#include <phapi-util/linkedlist.h>
#include <phapi-util/comparator.h>

#include <stdlib.h>

// ----- PRIVATE FUNCTIONS DECLARATION -----
static unsigned int linkedlist_equals(void * element_a, void * element_b, unsigned int (*equals)(void *, void *));
static linkedlist_link_t * create_linkedlist_link(void * element);
static void free_linkedlist_link(linkedlist_link_t ** link);
// -----


/**
* Creates a new linked list
*
* @return	a new linked list
*/
linkedlist_t * create_linkedlist(){
	linkedlist_t * list;

	list = (linkedlist_t *)malloc(sizeof(linkedlist_t));
	list->count = 0;
	list->first = NULL;
	list->current = NULL;

	return list;
}

/**
* Frees a linked list
*
* @param	[in-out]	list : a pointer to the linked list pointer
*/
void free_linkedlist(linkedlist_t ** list){
	if(list != NULL && *list != NULL){
		linkedlist_clear(*list);
		free(*list);
		*list = NULL;
	}
}

/**
* Creates a new linked list link pointing to the given element
*
* @param	[in]	element : the element to be attached to that link
* @return	[out]	a new linked list link
*/
static linkedlist_link_t * create_linkedlist_link(void * element){
	linkedlist_link_t * link;

	link = (linkedlist_link_t *)malloc(sizeof(linkedlist_link_t));
	link->element = element;
	link->next = NULL;

	return link;
}

/**
* Frees the memory associated to a linked list link
*
* Example:
* <pre>
*	linkedlist_link_t * link;
*	free_linkedlist_link(&link);
* </pre>
*
* @param	[in-out]	link : a pointer to the linked list pointer to be freed and turned into NULL
*/
static void free_linkedlist_link(linkedlist_link_t ** link){
	if(link != NULL && *link != NULL){
		(*link)->element = NULL;
		(*link)->next = NULL;
		free(*link);
		*link = NULL;
	}
}

/**
* Adds an element at the end of the linked list.
* An element cannot be added more than once.
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in-out]	list : the list that receives the element
* @param	[in]	element : the element to add
* @param	[in]	equals : a pointer to the comparator function used to check the element equality
* @return	TRUE if the element could be added; FALSE else
*/
unsigned int linkedlist_add(linkedlist_t * list, void * element, unsigned int (*equals)(void *, void *)){
	linkedlist_link_t * new_link;
	linkedlist_link_t * link;

	if(!linkedlist_contains(list, element, equals)){
		new_link = create_linkedlist_link(element);

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
	return FALSE;
}

/**
* Removes an element of the linked list.
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in-out]	list : the list from which the element is to be removed
* @param	[in]	element : the element to remove
* @param	[in]	equals : a pointer to the comparator function used to check the element equality
* @return	TRUE if the element could be removed; FALSE else
*/
unsigned int linkedlist_remove_element(linkedlist_t * list, void * element, unsigned int (*equals)(void *, void *)){
	linkedlist_link_t * link;
	linkedlist_link_t * previous_link;

	if(list->count > 0){
		link = list->first;
		if(linkedlist_equals(link->element, element, equals)){
			// moves the current element to NULL
			if(list->current != NULL && list->current->element == element){
				list->current = NULL;
			}
			// first receives next, or NULL if no more
			list->first = link->next;
			free_linkedlist_link(&link);
			list->count--;
			return TRUE;
		}
		while(link->next != NULL){
			previous_link = link;
			link = link->next;
			if(linkedlist_equals(link->element, element, equals)){
				// moves the current element to the previous
				if(list->current != NULL && list->current->element == element){
					list->current = previous_link;
				}
				// previous receives next, or NULL if no more
				previous_link->next = link->next;
				free_linkedlist_link(&link);
				list->count--;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
* Checks if the list, from the current position, has a next element
*
* @param	[in-out]	list : a linked list
* @return	TRUE if the list has a next element; FALSE else
*/
unsigned int linkedlist_has_next(linkedlist_t * list){
	if (!list){
		return FALSE;
	}

	if(list->current == NULL){
		if(list->first != NULL){
			return TRUE;
		}
		return FALSE;
	}
	if(list->current->next != NULL){
		return TRUE;
	}
	return FALSE;
}

/**
* Moves the current position in the list before the first element
*
* @param	[in-out]	list : a linked list
*/
void linkedlist_move_before_first(linkedlist_t * list){
	list->current = NULL;
}

/**
* Moves the current position in the list to the first element
*
* @param	[in-out]	list : a linked list
*/
void linkedlist_move_first(linkedlist_t * list){
	list->current = list->first;
}

/**
* Moves from the current position to the next element
*
* @param	[in-out]	list : a linked list
*/
unsigned int linkedlist_move_next(linkedlist_t * list){
	if(linkedlist_has_next(list)){
		if(list->current == NULL){
			list->current = list->first;
			return TRUE;
		}
		list->current = list->current->next;
		return TRUE;
	}
	return FALSE;
}

/**
* Gets the element at the current position
*
* @param	[in-out]	list : a linked list
* @return	the element at the current position
*/
void * linkedlist_get(linkedlist_t * list){
	return list->current->element;
}

/**
* Removes all elements from the linked list but the elements are not freed
*
* @param	[in-out]	list : a linked list
*/
void linkedlist_clear(linkedlist_t * list){
	linkedlist_link_t * link;
	linkedlist_link_t * temp_link;

	temp_link = NULL;
	list->current = NULL;
	if(list->count > 0){
		link = list->first;
		while(link->next != NULL){
			temp_link = link->next;
			free_linkedlist_link(&link);
			list->count--;
			link = temp_link;
		}
		free_linkedlist_link(&link); // last one
		list->count--;
		list->first = NULL;
	}
}

/**
* Check if the linked list contains an element
* (based on a comparator function, defaults to pointer equality check if no comparator provided)
*
* @param	[in]	list : the list in which the element is to be found
* @param	[in]	element : the element to find
* @param	[in]	equals : a pointer to the comparator function used to check the element equality
* @return	TRUE if the element could is contained in the list; FALSE else
*/
unsigned int linkedlist_contains(linkedlist_t * list, void * element, unsigned int (*equals)(void *, void *)){
	linkedlist_link_t * link;

	if(list->count > 0){
		link = list->first;
		if(linkedlist_equals(link->element, element, equals)){
			return TRUE;
		}
		while(link->next != NULL){
			link = link->next;
			if(linkedlist_equals(link->element, element, equals)){
				return TRUE;
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
static unsigned int linkedlist_equals(void * element_a, void * element_b, unsigned int (*equals)(void *, void *)){
	if(equals == NULL){
		equals = &pointer_equals;
	}
	return equals(element_a, element_b);
}
