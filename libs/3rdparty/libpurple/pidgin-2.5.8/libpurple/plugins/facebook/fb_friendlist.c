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

#include "fb_friendlist.h"
#include "fb_connection.h"
#include "libfacebook.h"
#include "fb_util.h"

typedef struct _MoveRequest MoveRequest;
struct _MoveRequest {
	char *old_group;
	char *new_group;
	char *who;
};

/******************************************************************************/
/* Friend list modification methods */
/******************************************************************************/
static void handle_move_request(FacebookAccount *fba, MoveRequest *request)
{
	const gchar *old_list_id;
	const gchar *new_list_id;
	gchar *postdata;
	gboolean remove_flist, no_original_list;
	const gchar *command;

	g_return_if_fail(fba->post_form_id != NULL);		//VOXOX - JRT - 2009.08.05 - empty post_form_id causing crashes.

	purple_debug_info("facebook",
		"handling movement of %s from %s to %s\n",
		request->who, request->old_group, request->new_group);

	old_list_id = fb_get_list_id(fba, request->old_group);
	new_list_id = fb_get_list_id(fba, request->new_group);

	remove_flist = !new_list_id || g_str_equal(new_list_id, "-1");
	no_original_list = !old_list_id || g_str_equal(old_list_id, "-1");

	if (remove_flist) {
		command = "&remove_fl=true";
	} else if (no_original_list) {
		command = "&add_fl=true";
	} else {
		command = "&move_fl=true";
	}

	postdata = g_strdup_printf(
		"post_form_id=%s&drag_uid=%s&user=%" G_GINT64_FORMAT
		"&new_flid=%s&old_flid=%s%s",
		fba->post_form_id,
		request->who,
		fba->uid,
		remove_flist ? "" : new_list_id,
		no_original_list ? "" : old_list_id,
		command);

	fb_post_or_get(fba, FB_METHOD_POST, NULL,
		"/ajax/chat/buddy_list_settings.php",
		postdata, NULL, NULL, FALSE);

	g_free(postdata);

	g_free(request->who);
	g_free(request->old_group);
	g_free(request->new_group);
	g_free(request);
}

static void create_list_cb(FacebookAccount *fba, gchar *data,
	gsize data_len, gpointer userdata)
{
	// NOTE: this method can also be used for movements between
	// friend lists if necessary.

	JsonParser *parser;
	JsonObject *objnode;
	MoveRequest *request;

	// Parse out old data.
	parser = fb_get_parser(data, data_len);
	if (parser == NULL) {
		return;
	}

	objnode = fb_get_json_object(parser, NULL);
	if (!objnode ||
	    !json_object_has_member(objnode, "payload"))
	{
		json_parser_free(parser);
		return;
	}

	objnode = json_node_get_object(json_object_get_member(
			objnode, "payload"));
	fb_process_friend_lists(fba, objnode);

	json_parser_free(parser);

	// Move Friend
	request = (MoveRequest *) userdata;
	if (request) {
		handle_move_request(fba, request);
	}
}

static void create_friend_list(FacebookAccount *fba, const gchar *new_group,
	MoveRequest *request)
{
	gchar *postdata;
	gchar *new_group_escaped;

	g_return_if_fail(fba->post_form_id != NULL);		//VOXOX - JRT - 2009.08.05 - empty post_form_id causing crashes.

	purple_debug_info("facebook", "creating friend list %s\n", new_group);

	new_group_escaped = fb_strdup_withhtml(new_group);

	postdata = g_strdup_printf(
		"post_form_id=%s&create=%s&user=%" G_GINT64_FORMAT,
		fba->post_form_id,
		new_group_escaped,
		fba->uid);

	fb_post_or_get(fba, FB_METHOD_POST, NULL,
		"/ajax/chat/buddy_list_settings.php",
		postdata, create_list_cb, request, FALSE);

	g_free(postdata);
	g_free(new_group_escaped);
}

