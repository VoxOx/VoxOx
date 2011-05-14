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

#include <string.h>
#include <stdlib.h>

char *strndup(const char *str, int size)
{
	char *newstr, *start;
	int newsize;
	int i;

	if (!str)
		return 0;
	
	newsize = (strlen(str) < size ? strlen(str) : size);

	newstr = (char *) malloc(newsize + 1);
	
	start = newstr;
	for (i = 0; *str && i < newsize; i++)
		*newstr++ = *str++;

	*newstr = 0;
	return start;
}
