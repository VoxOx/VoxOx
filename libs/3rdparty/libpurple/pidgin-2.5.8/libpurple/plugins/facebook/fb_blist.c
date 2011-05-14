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
#include "fb_blist.h"
#include "fb_util.h"
#include "fb_friendlist.h"
#include "blist.h"

static void set_buddies_offline(PurpleBuddy *buddy,
		GHashTable *online_buddies_list)
{
	if (PURPLE_BUDDY_IS_ONLINE(buddy) &&
		g_hash_table_lookup(online_buddies_list, buddy->name) == NULL)
	{
		purple_prpl_got_user_status(buddy->account, buddy->name,
				purple_primitive_get_id_from_type(
					PURPLE_STATUS_OFFLINE),
				NULL);
	}
}

static void buddy_icon_cb(FacebookAccount *fba, gchar *data, gsize data_len,
		gpointer user_data)
{
	gchar *buddyname;
	PurpleBuddy *buddy;
	FacebookBuddy *fbuddy;
	gpointer buddy_icon_data;

	buddyname = user_data;

	purple_debug_info("facebook",
			"buddy icon for buddy %s %" G_GSIZE_FORMAT "\n",
			buddyname, data_len);

	buddy = purple_find_buddy(fba->account, buddyname);
	g_free(buddyname);

	g_return_if_fail(buddy != NULL);

	fbuddy = buddy->proto_data;

	g_return_if_fail(fbuddy != NULL);

	buddy_icon_data = g_memdup(data, data_len);

	purple_buddy_icons_set_for_user(fba->account, buddy->name,
			buddy_icon_data, data_len, fbuddy->thumb_url);
}

/**
 * Find buddy names 
 */
static GList *get_buddies(FacebookAccount *fba, const gchar *uid,
		const gchar *name, JsonArray *friend_list_ids)
{
	GList *buddies;
	GList *cur;

	buddies = fb_get_buddies_friend_list(fba, uid, friend_list_ids);

	// Initialize proto data for each buddy.
	for (cur = buddies; cur != NULL; cur = cur->next)
	{
		PurpleBuddy *buddy;

		buddy = (PurpleBuddy *) cur->data;

		/* Set the FacebookBuddy structure */
		if (buddy->proto_data == NULL)
		{
			FacebookBuddy *fbuddy;
			gchar *buddy_icon_url;

			fbuddy = g_new0(FacebookBuddy, 1);
			fbuddy->buddy = buddy;
			fbuddy->fba = fba;
			fbuddy->uid = atoll(uid);
			fbuddy->name = g_strdup(name);

			// load the old buddy icon url from the icon 'checksum'
			buddy_icon_url = (char *)
				purple_buddy_icons_get_checksum_for_user(buddy);
			if (buddy_icon_url != NULL)
				fbuddy->thumb_url = g_strdup(buddy_icon_url);

			buddy->proto_data = fbuddy;
		}
	}

	return buddies;
}

static void process_buddy_icon(FacebookAccount *fba, PurpleBuddy *buddy,
	JsonObject *userInfo)
{
	FacebookBuddy *fbuddy;
	gchar *buddy_icon_url;
	gchar *icon_host;
	gchar *icon_path, *real_path;
	gchar *search_tmp;

	fbuddy = buddy->proto_data;
	
	/* Set the buddy icon (if it hasn't changed) */
	buddy_icon_url = g_strdup(json_node_get_string(json_object_get_member(
			userInfo, "thumbSrc")));
	/* Seperate the URL into pieces */
	purple_url_parse(buddy_icon_url, &icon_host, NULL, &icon_path, NULL, NULL);
	g_free(buddy_icon_url);
	
	if (icon_path != NULL && icon_path[0] != '/')
	{
		/* Slap a / at the front of that badboy */
		real_path = g_strconcat("/", icon_path, NULL);
		g_free(icon_path);
		icon_path = real_path;
	}
	if (fbuddy->thumb_url == NULL ||
	    !g_str_equal(fbuddy->thumb_url, icon_path))
	{
		g_free(fbuddy->thumb_url);
		if (g_str_equal(icon_path, "/pics/q_silhouette.gif"))
		{
			fbuddy->thumb_url = NULL;
			/* User has no icon */
			purple_buddy_icons_set_for_user(fba->account,
				purple_buddy_get_name(buddy), NULL, 0, NULL);
		}
		else
		{
			fbuddy->thumb_url = g_strdup(icon_path);

			/* small icon at /profile6/1845/74/q800753867_2878.jpg */
			/* bigger icon at /profile6/1845/74/n800753867_2878.jpg */
			search_tmp = strstr(icon_path, "/q");
			if (search_tmp)
				*(search_tmp + 1) = 'n';
				
			/* Fetch their icon */
			fb_post_or_get(fba, FB_METHOD_GET, icon_host,
					icon_path, NULL,
					buddy_icon_cb, g_strdup(purple_buddy_get_name(buddy)), FALSE);
		}
	}
	g_free(icon_host);
	g_free(icon_path);
}