void fb_group_buddy_move(PurpleConnection *pc, const char *who,
	const char *old_group, const char *new_group)
{
	FacebookAccount *fba;
	MoveRequest *request;
	const gchar *new_list_id;
	
	if (!purple_account_get_bool(pc->account, "facebook_use_groups", TRUE))
	{
		//Dont do anything if we're ignoring groups
		return;
	}
	fba = pc->proto_data;

	purple_debug_info("facebook", "handling move of %s from %s to %s\n",
		who, old_group, new_group);

	// Don't do anything if groups are not actually changing.
	if (!purple_utf8_strcasecmp(old_group, new_group)) {
		purple_debug_info("facebook", "groups are same, not moving\n");
		return;
	}

	// Facebook doesn't support moving yourself because you can't be in a
	// friend list.  Let buddy list be updated as appropriate.
	if (atoll(who) == fba->uid) {
		purple_debug_info("facebook",
			"moving self, do not update server\n");
		return;
	}

	request = g_new0(MoveRequest, 1);
	request->old_group = g_utf8_strdown(old_group, -1);
	request->new_group = g_utf8_strdown(new_group, -1);
	request->who = g_strdup(who);

	new_list_id = fb_get_list_id(fba, request->new_group);
	if (new_list_id) {
		handle_move_request(fba, request);
	} else {
		create_friend_list(fba, new_group, request);
	}
}

void fb_buddy_remove(PurpleConnection *pc, PurpleBuddy *buddy,
		PurpleGroup *group)
{
	// This method should only remove a buddy from a friend list.
	// Nothing more.  It should not defriend a user ever.  See issue
	// #185 for a good explaination of why this is a bad idea.
	//
	// Moreover, defriending is such a rare operation that we should
	// never make it easy.  Facebook intentionally hides such a action
	// behind multiple layers of links and dialogs.
	//
	// If the plugin is ever to perform an actual defriending, it needs
	// to provide a dialog and user prompt at the absolute bare minimum.
	FacebookAccount *fba;
	if (!purple_account_get_bool(pc->account, "facebook_use_groups", TRUE))
	{
		//Dont do anything if we're ignoring groups
		return;
	}

	purple_debug_info("facebook", "handing removal of buddy %s\n",
		buddy->name);
       
	fba = pc->proto_data;

	fb_group_buddy_move(pc, buddy->name, purple_group_get_name(group),
			DEFAULT_GROUP_NAME);
}

void fb_group_rename(PurpleConnection *pc, const char *old_name,
	PurpleGroup *group, GList *moved_buddies)
{
	if (!purple_account_get_bool(pc->account, "facebook_use_groups", TRUE))
	{
		//Dont do anything if we're ignoring groups
		return;
	}
	purple_debug_info("facebook",
		"handling group rename of %s to %s\n",
		old_name, purple_group_get_name(group));
	
	// We don't do anything here.  Facebook's AJAX API for renaming groups
	// is horribly, horribly overcomplicated.  There is no simple rename
	// call, instead you must also pass in all the current data about the
	// friend list and port it over.  While it is possible to implement
	// this, it is risky and could potentially destroy a friend list if
	// the API changes.  That's a Bad Thing(tm).  Given the risk involved
	// with this operation and how rare it is, it's not worth it.
	// 
	// The problem is compounded by the fact that renaming groups triggers
	// all sorts of weird behaviors in Pidgin.  Renaming to a new name is
	// simple.  Renaming to an existing group name (hence a merge)
	// triggers completely different behavior with calls to group_buddy
	// before the call to rename.  This completely defeats the purpose of
	// having a rename function because group_buddy is called instead.
	//
	// Thus, the final decision is to use the buddy_move call.

	// TODO: warn users that renaming has no effect here.
}

void fb_group_remove(PurpleConnection *pc, PurpleGroup *group)
{
	if (!purple_account_get_bool(pc->account, "facebook_use_groups", TRUE))
	{
		//Dont do anything if we're ignoring groups
		return;
	}
	purple_debug_info("facebook", "got group removal of %s\n",
		purple_group_get_name(group));

	// We don't do anything here.  This is because a group rename also
	// fires a group removal event.  This assumes that the new group is
	// equivalent to the old group, but Facebook friend lists are much more
	// than simple groups- they are privacy control lists too.  There is
	// no easy way to port the settings between groups.  Better off not
	// deleting, and the user can do the cleanup with their browser.
}

