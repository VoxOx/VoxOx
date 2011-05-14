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

#include "fb_managefriends.h"
#include "fb_connection.h"

static void fb_auth_accept_cb(gpointer data)
{
	FacebookBuddy *fbuddy = data;
	FacebookAccount *fba = fbuddy->fba;
	gchar *buddy_uid;
	gchar *postdata;

	g_return_if_fail(fba != NULL);
	g_return_if_fail(fba->post_form_id != NULL);
	g_return_if_fail(fbuddy->uid != 0);

	buddy_uid = g_strdup_printf("%" G_GINT64_FORMAT, fbuddy->uid);

	postdata = g_strdup_printf(
			"type=friend_connect&id=%s&action=accept&"
			"params[verify_age]=0&params[lists]=&"
			"post_form_id=%s&fb_dtsg=%s&"
			"post_form_id_source=AsyncRequest&__a=1",
			buddy_uid, fba->post_form_id, fba->dtsg);
	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/profile/reqs.php",
			postdata, NULL, NULL, FALSE);

	g_hash_table_remove(fba->auth_buddies, buddy_uid);
	
	g_free(postdata);
	g_free(fbuddy);
	g_free(buddy_uid);
}

static void fb_auth_reject_cb(gpointer data)
{
	FacebookBuddy *fbuddy = data;
	FacebookAccount *fba = fbuddy->fba;
	gchar *buddy_uid;
	gchar *postdata;

	g_return_if_fail(fba != NULL);
	g_return_if_fail(fba->post_form_id != NULL);
	g_return_if_fail(fbuddy->uid != 0);

	buddy_uid = g_strdup_printf("%" G_GINT64_FORMAT, fbuddy->uid);

	postdata = g_strdup_printf(
			"type=friend_connect&id=%s&action=reject&"
			"params[verify_age]=0&params[lists]=&"
			"post_form_id=%s&fb_dtsg=%s&"
			"post_form_id_source=AsyncRequest&__a=1",
			buddy_uid, fba->post_form_id, fba->dtsg);
	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/profile/reqs.php",
			postdata, NULL, NULL, FALSE);

	g_hash_table_remove(fba->auth_buddies, buddy_uid);
	
	g_free(postdata);
	g_free(fbuddy);
	g_free(buddy_uid);
}

static void fb_check_friend_request_cb(FacebookAccount *fba, gchar *data,
		gsize data_len, gpointer user_data)
{
	const char *uid_pre_text = "class=\"confirm\" id=\"friend_connect_";
	const char *name_pre_text = "<td class=\"info\"><a ";
	const char *msg_pre_text = "<div class=\"personal_msg\"><span>";
	gchar *uid;
	gint32 uid_int;
	gchar *name;
	gchar *msg;
	gchar *msg_plain;
	FacebookBuddy *buddy;
	gchar *search_start = data;

	g_return_if_fail(data_len > 0);
	g_return_if_fail(data != NULL);

	/* loop through the data and look for confirm_friend_add_([0-9]*)" */
	while ((search_start = strstr(search_start, uid_pre_text)))
	{
		search_start += strlen(uid_pre_text);
		uid = g_strndup(search_start,
				strchr(search_start, '"') - search_start);
		purple_debug_info("facebook", "uid: %s\n", uid);

		uid_int = atoll(uid);

		if (g_hash_table_lookup_extended(fba->auth_buddies,
						uid, NULL, NULL))
		{
			/* we've already notified the user of this friend request */
			g_free(uid);
			continue;
		}

		name = strstr(search_start, name_pre_text);
		if (name != NULL)
		{
			name += strlen(name_pre_text);
			name = strchr(name, '>') + 1;
			name = g_strndup(name, strchr(name, '<') - name);
			purple_debug_info("facebook", "name: %s\n", name);
		} else {
			name = NULL;
		}

		msg = strstr(search_start, msg_pre_text);
		if (msg != NULL)
		{
			msg += strlen(msg_pre_text);
			msg = g_strndup(msg, strstr(msg, "</span></div>") - msg);
			msg_plain = purple_markup_strip_html(msg);
			g_free(msg);
			purple_debug_info("facebook", "msg: %s\n", msg_plain);
		} else {
			msg_plain = NULL;
		}

		buddy = g_new0(FacebookBuddy, 1);
		buddy->fba = fba;
		buddy->uid = uid_int;
		purple_account_request_authorization(
				fba->account, uid, NULL,
				name, msg_plain, TRUE,
				fb_auth_accept_cb, fb_auth_reject_cb, buddy);

		/* Don't display an auth request for this buddy again */
		g_hash_table_insert(fba->auth_buddies, uid, NULL);
		
		g_free(name);
		g_free(uid);
		g_free(msg_plain);		
	}
}

gboolean fb_check_friend_requests(gpointer data)
{
	FacebookAccount *fba;

	fba = data;

	if (purple_account_get_bool(
			fba->account, "facebook_manage_friends", FALSE)) {
		fb_post_or_get(fba, FB_METHOD_GET, NULL, "/reqs.php", NULL,
				fb_check_friend_request_cb, NULL, FALSE);
	}

	return TRUE;
}

void fb_add_buddy(PurpleConnection *pc, PurpleBuddy *buddy, PurpleGroup *group)
{
	gchar *postdata;
	FacebookAccount *fba = pc->proto_data;
	gchar *buddy_tmp;

	if (!purple_account_get_bool(
				fba->account, "facebook_manage_friends", FALSE)) {
		/*
		 * We used to pop up dialogs here but if a user renamed a group,
		 * this would spawn message for each person in the buddy list.  Bad!
		purple_notify_info(fba->pc, _("Friend not added"),
				_("Adding Facebook friends via Pidgin is disabled"),
				_("Either add a friend via Facebook.com or edit your account preferences"));
		*/
		purple_debug_warning("facebook", "attempted to add %s but was blocked\n", buddy->name);
		return;
	}

	if (atoll(buddy->name) == fba->uid)
	{
		purple_account_set_bool(fba->account,
				"facebook_hide_self", FALSE);
		return;
	}

	buddy_tmp = g_strdup(purple_url_encode(buddy->name));
	postdata = g_strdup_printf(
			"user=%" G_GINT64_FORMAT "&profile_id=%s&message=&"
			"source=&submit=1&post_form_id=%s&fb_dtsg=%s&"
			"post_form_id_source=AsyncRequest&__a=1",
			fba->uid, buddy_tmp, fba->post_form_id, fba->dtsg);
	g_free(buddy_tmp);

	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/profile/connect.php",
			postdata, NULL, NULL, FALSE);

	g_free(postdata);
}
void fb_buddy_delete(PurpleConnection *pc, PurpleBuddy *buddy,
		PurpleGroup *group)
{
	FacebookAccount *fba = pc->proto_data;
	gchar *buddy_tmp, *postdata;

	//This function removes a buddy from our friends list on facebook
	//and shouldn't really be used
	if (!purple_account_get_bool(fba->account, "facebook_manage_friends", FALSE)) {
		purple_debug_warning("facebook", "attempted to add %s but was blocked\n", buddy->name);
		return;
	}

	buddy_tmp = g_strdup(purple_url_encode(buddy->name));
	postdata = g_strdup_printf(
			"uid=%s&post_form_id=%s&fb_dtsg=%s&"
			"post_form_id_source=AsyncRequest&__a=1",
			buddy_tmp, fba->post_form_id, fba->dtsg);
	g_free(buddy_tmp);

	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/profile/removefriend.php",
			postdata, NULL, NULL, FALSE);

	g_free(postdata);
}
