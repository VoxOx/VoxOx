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

#ifndef FACEBOOK_JSON_H
#define FACEBOOK_JSON_H

#ifndef USE_JSONC
#	include "json-glib/json-glib.h"
#define json_parser_free(parser) g_object_unref(parser)
#else /* USE_JSONC */
#	include <glib.h>
#	include <json/json.h>
typedef struct json_object JsonNode;
typedef struct json_object JsonObject;
typedef struct json_object JsonArray;
typedef struct {
	struct json_tokener *tok;
	struct json_object *root;
} JsonParser;

gboolean json_parser_load_from_data(JsonParser *parser,
									const gchar *data,
                                    gssize length,
                                    GError **error);

JsonNode* json_parser_get_root(JsonParser *parser);
JsonParser* json_parser_new(void);
void json_parser_free(JsonParser *parser);

#define json_object_has_member(obj, key) ((gboolean)json_object_object_get(obj, key))
#define json_object_get_member(obj, key) json_object_object_get(obj, key)
GList* json_object_get_members(JsonObject *object);

#define json_node_get_array(node) (node)
#define json_node_get_object(node) (node)
#define json_node_get_boolean(node) json_object_get_boolean(node)
#define json_node_get_double(node) json_object_get_double(node)
#define json_node_get_int(node) json_object_get_int(node)
#define json_node_get_string(node) json_object_get_string(node)

#define json_array_get_element(array, index) json_object_array_get_idx(array, index)
#define json_array_get_length(array) json_object_array_length(array)

#endif /* USE_JSONC */

#endif /* FACEBOOK_JSON_H */