/******************************************************************************/
/* Friend list fetch methods */
/******************************************************************************/

const gchar *fb_get_list_id(FacebookAccount *fba, const gchar *list_name)
{
	if (!purple_utf8_strcasecmp(list_name, DEFAULT_GROUP_NAME)) {
		return "-1";
	}

	return g_hash_table_lookup(fba->friend_lists_reverse, purple_normalize_nocase(NULL, list_name));
}

gboolean fb_process_friend_lists(FacebookAccount *fba,
		JsonObject *buddy_list)
{
	JsonObject *fl_obj;
	GList *friend_list_ids, *cur;

	purple_debug_info("facebook", "processing friend list data\n");

	if (!json_object_has_member(buddy_list, "flData"))
	{
		purple_debug_info("facebook", "no friend list data\n");
		return FALSE;
	}


	fl_obj = json_node_get_object(json_object_get_member(
			buddy_list, "flData"));
	friend_list_ids = json_object_get_members(fl_obj);
	for (cur = friend_list_ids; cur != NULL; cur = cur->next)
	{
		gchar *id;
		const gchar *name;
		JsonObject *data;

		id = (gchar *) cur->data;
		data = json_node_get_object(json_object_get_member(
				fl_obj, id));
		name = json_node_get_string(json_object_get_member(
				data, "n"));
		if (name) {
			// Either -1 isnt a valid JSON string or JSON-glib does
			// this wrong.  I'm too tired to tell the difference.
			if (g_str_equal(id, "_1")) {
				id = "-1";
			}
			purple_debug_info("facebook",
					"got friend list %s with id %s\n",
					name, id);
			g_hash_table_insert(fba->friend_lists,
					g_strdup(id), g_strdup(name));
			g_hash_table_insert(fba->friend_lists_reverse,
					g_utf8_strdown(name, -1), g_strdup(id));
		}
	}

	g_list_free(friend_list_ids);

	return TRUE;
}

static void destroy_buddy(gpointer key, gpointer value, gpointer data)
{
	PurpleBuddy *buddy;
	gchar *group_name;
	FacebookAccount *fba;

	buddy = (PurpleBuddy *) value;
	group_name = (gchar *) key;
	fba = (FacebookAccount *) data;

	purple_debug_info("facebook", "removing %s from group %s\n",
			buddy->name, group_name);
	if (atoll(buddy->name) == fba->uid) {
		purple_debug_info("facebook", "not removing self from %s\n",
			group_name);
		return;
	}

	purple_blist_remove_buddy(buddy);
}

static PurpleBuddy *add_buddy(FacebookAccount *fba,
	const gchar *friend_list_id, const gchar *uid, GHashTable *cur_groups)
{
	const gchar *group_name;
	PurpleGroup *fb_group;
	PurpleBuddy *buddy;

	group_name = g_hash_table_lookup(fba->friend_lists, purple_normalize_nocase(NULL, friend_list_id));
	if (!group_name || group_name[0] == '\0') {
		purple_debug_info("facebook",
				"did not find name of list %s\n",
				friend_list_id);
		group_name = DEFAULT_GROUP_NAME;
	}

	// Initialize group as necessary.
	fb_group = purple_find_group(group_name);
	if (fb_group == NULL)
	{
		purple_debug_info("facebook", "adding friend list %s\n",
				group_name);
		fb_group = purple_group_new(group_name);
		purple_blist_add_group(fb_group, NULL);
	} 

	buddy = (PurpleBuddy *)g_hash_table_lookup(cur_groups, purple_normalize_nocase(NULL, group_name));
	if (!buddy) {
		purple_debug_info("facebook", "adding %s to %s\n",
				uid, group_name);
		buddy = purple_buddy_new(fba->account, uid, NULL);
		purple_blist_add_buddy(buddy, NULL, fb_group, NULL);
		g_hash_table_remove(cur_groups, purple_normalize_nocase(NULL, group_name));
	}

	return buddy;
}


