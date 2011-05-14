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

#include <phapi-util/mystring.h>

#include <phapi-util/comparator.h>


/**
* Basic pointer equality check
*
* @param	[in]	element_a :	a pointer
* @param	[in]	element_b :	another pointer
* @return	TRUE if element_a and element_b are the same pointers; FALSE else
*/
unsigned int pointer_equals(void * element_a, void * element_b){
	if(element_a == element_b){
		return TRUE;
	}
	return FALSE;
}

/**
* Basic char equality check
*
* @param	[in]	element_a :	a pointer on a char
* @param	[in]	element_b :	another pointer on a char
* @return	TRUE if element_a and element_b are the same char; FALSE else
*/
unsigned int char_equals(void * element_a, void * element_b){
	char _a;
	char _b;

	if(element_a != NULL && element_b != NULL){
		_a = *(char *)element_a;
		_b = *(char *)element_b;

		if((_a) == (_b)){
			return TRUE;
		}
	}
	return FALSE;
}

/**
* Case insensitive string equality checker
*
* @param	[in]	element_a :	a pointer to a string (so a char **)
* @param	[in]	element_b :	another pointer to a string
* @return	TRUE if element_a and element_b are the same string; FALSE else
*/
unsigned int string_equals(void * element_a, void * element_b){
	char * _a;
	char * _b;

	if(element_a != NULL && element_b != NULL){
		_a = (char *)element_a;
		_b = (char *)element_b;

		if(strcasecmp(_a, _b) == 0){
			return TRUE;
		}
	}
	return FALSE;
}

/**
* Basic int equality check
*
* @param	[in]	element_a :	a pointer to an int
* @param	[in]	element_b :	another pointer an int
* @return	TRUE if element_a and element_b are the same integers; FALSE else
*/
unsigned int int_equals(void * element_a, void * element_b){
	int _a;
	int _b;

	if(element_a != NULL && element_b != NULL){
		_a = *(int *)element_a;
		_b = *(int *)element_b;

		if((_a) == (_b)){
			return TRUE;
		}
	}
	return FALSE;
}
