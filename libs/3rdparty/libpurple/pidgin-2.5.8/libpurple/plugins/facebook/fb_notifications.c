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

#include "fb_notifications.h"
#include "fb_connection.h"

static void fb_got_notifications_cb(FacebookAccount *fba, gchar *url_text, gsize len, gpointer userdata)
{
	gchar *salvaged;
	time_t last_fetch_time;
	time_t time_of_message;
	time_t newest_message = 0;
	xmlnode *channel;//VOXOX - CJC - 2009.07.06 
	xmlnode *rss_root;//VOXOX - CJC - 2009.07.06 
	xmlnode *item;//VOXOX - CJC - 2009.07.06 
	xmlnode *link;//VOXOX - CJC - 2009.07.06 
	xmlnode *title;//VOXOX - CJC - 2009.07.06 
	gchar *tmp;
	gchar month_string[4], weekday[4];
	guint year, month, day, hour, minute, second;
	long timezone;
	gchar *subject, *url;

	month_string[3] = weekday[3] = '\0';
	year = month = day = hour = minute = second = 0;

	if (!url_text || !len)
		return;

	last_fetch_time = purple_account_get_int(fba->account, "facebook_notifications_last_fetch", 0);
	/* purple_debug_info("facebook", "last fetch time: %zu\n", last_fetch_time); */

	salvaged = purple_utf8_salvage(url_text);
	rss_root = xmlnode_from_str(salvaged, -1);
	g_free(salvaged);

	if (rss_root == NULL)
	{
		purple_debug_error("facebook", "Could not load RSS file\n");
		return;
	}
	channel = xmlnode_get_child(rss_root, "channel");
	if (channel == NULL)
	{
		purple_debug_warning("facebook", "Invalid RSS feed\n");
		xmlnode_free(rss_root);
		return;
	}
	item = xmlnode_get_child(channel, "item");
	if (item == NULL)
	{
		purple_debug_info("facebook", "No new notifications\n");
	}
	for (; item != NULL; item = xmlnode_get_next_twin(item))
	{
		xmlnode *pubDate = xmlnode_get_child(item, "pubDate");
		if (!pubDate)
			continue;
		tmp = xmlnode_get_data_unescaped(pubDate);
		/* rss times are in Thu, 19 Jun 2008 15:51:25 -1100 format */
		sscanf(tmp, "%3s, %2u %3s %4u %2u:%2u:%2u %5ld", (char*)&weekday, &day, (char*)&month_string, &year, &hour, &minute, &second, &timezone);
		if (g_str_equal(month_string, "Jan")) month = 0;
		else if (g_str_equal(month_string, "Feb")) month = 1;
		else if (g_str_equal(month_string, "Mar")) month = 2;
		else if (g_str_equal(month_string, "Apr")) month = 3;
		else if (g_str_equal(month_string, "May")) month = 4;
		else if (g_str_equal(month_string, "Jun")) month = 5;
		else if (g_str_equal(month_string, "Jul")) month = 6;
		else if (g_str_equal(month_string, "Aug")) month = 7;
		else if (g_str_equal(month_string, "Sep")) month = 8;
		else if (g_str_equal(month_string, "Oct")) month = 9;
		else if (g_str_equal(month_string, "Nov")) month = 10;
		else if (g_str_equal(month_string, "Dec")) month = 11;
		g_free(tmp);

		/* try using pidgin's functions */
		tmp = g_strdup_printf("%04u%02u%02uT%02u%02u%02u%05ld", year, month, day, hour, minute, second, timezone);
		time_of_message = purple_str_to_time(tmp, FALSE, NULL, NULL, NULL);
		g_free(tmp);

		if (time_of_message <= 0)
		{
			/* there's no cross-platform, portable way of converting string to time
			   which doesn't need a new version of glib, so just cheat */
			time_of_message = second + 60*minute + 3600*hour + 86400*day + 2592000*month + 31536000*(year-1970);
		}

		if (time_of_message > newest_message)
		{
			/* we'll keep the newest message to save */
			newest_message = time_of_message;
		}

		if (time_of_message <= last_fetch_time)
		{
			/* fortunatly, rss messages are ordered from newest to oldest */
			/* so if this message is older than the last one, ignore rest */
			break;
		}
		
		link = xmlnode_get_child(item, "link");
		if (link)
		{
			url = xmlnode_get_data_unescaped(link);
		} else {
			url = g_strdup("");
		}
		
		title = xmlnode_get_child(item, "title");
		if (title)
		{
			subject = xmlnode_get_data_unescaped(title);
		} else {
			subject = g_strdup("");
		}
		
		purple_notify_email(fba->pc, subject, NULL, fba->account->username, url, NULL, NULL);
		g_free(subject);
		g_free(url);
	}
	xmlnode_free(rss_root);

	if (newest_message > last_fetch_time)
	{
		/* update the last fetched time if we had newer messages */
		purple_account_set_int(fba->account, "facebook_notifications_last_fetch", newest_message);
	}
}

static void find_feed_url_cb(FacebookAccount *fba, gchar *data, gsize data_len, gpointer userdata)
{
	const gchar *search_string = "/feeds/notifications.php";
	gchar *feed_url;
	gchar *stripped;

	purple_debug_info("facebook", "find_feed_url_cb\n");

	if (!data)
		data = "(null)";

	feed_url = g_strstr_len(data, data_len, search_string);
	if (!feed_url)
	{
		purple_debug_error("facebook", "received data, but could not find url on page\n");
		return;
	}

	feed_url = g_strndup(feed_url, strchr(feed_url, '"') - feed_url);

	/* convert &amp; to & */
	stripped = purple_unescape_html(feed_url);
	g_free(feed_url);
	feed_url = stripped;
	
	purple_debug_info("facebook", "parsed feed url %s\n", feed_url);

	if (feed_url && *feed_url)
	{
		purple_account_set_string(fba->account, "notifications_feed_url", feed_url);
		fb_get_notifications_feed(fba);
	}
}

static void fb_find_feed_url(FacebookAccount *fba)
{
	fb_post_or_get(fba, FB_METHOD_GET, NULL, "/notifications.php", NULL,
			find_feed_url_cb, NULL, FALSE);
}

gboolean fb_get_notifications_feed(FacebookAccount *fba)
{
	const gchar *feed_url;

	if (purple_account_get_bool(fba->account, "facebook_get_notifications", TRUE)) {
		feed_url = purple_account_get_string(fba->account, "notifications_feed_url", NULL);
		if (!feed_url)
		{
			purple_debug_info("facebook", "no notifications feed url available, searching for it\n");
			fb_find_feed_url(fba);
			return TRUE;
		}

		fb_post_or_get(fba, FB_METHOD_GET, NULL, feed_url, NULL, fb_got_notifications_cb, NULL, FALSE);
	}

	return TRUE;
}