static void process_buddies(FacebookAccount *fba, GHashTable *online_buddies_list,
	JsonObject *nowAvailableList, gchar *uid, JsonObject *userInfo)
{
	const gchar *name;
	gboolean idle;
	GList *buddies, *cur;
	gboolean current_buddy_online;

	JsonArray *friend_list_ids;

	friend_list_ids = NULL;
	name = json_node_get_string(json_object_get_member(userInfo, "name"));

	/* look for "uid":{"i":_____} */
	if (json_object_has_member(nowAvailableList, uid))
	{
		JsonObject *userBlistInfo;
		userBlistInfo = json_node_get_object(
			json_object_get_member(nowAvailableList, uid));
		idle = json_node_get_boolean(
			json_object_get_member(userBlistInfo, "i"));
		if (json_object_has_member(userBlistInfo, "fl")) {
			friend_list_ids = json_node_get_array(
				json_object_get_member(userBlistInfo, "fl"));
		}

		current_buddy_online = TRUE;
	} else {
		/* if we're here, the buddy's info has been sent, 
		 * but they're not actually online */
		current_buddy_online = FALSE;
		idle = FALSE;
	}

	/* is this us? */
	if (atoll(uid) == fba->uid)
	{
		purple_connection_set_display_name(fba->pc, name);

		/* check that we don't want to show ourselves */
		current_buddy_online = !purple_account_get_bool(
			fba->account, "facebook_hide_self", TRUE);
	}

	buddies = get_buddies(fba, uid, name, friend_list_ids);
	for (cur = buddies; cur != NULL; cur = cur->next)
	{
		PurpleBuddy *buddy;

		buddy = (PurpleBuddy *)cur->data;

		process_buddy_icon(fba, buddy, userInfo);

		purple_presence_set_idle(purple_buddy_get_presence(buddy),
				idle, 0);

		if (current_buddy_online)
		{
			/* Add buddy to the list of online buddies */
			g_hash_table_insert(online_buddies_list, buddy->name, buddy);

			// Set buddy as online in buddy list.  We check for several
			// conditions before doing this, because if we set it always
			// Pidgin has a bug where the logs go nuts with "x is online".
			if (!PURPLE_BUDDY_IS_ONLINE(buddy) ||
			    idle != purple_presence_is_idle(
				purple_buddy_get_presence(buddy)))
			{
				purple_prpl_got_user_status(fba->account, buddy->name,
					purple_primitive_get_id_from_type(
						idle ? PURPLE_STATUS_AWAY :
					       	PURPLE_STATUS_AVAILABLE), NULL);
			}
		}
	}

	/* update the blist if we have no previous alias */
	fb_blist_set_alias(fba, uid, name);
}

