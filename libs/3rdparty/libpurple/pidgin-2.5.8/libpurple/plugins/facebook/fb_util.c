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

#include "fb_util.h"
/*****************************************************************************
 * UTILITY CODE                                                              *
 *****************************************************************************/

gchar *fb_convert_unicode(const gchar *input)
{
	/* \u00e9t\u00e9 should be été */

	gunichar unicode_char;
	gchar unicode_char_str[6];
	gint unicode_char_len;
	gchar *next_pos;
	gchar *input_string;
	gchar *output_string;

	if (input == NULL)
		return NULL;

	next_pos = input_string = g_strdup(input);

	/* purple_debug_info("facebook", "unicode convert: in: %s\n", input); */
	while ((next_pos = strstr(next_pos, "\\u")))
	{
		/* grab the unicode */
		sscanf(next_pos, "\\u%4x", &unicode_char);
		/* turn it to a char* */
		unicode_char_len = g_unichar_to_utf8(unicode_char, unicode_char_str);
		/* shove it back into the string */
		g_memmove(next_pos, unicode_char_str, unicode_char_len);
		/* move all the data after the \u0000 along */
		g_stpcpy(next_pos + unicode_char_len, next_pos + 6);
	}

	/* purple_debug_info("facebook", "unicode convert: out: %s\n", input); */
	output_string = g_strcompress(input_string);
	g_free(input_string);

	return output_string;
}

/* Like purple_strdup_withhtml, but escapes htmlentities too */
gchar *fb_strdup_withhtml(const gchar *src)
{
	gulong destsize, i, j;
	gchar *dest;

	g_return_val_if_fail(src != NULL, NULL);

	/* New length is (length of src) + (number of \n's * 3) + (number of &'s * 5) +
		(number of <'s * 4) + (number of >'s *4) + (number of "'s * 6) -
		(number of \r's) + 1 */
	destsize = 1;
	for (i = 0; src[i] != '\0'; i++)
	{
		if (src[i] == '\n' || src[i] == '<' || src[i] == '>')
			destsize += 4;
		else if (src[i] == '&')
			destsize += 5;
		else if (src[i] == '"')
			destsize += 6;
		else if (src[i] != '\r')
			destsize++;
	}

	dest = g_malloc(destsize);

	/* Copy stuff, ignoring \r's, because they are dumb */
	for (i = 0, j = 0; src[i] != '\0'; i++) {
		if (src[i] == '\n') {
			strcpy(&dest[j], "<BR>");
			j += 4;
		} else if (src[i] == '<') {
			strcpy(&dest[j], "&lt;");
			j += 4;
		} else if (src[i] == '>') {
			strcpy(&dest[j], "&gt;");
			j += 4;
		} else if (src[i] == '&') {
			strcpy(&dest[j], "&amp;");
			j += 5;
		} else if (src[i] == '"') {
			strcpy(&dest[j], "&quot;");
			j += 6;
		} else if (src[i] != '\r')
			dest[j++] = src[i];
	}

	dest[destsize-1] = '\0';

	return dest;
}
static gboolean is_json_64bit_safe()
{
	//Cache the result to try make this function quick
	static gint result = -1;
	gint64 largeint = G_MAXINT64;
	
	if (result == 1)
		return TRUE;
	if (result == 0)
		return FALSE;
	
	if (sizeof(gint) >= sizeof(gint64))
	{
		result = 1;
		return TRUE;
	}

//#ifndef USE_JSONC
//	JsonNode *node;
//
//	node = json_node_new(JSON_NODE_VALUE);
//	json_node_set_int(node, largeint);
//	if(json_node_get_int(node) == largeint)
//	{
//		result = 1;
//	} else {
//		result = 0;
//	}
//	json_node_free(node);
//	return result ? TRUE : FALSE;
//	
//#endif /* !USE_JSONC */
	
	result = 0;
	return FALSE;
}

