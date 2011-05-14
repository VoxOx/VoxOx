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

#include "fb_connection.h"
#include "fb_info.h"
#include "fb_blist.h"

/*
 * TODO: Do we really want to do this?  Maybe we could just set a
 *       flag that says that this protocol supports HTML?
 */
static gchar *fb_remove_useless_stripped_links(const gchar *input)
{
	/* removes stripped links like "(/s.php? ... )" from user info */
	/* as an artifact of purple_markup_strip_html */

	gchar *output = g_strdup(input);
	gchar *i = output;
	gchar *end;

	while ((i = strstr(i, " (/")))
	{
		end = strchr(i, ')');
		if (end)
		{
			end += 1;
			/* overwrite everything after the brackets to before it */
			g_stpcpy(i, end);
		}
	}

	return output;
}

static void fb_get_info_cb(FacebookAccount *fba, gchar *data, gsize data_len, gpointer user_data)
{
	PurpleNotifyUserInfo *user_info;
	PurpleBuddyIcon *buddy_icon;
	size_t image_size;
	gconstpointer image_pointer;
	int icon_id = -1;
	gchar *uid = user_data;
	gchar *label_tmp;
	gchar *value_tmp;
	gchar *value_tmp2;
	gchar *search_start;
	gchar *search_end;
	PurpleBuddy *buddy = NULL;
	FacebookBuddy *fbuddy = NULL;

	purple_debug_info("facebook", "get_info_cb\n");
	purple_debug_info("facebook", "%s\n", data);

	buddy = purple_find_buddy(fba->account, uid);
	if (buddy)
	{
		fbuddy = buddy->proto_data;
	}
	
	user_info = purple_notify_user_info_new();

	/* Insert link to profile at top */
	value_tmp = g_strdup_printf("<a href=\"http://www.facebook.com/profile.php?id=%s\">%s</a>",
			uid, _("View web profile"));
	purple_notify_user_info_add_pair(user_info, NULL, value_tmp);
	purple_notify_user_info_add_section_break(user_info);
	g_free(value_tmp);

	/* look from <div id="info_tab" class="info_tab"> */
	/* until </div></div></div></div> */
	search_start = g_strstr_len(data, data_len, "<div id=\"info_tab\" class=\"info_tab\">");
	if (search_start == NULL)
	{
		search_start = g_strstr_len(data, data_len, "http:\\/\\/");
		if (search_start)
		{
			search_end = strstr(search_start, "\"");
			value_tmp = g_strndup(search_start, search_end - search_start);
			value_tmp2 = value_tmp;
			if (value_tmp) {
				char * buf = g_new(char, strlen(value_tmp) + 1); 
				char * url = buf;
				while(*value_tmp) { 
				if (*value_tmp=='\\') {
						// skip escape char 
						*buf++ = value_tmp[1]; 
						value_tmp += 2; 
					} else { 
						*buf++ = *value_tmp++; 
					}
				}
				*buf = 0;
				purple_debug_info("facebook", "info url: %s\n", url);
				fb_post_or_get(fba->pc->proto_data, FB_METHOD_GET, NULL, url, NULL, fb_get_info_cb, g_strdup(uid), FALSE);
				g_free(uid);
				g_free(url);
				g_free(value_tmp2);
				return;
			}
		}
		purple_debug_warning("facebook",
				"could not find user info, showing default");
		purple_notify_userinfo(fba->pc, uid, user_info, NULL, NULL);
		purple_notify_user_info_destroy(user_info);
		g_free(uid);
		return;
	}
	search_end = strstr(search_start, "</div></div></div></div>");

	value_tmp = g_strstr_len(data, data_len, "<title>Facebook | ");
	if (value_tmp)
	{
		value_tmp = strchr(value_tmp, '|')+2;
		value_tmp2 = g_strndup(value_tmp, strstr(value_tmp, "</title>")-value_tmp);
		value_tmp = g_strchomp(purple_markup_strip_html(value_tmp2));
		purple_notify_user_info_add_pair(user_info, _("Name"), value_tmp);
		fb_blist_set_alias(fba, uid, value_tmp);
		g_free(value_tmp);
		g_free(value_tmp2);
	}

	value_tmp = g_strstr_len(data, data_len, "<span id=\"profile_status\"");
	if (value_tmp)
	{
		value_tmp2 = strstr(value_tmp, "</span>");
		if (value_tmp2)
		{
			value_tmp = strchr(value_tmp, '>')+1;
			value_tmp2 = g_strndup(value_tmp, strchr(value_tmp, '<')-value_tmp);
			purple_debug_info("facebook", "status: %s\n", value_tmp2);
			value_tmp = g_strchomp(purple_markup_strip_html(value_tmp2));
			if (*value_tmp == '\0')
			{
				//For some reason their status message disappeared
				//Try using their status message from the buddy list
				if (fbuddy)
				{
					g_free(value_tmp);
					value_tmp = g_strdup(fbuddy->status);
				}
			}
			purple_notify_user_info_add_pair(user_info, _("Status"), value_tmp);
			g_free(value_tmp);
			g_free(value_tmp2);
		}
	}

	buddy_icon = purple_buddy_icons_find(fba->account, uid);
	if (buddy_icon)
	{
		image_pointer = purple_buddy_icon_get_data(buddy_icon, &image_size);
		icon_id = purple_imgstore_add_with_id(g_memdup(image_pointer, image_size), image_size, NULL);
		value_tmp = g_strdup_printf("<img id='%d'>", icon_id);
		purple_debug_info("facebook", "user info pic: '%s'\n", value_tmp);
		purple_notify_user_info_add_pair(user_info, NULL, value_tmp);
		g_free(value_tmp);
	}

	while ((search_start = strstr(search_start, "<dt>")) && search_start < search_end)
	{
		search_start += 4;
		if (search_start[0] == '<' && search_start[1] == '/' && search_start[2] == 'd' && search_start[3] == 't')
		{
			/* the tag closes as soon as it opens (bad xhtml) */
			continue;
		}

		label_tmp = g_strndup(search_start, strchr(search_start, ':')-search_start);
		if (!*label_tmp)
		{
			g_free(label_tmp);
			continue;
		}

		search_start = strstr(search_start, "<dd>");
		if (!search_start)
		{
			g_free(label_tmp);
			break;
		}

		search_start += 4;
		value_tmp = g_strndup(search_start, strstr(search_start, "</dd>")-search_start);
		if (!*value_tmp)
		{
			g_free(label_tmp);
			g_free(value_tmp);
			continue;
		}

		/* turn html to plaintext */
		if (strcmp(label_tmp, "AIM")) {
			value_tmp2 = g_strchomp(purple_markup_strip_html(value_tmp));
			g_free(value_tmp);
			value_tmp = value_tmp2;

			/* remove the silly links */
			value_tmp2 = fb_remove_useless_stripped_links(value_tmp);
			g_free(value_tmp);
			value_tmp = value_tmp2;
		}

		purple_debug_info("facebook", "label: %s\n", label_tmp);
		purple_debug_info("facebook", "value: %s\n", value_tmp);
		purple_notify_user_info_add_pair(user_info, label_tmp, value_tmp);
		g_free(label_tmp);
		g_free(value_tmp);
	}

	purple_notify_userinfo(fba->pc, uid, user_info, NULL, NULL);
	purple_notify_user_info_destroy(user_info);

	if (icon_id >= 0)
		purple_imgstore_unref_by_id(icon_id);

	g_free(uid);
}

void fb_get_info(PurpleConnection *pc, const gchar *uid)
{
	gchar *profile_url;

	profile_url = g_strdup_printf("/profile.php?id=%s&v=info", uid);

	fb_post_or_get(pc->proto_data, FB_METHOD_GET, NULL, profile_url, NULL, fb_get_info_cb, g_strdup(uid), FALSE);

	g_free(profile_url);
}
