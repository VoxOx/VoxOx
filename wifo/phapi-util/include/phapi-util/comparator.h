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

#ifndef __COMPARATOR_H__
#define __COMPARATOR_H__

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

	/**
	* Basic pointer equality check
	*
	* @param	[in]	element_a :	a pointer
	* @param	[in]	element_b :	another pointer
	* @return	TRUE if element_a and element_b are the same pointers; FALSE else
	*/
	PHAPIUTIL_API unsigned int pointer_equals(void * element_a, void * element_b);

	/**
	* Basic char equality check
	*
	* @param	[in]	element_a :	a pointer on a char
	* @param	[in]	element_b :	another pointer on a char
	* @return	TRUE if element_a and element_b are the same char; FALSE else
	*/
	PHAPIUTIL_API unsigned int char_equals(void * element_a, void * element_b);

	/**
	* Case insensitive string equality checker
	*
	* @param	[in]	element_a :	a pointer to a string (so a char **)
	* @param	[in]	element_b :	another pointer to a string
	* @return	TRUE if element_a and element_b are the same string; FALSE else
	*/
	PHAPIUTIL_API unsigned int string_equals(void * element_a, void * element_b);

	/**
	* Basic int equality check
	*
	* @param	[in]	element_a :	a pointer to an int
	* @param	[in]	element_b :	another pointer an int
	* @return	TRUE if element_a and element_b are the same integers; FALSE else
	*/
	PHAPIUTIL_API unsigned int int_equals(void * element_a, void * element_b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPARATOR */


