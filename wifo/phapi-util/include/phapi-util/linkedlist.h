#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <phapi-util/phapiutildll.h>

#ifndef TRUE
	#define TRUE	1
#endif /* TRUE */
#ifndef FALSE
	#define FALSE	0
#endif /*FALSE*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef struct linkedlist_link linkedlist_link_t;
	/**
	* The structure of a linked list link
	*/
	struct linkedlist_link {
		void * element;
		linkedlist_link_t * next;
	};

	typedef struct linkedlist linkedlist_t;
	/**
	* The structure of a linked list
	*/
	struct linkedlist{
		unsigned int count;
		linkedlist_link_t * first;
		linkedlist_link_t * current;
	};


	/**
	* Creates a new linked list
	*
	* @return	a new linked list
	*/
	PHAPIUTIL_API linkedlist_t * create_linkedlist();

	/**
	* Frees a linked list
	*
	* @param	[in-out]	list : a pointer to the linked list pointer
	*/
	PHAPIUTIL_API void free_linkedlist(linkedlist_t ** list);

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
	PHAPIUTIL_API unsigned int linkedlist_add(linkedlist_t * list, void * element, unsigned int (*equals)(void *, void *));

	/**
	* Removes an element of the linked list.
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in-out]	list : the list from which the element is to be removed
	* @param	[in]	element : the element to remove
	* @param	[in]	equals : a pointer to the comparator function used to check the element equality
	* @return	TRUE if the element could be removed; FALSE else
	*/
	PHAPIUTIL_API unsigned int linkedlist_remove_element(linkedlist_t * list, void * element, unsigned int (*equals)(void *, void *));

	/**
	* Checks if the list, from the current position, has a next element
	*
	* @param	[in-out]	list : a linked list
	* @return	TRUE if the list has a next element; FALSE else
	*/
	PHAPIUTIL_API unsigned int linkedlist_has_next(linkedlist_t * list);

	/**
	* Moves the current position in the list before the first element
	*
	* @param	[in-out]	list : a linked list
	*/
	PHAPIUTIL_API void linkedlist_move_before_first(linkedlist_t * list);

	/**
	* Moves the current position in the list to the first element
	*
	* @param	[in-out]	list : a linked list
	*/
	PHAPIUTIL_API void linkedlist_move_first(linkedlist_t * list);

	/**
	* Moves from the current position to the next element
	*
	* @param	[in-out]	list : a linked list
	*/
	PHAPIUTIL_API unsigned int linkedlist_move_next(linkedlist_t * list);

	/**
	* Gets the element at the current position
	*
	* @param	[in-out]	list : a linked list
	* @return	the element at the current position
	*/
	PHAPIUTIL_API void * linkedlist_get(linkedlist_t * list);

	/**
	* Removes all elements from the linked list but the elements are not freed
	*
	* @param	[in-out]	list : a linked list
	*/
	PHAPIUTIL_API void linkedlist_clear(linkedlist_t * list);

	/**
	* Check if the linked list contains an element
	* (based on a comparator function, defaults to pointer equality check if no comparator provided)
	*
	* @param	[in]	list : the list in which the element is to be found
	* @param	[in]	element : the element to find
	* @param	[in]	equals : a pointer to the comparator function used to check the element equality
	* @return	TRUE if the element could is contained in the list; FALSE else
	*/
	PHAPIUTIL_API unsigned int linkedlist_contains(linkedlist_t * list, void * element, unsigned int (*equals)(void *, void *));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LINKEDLIST_H__ */

