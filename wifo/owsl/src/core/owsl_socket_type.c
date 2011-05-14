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

#include "owsl_internal.h"

#include <owlist.h>

#include <stdlib.h>
#include <memory.h>

/* all socket type infos are stored in an array */
static OWSLSocketTypeInfo owsl_socket_type_info_array [OWSL_TYPE_MAX] ;
/* types are registered in a list to allow their use */
static OWList * owsl_socket_type_list = NULL ;

int
owsl_socket_type_initialize_all
(void)
{
	OWSLSocketType type ;

	/* init socket type info array */
	memset (owsl_socket_type_info_array, 0, sizeof (owsl_socket_type_info_array)) ;

	/* init each socket type */
	if (owsl_tcp_initialize ())
	{
		return -1 ;
	}
	if (owsl_udp_initialize ())
	{
		return -1 ;
	}
	if (owsl_tls_initialize ())
	{
		return -1 ;
	}
	if (owsl_uoh_initialize ())
	{
		return -1 ;
	}
	if (owsl_uohs_initialize ())
	{
		return -1 ;
	}

	/* check if all socket types are initialized */
	for (type = 0 ; type < OWSL_TYPE_MAX ; type ++)
	{
		if (owsl_socket_type_info_get (type)->type != type)
		{
			return -1 ;
		}
	}

	/* init socket type list */
	owsl_socket_type_list = owlist_new () ;
	if (owsl_socket_type_list == NULL)
	{
		return -1 ;
	}

	return 0 ;
}

int
owsl_socket_type_terminate_all
(void)
{
	int return_code = 0 ;

	/* free socket type list */
	return_code |= owlist_free_all (owsl_socket_type_list, free) ;
	owsl_socket_type_list = NULL ;

	/* terminate socket types */
	return_code |= owsl_tls_terminate () ;

	return return_code ;
}

int
owsl_global_parameter_set
(
	const char * name,
	const void * value
)
{
	OWSLSocketType type ;

	for (type = 0 ; type < OWSL_TYPE_MAX ; type ++)
	{
		if (owsl_socket_type_info_get (type)->global_parameter_set != NULL)
		{
			if (owsl_socket_type_info_get (type)->global_parameter_set (name, value))
			{
				return -1 ;
			}
		}
	}

	return 0 ;
}

int
owsl_socket_type_initialize
(
	OWSLSocketTypeInfo * info
)
{
	memcpy (owsl_socket_type_info_get (info->type), info, sizeof (OWSLSocketTypeInfo)) ;
	return 0 ;
}

static int
owsl_socket_type_compare
(
	const void * void1,
	const void * void2
)
{
	const OWSLSocketType * type1 = void1 ;
	const OWSLSocketType * type2 = void2 ;
	if (type1 < type2)
	{
		return -1 ;
	}
	else if (type1 == type2)
	{
		return 0 ;
	}
	else
	{
		return 1 ;
	}
}

int
owsl_socket_type_add
(
	OWSLSocketType type
)
{
	OWSLSocketType * allocated_type = malloc (sizeof (OWSLSocketType)) ;
	if (allocated_type == NULL)
	{
		return -1 ;
	}
	* allocated_type = type ;
	if (owlist_add (owsl_socket_type_list, allocated_type, owsl_socket_type_compare))
	{
		free (allocated_type) ;
		return -1 ;
	}
	return 0 ;
}

int
owsl_socket_type_remove
(
	OWSLSocketType type
)
{
	OWListIterator * iterator ;
	OWSLSocketType * type_from_list ;
	int found = 0, return_code = 0 ;

	/* search type in the list */
	iterator = owlist_iterator_new (owsl_socket_type_list, OWLIST_WRITE) ;
	if (iterator == NULL)
	{
		return -1 ;
	}
	while (owlist_iterator_next (iterator) == 0)
	{
		type_from_list = owlist_iterator_get (iterator) ;
		if (* type_from_list == type)
		{
			found = 1 ;
			break ;
		}
	}

	/* remove type */
	if (found)
	{
		if (owlist_iterator_remove (iterator))
		{
			return_code = -1 ;
		}
		free (type_from_list) ;
	}

	if (owlist_iterator_free (iterator))
	{
		return_code = -1 ;
	}

	return return_code ;
}

int
owsl_socket_type_contains
(
	OWSLSocketType type
)
{
	return owlist_contains (owsl_socket_type_list, & type, owsl_socket_type_compare) ;
}

OWSLSocketType
owsl_socket_type_get
(
	OWSLAddressFamily address_family,
	OWSLSocketMode mode,
	OWSLCiphering ciphering
)
{
	OWListIterator * type_iterator ;
	OWSLSocketType * type_from_list, matching_type = OWSL_TYPE_UNKNOWN ;
	OWSLSocketTypeInfo * type_info ;

	type_iterator = owlist_iterator_new (owsl_socket_type_list, OWLIST_READ) ;
	if (type_iterator == NULL)
	{
		return OWSL_TYPE_UNKNOWN ;
	}
	while (owlist_iterator_next (type_iterator) == 0)
	{
		type_from_list = owlist_iterator_get (type_iterator) ;
		type_info = owsl_socket_type_info_get (* type_from_list) ;
		if
		(
			(address_family == OWSL_AF_UNDEFINED || type_info->address_family == OWSL_AF_UNDEFINED || address_family == type_info->address_family)
			&&
			(mode == OWSL_MODE_UNDEFINED || type_info->mode == OWSL_MODE_UNDEFINED || mode == type_info->mode)
			&&
			(ciphering == OWSL_CIPHERING_UNDEFINED || type_info->ciphering == OWSL_CIPHERING_UNDEFINED || ciphering == type_info->ciphering)
		)
		{
			if (matching_type == OWSL_TYPE_UNKNOWN)   /* first matching type */
			{
				matching_type = * type_from_list ;
			}
			else   /* more than one matching type */
			{
				matching_type = OWSL_TYPE_UNKNOWN ;
				break ;
			}
		}
	}
	if (owlist_iterator_free (type_iterator))
	{
		return OWSL_TYPE_UNKNOWN ;
	}

	return matching_type ;
}

OWSLSocketTypeInfo *
owsl_socket_type_info_get
(
	OWSLSocketType type
)
{
	if (type < 0 || type >= OWSL_TYPE_MAX)
	{
		return NULL ;
	}
	return & owsl_socket_type_info_array [type] ;
}

OWSLAddressFamily
owsl_socket_type_address_family_get
(
	OWSLSocketType type
)
{
	OWSLSocketTypeInfo * type_info = owsl_socket_type_info_get (type) ;
	if (type_info == NULL)
	{
		return OWSL_AF_UNDEFINED ;
	}
	return type_info->address_family ;
}

OWSLSocketMode
owsl_socket_type_mode_get
(
	OWSLSocketType type
)
{
	OWSLSocketTypeInfo * type_info = owsl_socket_type_info_get (type) ;
	if (type_info == NULL)
	{
		return OWSL_MODE_UNDEFINED ;
	}
	return type_info->mode ;
}

OWSLCiphering
owsl_socket_type_ciphering_get
(
	OWSLSocketType type
)
{
	OWSLSocketTypeInfo * type_info = owsl_socket_type_info_get (type) ;
	if (type_info == NULL)
	{
		return OWSL_CIPHERING_UNDEFINED ;
	}
	return type_info->ciphering ;
}
