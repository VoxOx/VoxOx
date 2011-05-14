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

#include "fb_json.h"

#ifdef USE_JSONC

JsonParser *
json_parser_new(void)
{
	JsonParser *parser;
	
	parser = g_new0(JsonParser, 1);
	parser->tok = json_tokener_new();
	
	return parser;
}

gboolean 
json_parser_load_from_data(JsonParser *parser, const gchar *data,
                           gssize length, GError **error)
{
	if (parser->tok == NULL)
		return FALSE;

	parser->root = json_tokener_parse_ex(parser->tok, (char*)data, (int)length);
	
	if (parser->tok->err != json_tokener_success)
	{
		json_object_put(parser->root);
		parser->root = NULL;
		return FALSE;
	}
	
	return TRUE;
}

void
json_parser_free(JsonParser *parser)
{
	json_tokener_free(parser->tok);
	json_object_put(parser->root);
	g_free(parser);
}

JsonNode *
json_parser_get_root(JsonParser *parser)
{
	return parser->root;
}

GList *
json_object_get_members(JsonObject *obj)
{
	GList *keys = NULL;
	struct lh_entry *entry;	

	for (entry = json_object_get_object(obj)->head;
		entry;
		entry = entry->next)
	{
		keys = g_list_prepend(keys, entry->k);
	}
	
	keys = g_list_reverse(keys);
	
	return keys;
}

#endif