gint64 fb_time_kludge(gint64 initial_time)
{
	gint64 now_millis;//VOXOX - CJC - 2009.07.17 
	gint64 final_time;//VOXOX - CJC - 2009.07.17 
	if (is_json_64bit_safe())
		return initial_time;
	
	now_millis = (gint64) time(NULL);
	now_millis *= 1000;
	now_millis &= 0xFFFFFFFF00000000LL;
	final_time = now_millis | ((guint)initial_time);

	return final_time;
}

JsonParser *fb_get_parser(const gchar *data, gsize data_len)
{
	JsonParser *parser;

	if (data == NULL) {
		return NULL;
	}

	data = g_strstr_len(data, data_len, "for (;;);");
	if (!data) {
		return NULL;
	} else {
		data += strlen("for (;;);");
	}

	parser = json_parser_new();
	if (!json_parser_load_from_data(parser, data, -1, NULL)) {
		json_parser_free(parser);
		return NULL;
	}

	return parser;
}

JsonObject *fb_get_json_object(JsonParser *parser, char **error_message)
{
	JsonNode *root;//VOXOX - CJC - 2009.07.17 
	JsonObject *objnode;//VOXOX - CJC - 2009.07.17 
	root = json_parser_get_root(parser);
	objnode = json_node_get_object(root);

	/* Sample error messages */
	/* for (;;);{"error":1357001,"errorSummary":"Not Logged In",
		"errorDescription":"You must be logged in to do that.",
		"payload":null,"bootload":[{"name":"js\/common.js.pkg.php",
		"type":"js","src":"http:\/\/static.ak.fbcdn.net\/rsrc.php\/pkg\/59\
		/98561\/js\/common.js.pkg.php"}]} */
	if (json_object_has_member(objnode, "error"))
	{
		guint32 error_number;
		const char *summary;
		const char *description;

		error_number = json_node_get_int(
			json_object_get_member(objnode, "error"));
		summary = json_node_get_string(
			json_object_get_member(objnode, "errorSummary"));
		description = json_node_get_string(
			json_object_get_member(objnode, "errorDescription"));

		if (error_number)
		{
			purple_debug_error("facebook",
				"got error from facebook of %s (%s)",
				summary, description);
			// Pass error message to calling function if they asked for it.
			if (error_message) {
				*error_message = g_strdup(description);
			}
		}
	}

	return objnode;
}

/* Converts *text* into <b>text</b>  and _text_ into <i>text</i> */
gchar *fb_replace_styled_text(const gchar *text)
{
#ifdef __ARM_EABI__
	return g_strdup(text);
#else /*__ARM_EABI__*/
#if GLIB_MAJOR_VERSION >= 2 && GLIB_MINOR_VERSION >= 14
	if (glib_check_version(2, 14, 0))
	{
		return g_strdup(text);
	} else {
		static GRegex *underline_regex = NULL;
		static GRegex *bold_regex = NULL;
		gchar *dup_text;
		gchar *midway_string;
		gchar *output_string;
		
		if (underline_regex == NULL)
		{
			underline_regex = g_regex_new(
				"\\b_([^_\\*]+)_\\b", G_REGEX_OPTIMIZE,
				0, NULL);
		}
		if (bold_regex == NULL)
		{
			bold_regex = g_regex_new(
				"(\\s|^)\\*([^_\\*]+)\\*(?=$|\\s)",
				G_REGEX_OPTIMIZE, 0, NULL);
		}
		
		dup_text = g_strdup(text);
		midway_string = g_regex_replace(underline_regex, dup_text,
			strlen(dup_text), 0, "<u>\\1</u>", 0, NULL);
		if (midway_string == NULL)
		{
			purple_debug_warning("facebook", "regex failed for underline\n");
			return dup_text;
		}
		g_free(dup_text);
		output_string = g_regex_replace(bold_regex, midway_string,
			strlen(midway_string), 0, "\\1<b>\\2</b>", 0, NULL);
		if (output_string == NULL)
		{
			purple_debug_warning("facebook", "regex failed for bold\n");
			return midway_string;
		}
		g_free(midway_string);
		
		return output_string;
	}
#else /* GLIB check */
	return g_strdup(text);
#endif /* GLIB check */
#endif /*__ARM_EABI__*/
}