GList *fb_get_buddies_friend_list (FacebookAccount *fba,
		const gchar *uid, JsonArray *friend_list_ids)
{
	GSList *buddies;
	GSList *cur;       
	GHashTable *cur_groups;
	int i;
	GList *final_buddies, *cur_buddy;
	PurpleGroup *fb_group;
	PurpleBuddy *buddy;

	final_buddies = NULL;
	buddies = purple_find_buddies(fba->account, uid);

	// If we're already in the buddy list, stop.  Ignore FB info because
	// it will be incorrect.
	if (atoll(uid) == fba->uid && buddies != NULL) {
		purple_debug_info("facebook",
			"already have buddies for self, not adding\n");
		for (cur = buddies; cur != NULL; cur = cur->next)
		{
			final_buddies = g_list_append(
				final_buddies, cur->data);
		}
		g_slist_free(buddies);
		return final_buddies;
	}

	//Do we want to ignore groups?
	if (!purple_account_get_bool(fba->account, "facebook_use_groups", TRUE))
	{
		if (buddies != NULL) {
			//Copy the slist into the list
			for (cur = buddies; cur != NULL; cur = cur->next)
			{
				final_buddies = g_list_append(
					final_buddies, cur->data);
			}
			g_slist_free(buddies);
			return final_buddies;
		} else {
			buddy = purple_buddy_new(fba->account, uid, NULL);
			fb_group = purple_find_group(DEFAULT_GROUP_NAME);
			if (fb_group == NULL)
			{
				fb_group = purple_group_new(DEFAULT_GROUP_NAME);
				purple_blist_add_group(fb_group, NULL);
			}
			purple_blist_add_buddy(buddy, NULL, fb_group, NULL);
			final_buddies = g_list_append(final_buddies, buddy);
			return final_buddies;
		}
	}
	// Determine what buddies exist and what groups they are in.
	cur_groups = g_hash_table_new_full(g_str_hash, g_str_equal,
		g_free, NULL);
	for (cur = buddies; cur != NULL; cur = cur->next)
	{
		const gchar *group_name;

		group_name = purple_group_get_name(purple_buddy_get_group(
					(PurpleBuddy *)cur->data));

		g_hash_table_insert(cur_groups, g_utf8_strdown(group_name, -1), cur->data);
	}
	g_slist_free(buddies);

	// Create/insert necessary buddies
	if (friend_list_ids) {
		for (i = 0; i < json_array_get_length(friend_list_ids); i++)
		{
			const gchar *friend_list_id;


			friend_list_id = json_node_get_string(
				json_array_get_element(friend_list_ids, i));

			buddy = add_buddy(fba, friend_list_id, uid, cur_groups);

			final_buddies = g_list_append(final_buddies, buddy);
		}
	} else {
		// No friend list data, so we use the default group.
		final_buddies = g_list_append(final_buddies,
			add_buddy(fba, "-1", uid, cur_groups));
	}

	// Figure out which groups/buddies are not represented.
	for (cur_buddy = final_buddies; cur_buddy != NULL;
	     cur_buddy = cur_buddy->next)
	{
		g_hash_table_remove(cur_groups, purple_normalize_nocase(NULL, 
			purple_group_get_name(purple_buddy_get_group(
				(PurpleBuddy *)cur_buddy->data))));
	}

	// Delete remaining buddies to maintain sync state with server.
	g_hash_table_foreach(cur_groups, destroy_buddy, fba);

	// Cleanup!
	g_hash_table_destroy(cur_groups);

	return final_buddies;
}

void fb_friendlist_init(FacebookAccount *fba)
{
	/* data structure mapping friend list id to name.  libpurple only
	 * recognizes name, does not have group aliases */
	fba->friend_lists = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, g_free);
	/* structure mapping names to list id for speed. */
	fba->friend_lists_reverse = g_hash_table_new_full(g_str_hash,
			g_str_equal, g_free, g_free);
}

void fb_friendlist_destroy(FacebookAccount *fba)
{
	if (fba->friend_lists) {
		g_hash_table_destroy(fba->friend_lists);
	}
	if (fba->friend_lists_reverse) {
		g_hash_table_destroy(fba->friend_lists_reverse);
	}
}
