/*
 * libfacebook
 *
 * libfacebook is the property of its developers.  See the COPYRIGHT file
 * for more details.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FACEBOOK_UTIL_H
#define FACEBOOK_UTIL_H

#include "libfacebook.h"

#include "fb_json.h"


JsonParser *fb_get_parser(const gchar *data, gsize data_len);
JsonObject *fb_get_json_object(JsonParser *parser, char **error_message);

gchar *fb_replace_styled_text(const gchar *text);
gchar *fb_strdup_withhtml(const gchar *src);
gchar *fb_convert_unicode(const gchar *input);
gint64 fb_time_kludge(gint64 initial_time);

#endif /* FACEBOOK_UTIL_H */

