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

#include <owcommon.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef OS_WINDOWS
	#include <windows.h>
	#include <winbase.h>
#else
	#include <errno.h>
#endif

#if defined(CC_MSVC) || defined(CC_MINGW) || defined(OS_MACOSX)
	#include "strndup.h"
#endif

char *make_error_string(const char *str)
{
	static char buffer[256];

	memset(buffer, 0, sizeof(buffer));
#ifdef OS_WIN32

	LPTSTR ptr = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ptr,
		0,
		NULL);
	_snprintf(buffer, sizeof(buffer), "%s: (%d) %s", str, GetLastError(), ptr);
	LocalFree(ptr);
#else
	snprintf(buffer, sizeof(buffer), "%s: (%d) %s\n", str, errno, strerror(errno));
#endif

	return buffer;
}

char *goto_next_elm(char *str, char sep)
{
	while (*str && (*str == sep || *str == ' '))
		str++;

	while (*str && *str != sep)
		str++;

	while (*str && (*str == sep || *str == ' '))
		str++;

	return str;
}

char *goto_elm_end(char *str, char sep)
{
	char *start;

	start = str;

	while (*str && *str != sep)
		str++;

	if (*(str - 1) == ' ')
	{
		for (str--; str != start && *str == ' '; str--);
		str++;
	}

	return str;
}

int count_elem(char *str, char sep)
{
	int nbr;

	if (!str)
		return 0;

	for (nbr = 0; *str; nbr++)
		str = goto_next_elm(str, sep);

	return nbr;
}

char **my_split(const char *str, char sep)
{
	char **list;
	char *tmp;
	int nbr_elm;
	int	i;

	if (str == 0 || *str == 0)
		return 0;

	while (*str && (*str == sep || *str == ' '))
		str++;

	if ((nbr_elm = count_elem((char *)str, sep)) == 0)
		return 0;

	list = (char **) malloc((nbr_elm + 1) * sizeof(char *));

	for (i = 0, tmp = (char*) str; i <= nbr_elm; i++)
	{
		if (!tmp || !*tmp)
			break;

		list[i] = strndup(tmp, (int)(goto_elm_end(tmp, sep) - tmp));
		tmp = goto_next_elm(tmp, sep);
	}

	list[i] = 0;

	return list;
}

void free_str_list(char **list)
{
	int i;

	if (!list)
		return;

	for (i = 0; list[i]; i++)
		free((void *)list[i]);

	free((void *)list);
}

char *get_domain_url(char *url)
{
	char *start;

	if (!url)
		return 0;

	for (start = url; *url && (*url != '/' && *url != ':'); url++);

	return strndup(start, url - start);
}

bool domain_url_cmp(char *url1, char *url2)
{
	char *start;

	if (!url1 && !url2)
		return false;
	else if (!url1 || !url2)
		return true;

	url1 = get_domain_url(url1);
	start = url1;

	while (*url1 && *url2)
	{
		if (*url1 == *url2)
		{
			url1++;
			url2++;
		}
		else if (*url1 == '*')
		{
			while (*url2 && *(url1 + 1) != *url2)
				url2++;

			if (!*url2)
			{
				free(start);
				return false;
			}
			url1++;
		}
		else if (*url2 == '*')
		{
			while (*url1 && *(url2 + 1) != *url1)
				url1++;

			if (!*url1)
			{
				free(start);
				return false;
			}
			url2++;
		}
		else
		{
			free(start);
			return true;
		}
	}

	free(start);
	return false;
}
