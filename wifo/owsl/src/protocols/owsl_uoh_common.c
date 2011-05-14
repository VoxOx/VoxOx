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

#include <stdlib.h>
#include <string.h>

#include <owlist.h>

#include "owsl_base.h"
#include "owsl_uoh_common.h"

/* ******************************************************** */
/*          Configuration Parameters management             */
/* ******************************************************** */

const OWSLParamKey_UoH *_owsl_uoh_parameter_key_get(const OWSLParamKey_UoH *OWSLParamKeyList_UoH,
													const char * name)
{
	int i;

	for (i = 0; &OWSLParamKeyList_UoH[i]; i++)
	{
		if (strcmp(name, OWSLParamKeyList_UoH[i].name) == 0)
		{
			return &OWSLParamKeyList_UoH[i];
		}
	}

	return 0;
}

int _owsl_uoh_parameter_key_compare(OWSLParamNode_UoH *elm1, OWSLParamNode_UoH *elm2)
{
	return strcmp(elm1->key->name, elm2->key->name);
}

OWSLDataType_UoH *owsl_uoh_parameter_value_get(OWList * list, const char * name)
{
	OWListIterator *it;
	OWSLParamNode_UoH node;
	OWSLParamKey_UoH key;
	static OWSLDataType_UoH value;
	
	value.i = 0;
	if (!(it = owlist_iterator_new(list, OWLIST_READ)))
	{
		return &value;
	}

	while (owlist_iterator_next(it) == 0)
	{
		node.key = &key;
		key.name = name;
		if (_owsl_uoh_parameter_key_compare(owlist_iterator_get(it), &node) == 0)
		{
			value = ((OWSLParamNode_UoH *)owlist_iterator_get(it))->value;
			owlist_iterator_free(it);
			return &value;
		}
	}

	owlist_iterator_free(it);
	return &value;
}

int owsl_uoh_common_parameter_set(const OWSLParamKey_UoH *OWSLParamKeyList_UoH,
								  const char * name,
								  const void * value,
								  struct OWList * list)
{
	OWSLParamNode_UoH *node = 0;
	const OWSLParamKey_UoH *key;

	if (name == 0 && *name == 0)
	{
		return -1;
	}

	if (!(key = _owsl_uoh_parameter_key_get(OWSLParamKeyList_UoH, name)))
	{
		return -1;
	}

	if (!(node = malloc(sizeof(OWSLParamNode_UoH))))
	{
		return -1;
	}

	node->key = key;
	switch (key->type)
	{
		case USHORT_PARAM:
			node->value.i = *((unsigned short *)value);
			break;

		case INT_PARAM:
			node->value.i = *((int *)value);
			break;

		case STR_PARAM:
			node->value.str = strdup((const char *) value);
			break;

		default:
			// TODO: LOG : unknown parameter type
			free(node);
			return -1;
	}

	owlist_add(list, node, 0);

	return 0;
}
