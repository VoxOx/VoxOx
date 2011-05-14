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

#ifndef _NETLIB_UTIL_H
#define _NETLIB_UTIL_H

char *make_error_string(const char *str);
char *my_strndup(const char *str, int size);
char *goto_next_elm(char *str, char sep);
char *goto_elm_end(char *str, char sep);
int count_elem(char *str, char sep);
char **my_split(const char *str, char sep);
void free_str_list(char **list);
char *get_domain_url(char *url);
bool domain_url_cmp(char *url1, char *url2);

#endif