static void process_notifications(FacebookAccount *fba,
		JsonObject *notifications)
{
	JsonNode *inboxCount_node;//VOXOX - CJC - 2009.07.17 
	gchar *url;//VOXOX - CJC - 2009.07.17 
	if (notifications != NULL &&
	    purple_account_get_check_mail(fba->account))
	{
		inboxCount_node = json_object_get_member(
			notifications, "inboxCount");
		if (inboxCount_node) {
			gint inbox_count = json_node_get_int(inboxCount_node);
			if (inbox_count &&
			    inbox_count != fba->last_inbox_count) {
				fba->last_inbox_count = inbox_count;
				url = g_strdup("http://www.facebook.com/inbox/");
				purple_notify_emails(
					fba->pc, inbox_count,
					FALSE, NULL, NULL,
					(const char**) &(fba->account->username),
					(const char**) &(url), NULL, NULL);
				g_free(url);
			}
		}
	}
}
static void got_status_stream_cb(FacebookAccount *fba, gchar *data,
		gsize data_len, gpointer userdata)
{
	gchar *error = NULL;
	JsonParser *parser;
	JsonObject *objnode;
	gint new_latest;
	const gchar *html;
	gchar **messages;
	gchar *message;
	gint i;
	gchar *uid_string;
	gchar *message_string;
	gsize uid_length;
	FacebookBuddy *fbuddy;
	PurpleBuddy *buddy;
	
	purple_debug_info("facebook", "parsing status message stream\n");
	
	if (fba == NULL)
		return;

	parser = fb_get_parser(data, data_len);
	if (parser == NULL) {
		purple_debug_info("facebook", "could not parse\n");
		return;
	}
	
	//purple_debug_misc("facebook", "status message stream\n%s\n", data);
	
	objnode = fb_get_json_object(parser, &error);
	
	if (error || !json_object_has_member(objnode, "payload")) {
		purple_debug_info("facebook", "no payload\n");
		json_parser_free(parser);
		return;
	}
	
	objnode = json_node_get_object(json_object_get_member(
			objnode, "payload"));
	
	html = json_node_get_string(json_object_get_member(
			objnode, "html"));
	//purple_debug_misc("facebook", "html data\n%s\n", html);
	
	messages = g_strsplit(html, "/h3>", -1);
	for(i = 0; messages[i]; i++)
	{
		message = messages[i];
		uid_length = 0;
		
		//find uid:
		//start with aid_ ...  "
		uid_string = strstr(message, "aid_");
		if (!uid_string)
			continue;
		uid_string += 4;
		while (uid_string[uid_length] >= '0' &&
				uid_string[uid_length] <= '9')
		{
			uid_length++;
		}
		uid_string = g_strndup(uid_string, uid_length);
		purple_debug_info("facebook", "uid: %s\n", uid_string);
		
		//find message:
		// last index of
		// /a> ... <
		message_string = g_strrstr(message, "/a>");
		if (!message_string)
		{
			g_free(uid_string);
			continue;	
		}
		message_string = strchr(message_string, '>');
		if (!message_string)
		{
			g_free(uid_string);
			continue;
		}
		message_string += 1;
		message_string = g_strndup(message_string, g_strrstr(message_string, "<")-message_string);
		purple_debug_info("facebook", "message: %s\n", message_string);
		
		buddy = purple_find_buddy(fba->account, uid_string);
		if (buddy && buddy->proto_data)
		{
			fbuddy = buddy->proto_data;
			g_free(fbuddy->status);
			
			fbuddy->status = purple_markup_strip_html(message_string);
			
			purple_prpl_got_user_status(fba->account, buddy->name,
				purple_primitive_get_id_from_type(
					purple_presence_is_idle(purple_buddy_get_presence(buddy)) ? PURPLE_STATUS_AWAY :
						PURPLE_STATUS_AVAILABLE), "message", fbuddy->status, NULL);
		}
		
		g_free(uid_string);
		g_free(message_string);
	}
	g_strfreev(messages);
	
	new_latest = json_node_get_int(json_object_get_member(
			objnode, "newestStoryTime"));
	if (!new_latest)
	{
		purple_debug_info("facebook", "no newestStoryTime\n");
	} else {
		fba->last_status_timestamp = new_latest;
	}
	json_parser_free(parser);
}

static void got_buddy_list_cb(FacebookAccount *fba, gchar *data,
		gsize data_len, gpointer userdata)
{
	GSList *buddies_list;
	GList *userIds;//VOXOX - CJC - 2009.07.06 
	GHashTable *online_buddies_list = g_hash_table_new(g_str_hash, g_str_equal);//VOXOX - CJC - 2009.07.06 
	JsonObject *buddy_list;//VOXOX - CJC - 2009.07.06 
	GList *currentUserNode;//VOXOX - CJC - 2009.07.06 
	JsonObject *objnode;//VOXOX - CJC - 2009.07.06 
	// Iterate through the list of buddy infos sent to us.	
	JsonObject *userInfos;
	JsonObject *nowAvailableList;
	JsonParser *parser;//VOXOX - CJC - 2009.07.06 
	JsonObject *userInfo;//VOXOX - CJC - 2009.07.06 
	

	gchar *uid;//VOXOX - CJC - 2009.07.06 
	gchar *error;//VOXOX - CJC - 2009.07.06 

	purple_debug_info("facebook", "parsing buddy list\n");

	if (fba == NULL)
		return;

	parser = fb_get_parser(data, data_len);
	if (parser == NULL) {
		if (fba->bad_buddy_list_count++ == 3)
		{
		purple_connection_error_reason(fba->pc,
				PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
				_("Could not retrieve buddy list"));
		}
		return;
	}

	purple_debug_misc("facebook", "buddy list\n%s\n", data);

	error = NULL;
	objnode = fb_get_json_object(parser, &error);
	if (error) {
		if (fba->bad_buddy_list_count++ == 3)
		{
		purple_connection_error_reason(
				fba->pc,
				PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
				error);
		}
			json_parser_free(parser);
			return;
	}
	
	/* look for "userInfos":{ ... }, */
	if (!json_object_has_member(objnode, "payload"))
	{
		json_parser_free(parser);
		return;
	}
	objnode = json_node_get_object(json_object_get_member(
			objnode, "payload"));
	if (!json_object_has_member(objnode, "buddy_list"))
	{
		json_parser_free(parser);
		return;
	}
	buddy_list = json_node_get_object(json_object_get_member(objnode, "buddy_list"));
	if (!json_object_has_member(buddy_list, "userInfos"))
	{
		json_parser_free(parser);
		return;
	}

	//Reset invalid buddy list counter
	fba->bad_buddy_list_count = 0;
	if (purple_account_get_bool(fba->account, "facebook_use_groups", TRUE))
	{
		//Only process if we have the setting
		fb_process_friend_lists(fba, buddy_list);
	}
	userInfos = json_node_get_object(json_object_get_member(
			buddy_list, "userInfos"));
	nowAvailableList = json_node_get_object(json_object_get_member(
			buddy_list, "nowAvailableList"));
	
	userIds = json_object_get_members(userInfos);
	
	for(	currentUserNode = userIds;
		currentUserNode;
		currentUserNode = g_list_next(currentUserNode))
	{
		uid = currentUserNode->data;

		
		userInfo = json_node_get_object(json_object_get_member(
					userInfos, uid));
		// Process the user, which generally consists of updating
		// state info such as name, idle item, status message,etc.
		process_buddies(fba, online_buddies_list, nowAvailableList,
				uid, userInfo);
	}
	g_list_free(userIds);

	// Set users offline.  We do this in a seperate function because FB
	// only sends us a list of users who are online.  We find the users
	// that are not in the union of of buddy list users + online, and
	// mark them as offline.
	buddies_list = purple_find_buddies(fba->account, NULL);
	if (buddies_list != NULL)
	{
		g_slist_foreach(
			buddies_list,
			(GFunc)set_buddies_offline, online_buddies_list);
		g_slist_free(buddies_list);
	}
	g_hash_table_destroy(online_buddies_list);
	
	// The buddy list also contains notifications data.  Process and
	// display is appropriate.
	process_notifications(fba, json_node_get_object(
		json_object_get_member(objnode, "notifications")));

	json_parser_free(parser);
}

