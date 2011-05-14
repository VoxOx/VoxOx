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

#include "libfacebook.h"
#include "fb_connection.h"
#include "fb_info.h"
#include "fb_search.h"

static void fb_searchresults_add_buddy(PurpleConnection *pc, GList *row,
		void *user_data)
{
	PurpleAccount *account = purple_connection_get_account(pc);

	if (!purple_find_buddy(account, g_list_nth_data(row, 0)))
		purple_blist_request_add_buddy(account,
				g_list_nth_data(row, 0), NULL, NULL);
}

static void fb_searchresults_info_buddy(PurpleConnection *pc, GList *row,
		void *user_data)
{
	/* PurpleAccount *account = purple_connection_get_account(pc); */

	/* if (purple_find_buddy(account, g_list_nth_data(row, 0))) */
		fb_get_info(pc, g_list_nth_data(row, 0));
}

static void fb_found_friends(FacebookAccount *fba, gchar *data,
		gsize data_len, gpointer user_data)
{
	PurpleNotifySearchResults *results;
	PurpleNotifySearchColumn *column;
	gchar *id, *tmp, *stripped, *last_id_pos = 0, *id_pos = data;
	gchar *search_term = user_data;
	const gchar *id_search_term =
			"facebook.com/inbox/?compose&amp;id="; /* " */
	const gchar *name_search_term = "class=\"url fn\""; /* < */
	const gchar *network_search_term = "class=\"result_network\">"; /* < */

	if (!g_strstr_len(data, data_len, id_search_term))
	{
		/* there's no friends found; notify as such */
		tmp = g_strdup_printf(_("No results found for %s"),
				search_term);
		purple_notify_error(fba->pc, NULL, tmp, NULL);
		g_free(tmp);
		g_free(search_term);
		return;
	}

	results = purple_notify_searchresults_new();
	if (results == NULL)
	{
		g_free(search_term);
		return;
	}

	/* columns: Facebook ID, Name, Network */
	column = purple_notify_searchresults_column_new(_("ID"));
	purple_notify_searchresults_column_add(results, column);
	column = purple_notify_searchresults_column_new(_("Name"));
	purple_notify_searchresults_column_add(results, column);
	column = purple_notify_searchresults_column_new(_("Network"));
	purple_notify_searchresults_column_add(results, column);
	column = purple_notify_searchresults_column_new(_("In List?"));
	purple_notify_searchresults_column_add(results, column);

	if (purple_account_get_bool(fba->account, "facebook_manage_friends",
				FALSE)) {
		purple_notify_searchresults_button_add(results,
				PURPLE_NOTIFY_BUTTON_ADD,
				fb_searchresults_add_buddy);
	}
	purple_notify_searchresults_button_add(results,
			PURPLE_NOTIFY_BUTTON_INFO,
			fb_searchresults_info_buddy);

	purple_debug_info("facebook", "found_friends\n");
	while ((id_pos = strstr(id_pos, id_search_term)))
	{
		/* the row in the search results table */
		/* prepend to it backwards then reverse to speed up adds */
		GList *row = NULL;

		/* grab id */
		id_pos += strlen(id_search_term);
		if (strchr(id_pos, '&') < strchr(id_pos, '"'))
		{
			/* new layout repeats the id */
			continue;
		}
		id = g_strndup(id_pos, strchr(id_pos, '"')-id_pos);
		purple_debug_info("facebook", "Found user with id: %s\n", id);
		row = g_list_prepend(row, id);

		/* look for name */
		tmp = g_strrstr_len(data, id_pos-data, name_search_term);
		if (tmp && tmp > last_id_pos)
		{
			tmp += strlen(name_search_term);
			tmp = strchr(tmp, '>') + 1;
			tmp = g_strndup(tmp, strchr(tmp, '<')-tmp);
			stripped = purple_unescape_html(tmp);
			g_free(tmp);
			purple_debug_info("facebook", "With name: %s\n",
					stripped);
			row = g_list_prepend(row, stripped);
		} else {
			row = g_list_prepend(row, NULL);
		}

		/* look for network */
		tmp = g_strrstr_len(data, id_pos-data, network_search_term);
		if (tmp && tmp > last_id_pos)
		{
			tmp += strlen(network_search_term);
			tmp = g_strndup(tmp, strchr(tmp, '<')-tmp);
			stripped = purple_unescape_html(tmp);
			g_free(tmp);
			purple_debug_info("facebook", "With network: %s\n",
					stripped);
			row = g_list_prepend(row, stripped);
		} else {
			row = g_list_prepend(row, NULL);
		}

		if (purple_find_buddy(fba->account, id))
			row = g_list_prepend(row, g_strdup(_("Yes")));
		else
			row = g_list_prepend(row, g_strdup(_("No")));

		row = g_list_reverse(row);
		purple_notify_searchresults_row_add(results, row);

		last_id_pos = id_pos;
	}
	purple_debug_info("facebook", "dumping search results\n");
	purple_notify_searchresults(fba->pc, NULL, search_term, NULL,
			results, NULL, NULL);

	g_free(search_term);
}

static void fb_search_users_search_cb(gpointer connection,
		const gchar *search_text)
{
	PurpleConnection *pc = connection;
	FacebookAccount *fba = pc->proto_data;
	gchar *search_url;
	gchar *search_tmp;
	gchar *sid_cookie_value;

	if (!search_text || !*search_text)
		return;

	search_tmp = g_strdup(purple_url_encode(search_text));
	sid_cookie_value = g_hash_table_lookup(fba->cookie_table, "sid");
	if (sid_cookie_value == NULL)
		sid_cookie_value = "1";
	search_url = g_strdup_printf("/s.php?q=%s&init=q&sid=%s",
			search_tmp, sid_cookie_value);
	g_free(search_tmp);

	fb_post_or_get(fba, FB_METHOD_GET, NULL, search_url, NULL,
			fb_found_friends, g_strdup(search_text), FALSE);

	g_free(search_url);
}

void fb_search_users(PurplePluginAction *action)
{
	PurpleConnection *pc = (PurpleConnection *) action->context;

	purple_request_input(pc, _("Search for Friends"),
			_("Search for Facebook Friends"),
			_("Type the full name or e-mail address of the friend you are searching for."),
			NULL, FALSE, FALSE, NULL,
			_("_Search"), G_CALLBACK(fb_search_users_search_cb),
			_("_Cancel"), NULL,
			purple_connection_get_account(pc), NULL, NULL,
			pc);
}