gboolean fb_get_buddy_list(gpointer data)
{
	FacebookAccount *fba;
	gchar *postdata;

	fba = data;

	postdata = g_strdup_printf(
			"user=%" G_GINT64_FORMAT "&popped_out=true&force_render=true&buddy_list=1&__a=1&post_form_id_source=AsyncRequest&post_form_id=%s&fb_dtsg=%s&notifications=1",
			fba->uid, fba->post_form_id?fba->post_form_id:"(null)", fba->dtsg?fba->dtsg:"(null)");
	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/presence/update.php",
			postdata, got_buddy_list_cb, NULL, FALSE);
	g_free(postdata);
	
	postdata = g_strdup_printf("/ajax/intent.php?filter=app_2915120374&request_type=1&__a=1&newest=%d&ignore_self=true",
			fba->last_status_timestamp);
	fb_post_or_get(fba, FB_METHOD_GET, NULL, postdata,
			NULL, got_status_stream_cb, NULL, FALSE);
	g_free(postdata);

	return TRUE;
}

void fb_blist_poke_buddy(PurpleBlistNode *node, gpointer data)
{
	PurpleBuddy *buddy;
	FacebookBuddy *fbuddy;
	FacebookAccount *fba;
	gchar *postdata;
	
	if(!PURPLE_BLIST_NODE_IS_BUDDY(node))
		return;
	buddy = (PurpleBuddy *) node;
	if (!buddy)
		return;
	fbuddy = buddy->proto_data;
	if (!fbuddy)
		return;
	fba = fbuddy->fba;
	if (!fba)
		return;
	
	postdata = g_strdup_printf("uid=%" G_GINT64_FORMAT "&pokeback=0&post_form_id=%s", fbuddy->uid, fba->post_form_id);
	
	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/poke.php",
				postdata, NULL, NULL, FALSE);
	
	g_free(postdata);
}

void fb_blist_set_alias(FacebookAccount *fba, const gchar *id,
		const gchar *name)
{
	const char *current_alias;
	PurpleBuddy *buddy;

	buddy = purple_find_buddy(fba->account, id);
	if (!buddy) {
		return;
	}	

	/* Set an alias if no user-defined alias is set yet.  This provides
	 * a basic name alias for each user which is more useful than a
	 * number.  A small corner case bug here- aliases will not change
	 * in accordance with people changing their names on Facebook.
	 */
	current_alias = purple_buddy_get_alias_only(buddy);
	if (!current_alias) {
		purple_debug_info("facebook", "aliasing %s to %s\n", id, name);
		purple_blist_alias_buddy(buddy, name);
	}

	/* In case user removes an alias, we have the server as fallback */
	serv_got_alias(fba->pc, id, name);
}

void fb_blist_init(FacebookAccount *fba)
{
	fb_friendlist_init(fba);

	fb_get_buddy_list(fba);

	/* periodically check for updates to your buddy list */
	fba->buddy_list_timer = purple_timeout_add_seconds(60,
			fb_get_buddy_list, fba);

}

void fb_blist_destroy(FacebookAccount *fba)
{
	if (fba->buddy_list_timer) {
		purple_timeout_remove(fba->buddy_list_timer);
	}

	fb_friendlist_destroy(fba);
}
