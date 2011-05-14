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
#include "fb_blist.h"
#include "fb_connection.h"
#include "fb_conversation.h"
#include "fb_info.h"
#include "fb_managefriends.h"
#include "fb_messages.h"
#include "fb_notifications.h"
#include "fb_search.h"
#include "fb_friendlist.h"

static void fb_login_cb(FacebookAccount *fba, gchar *response, gsize len,
		gpointer userdata);
static void fb_close(PurpleConnection *pc);
static void fb_buddy_free(PurpleBuddy *buddy);

/******************************************************************************/
/* PRPL functions */
/******************************************************************************/

static const char *fb_list_icon(PurpleAccount *account, PurpleBuddy *buddy)
{
	return "facebook";
}

static gchar *fb_status_text(PurpleBuddy *buddy)
{
	FacebookBuddy *fbuddy = buddy->proto_data;

	if (fbuddy && fbuddy->status && *fbuddy->status != '\0')
		return g_strdup(fbuddy->status);

	return NULL;
}

static void fb_tooltip_text(PurpleBuddy *buddy,
		PurpleNotifyUserInfo *userinfo, gboolean full)
{
	FacebookBuddy *fbuddy = buddy->proto_data;
	gchar *status;

	g_return_if_fail(fbuddy);

	if (fbuddy->status && *fbuddy->status != '\0')
	{
		status = g_strdup_printf("%s %s",
				fbuddy->name, fbuddy->status);

		purple_notify_user_info_add_pair(userinfo,
				_("Status"), status);
		g_free(status);
	}
}

static GList *fb_statuses(PurpleAccount *account)
{
	GList *types = NULL;
	PurpleStatusType *status;

	/* Online people have a status message and also a date when it was set */
	status = purple_status_type_new_with_attrs(PURPLE_STATUS_AVAILABLE,
		NULL, _("Online"), FALSE, TRUE, FALSE, "message",
		_("Message"), purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);
	
	/* Cave into feature requests and allow people to set themselves to be idle */
	status = purple_status_type_new_with_attrs(PURPLE_STATUS_AWAY,
		NULL, _("Idle"), FALSE, TRUE, FALSE, "message",
		_("Message"), purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);

	/* Offline people dont have messages */
	status = purple_status_type_new_full(PURPLE_STATUS_OFFLINE, NULL, _("Offline"), FALSE, TRUE, FALSE);
	types = g_list_append(types, status);

	return types;
}

static gboolean fb_get_messages_failsafe(FacebookAccount *fba)
{
	if (fba->last_messages_download_time < (time(NULL) - (60*5))) {
		/* Messages haven't been downloaded in a while-
		 * something is probably wrong */
		purple_debug_warning("facebook",
				"executing message check failsafe\n");
		fb_get_post_form_id(fba);
	}

	return TRUE;
}

void fb_login_captcha_cancel_cb(PurpleConnection *pc, PurpleRequestFields *fields)
{
	purple_connection_error_reason(pc, PURPLE_CONNECTION_ERROR_OTHER_ERROR,
		"Could not authenticate captcha.  Logging into the Facebook website may fix this.");
}
void fb_login_captcha_ok_cb(PurpleConnection *pc, PurpleRequestFields *fields)
{
	const gchar *captcha_response;
	gchar *postdata, *encoded_username, *encoded_password, *encoded_charset_test,
			*encoded_persist_data, *encoded_response, *encoded_extra_challenge,
			*encoded_session;
	FacebookAccount *fba = pc->proto_data;

	captcha_response = purple_request_fields_get_string(fields, "captcha_response");
	
	encoded_response = g_strdup(purple_url_encode(captcha_response));
	encoded_username = g_strdup(purple_url_encode(
			purple_account_get_username(fba->account)));
	encoded_password = g_strdup(purple_url_encode(
			purple_account_get_password(fba->account)));
	encoded_extra_challenge = g_strdup(purple_url_encode(
			fba->extra_challenge));
	encoded_persist_data = g_strdup(purple_url_encode(
			fba->persist_data));
	encoded_session = g_strdup(purple_url_encode(
			fba->captcha_session));
	encoded_charset_test = g_strdup(purple_url_encode("€,´,€,´,水,Д,Є"));

	postdata = g_strdup_printf("charset_test=%s&"
								"version=1.0&"
								"return_session=0&"
								"charset_test=%s&"
								"answered_captcha=1&"
								"captcha_persist_data=%s&"
								"captcha_session=%s&"
								"extra_challenge_params=%s&"
								"captcha_response=%s&"
								"email=%s&pass=%s&"
								"persistent=1",
								encoded_charset_test, encoded_charset_test,
								encoded_persist_data, encoded_session,
								encoded_extra_challenge, encoded_response,
								encoded_username, encoded_password);
	g_free(encoded_username);
	g_free(encoded_password);
	g_free(encoded_charset_test);
	g_free(encoded_extra_challenge);
	g_free(encoded_persist_data);
	g_free(encoded_response);
	g_free(encoded_session);

	fb_post_or_get(fba, FB_METHOD_POST | FB_METHOD_SSL, "login.facebook.com",
			"/login.php?login_attempt=1&_fb_noscript=1", postdata, fb_login_cb, NULL, FALSE);
	g_free(postdata);
	
	g_free(fba->extra_challenge);
	g_free(fba->persist_data);
	g_free(fba->captcha_session);
	fba->extra_challenge = NULL;
	fba->persist_data = NULL;
	fba->captcha_session = NULL;
}

static void fb_login_captcha_image_cb(FacebookAccount *fba, gchar *response, 
		gsize len, gpointer userdata)
{
	PurpleRequestFields *fields;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;
	
	fields = purple_request_fields_new();
	group = purple_request_field_group_new(NULL);
	purple_request_fields_add_group(fields, group);
	
	field = purple_request_field_image_new("captcha_image", "", response, len);
	purple_request_field_group_add_field(group, field);
	
	field = purple_request_field_string_new("captcha_response", "", "", FALSE);
	purple_request_field_group_add_field(group, field);
	
	purple_request_fields(fba->pc, 
		_("Facebook Captcha"), _("Facebook Captcha"), 
		_("Enter both words below, separated by a space"), 
		fields, 
		_("OK"), G_CALLBACK(fb_login_captcha_ok_cb), 
		_("Logout"), G_CALLBACK(fb_login_captcha_cancel_cb), 
		fba->account, NULL, NULL, fba->pc	 
	);
}
static void fb_login_captcha_cb(FacebookAccount *fba, gchar *response, 
		gsize len, gpointer userdata)
{
	const gchar *challenge_start = "challenge : '";
	gchar *challenge;
	gchar *image_url;
	
	challenge = g_strstr_len(response, len, challenge_start);
	if (challenge)
	{
		challenge += strlen(challenge_start);
		challenge = g_strndup(challenge, strchr(challenge, '\'') - challenge);
		
		image_url = g_strdup_printf("/image?c=%s", challenge);
		
		fb_post_or_get(fba, FB_METHOD_GET | FB_METHOD_SSL, "api-secure.recaptcha.net",
			image_url, NULL, fb_login_captcha_image_cb, NULL, FALSE);
	}
}
static void fb_login_cb(FacebookAccount *fba, gchar *response, gsize len,
		gpointer userdata)
{
	gchar *user_cookie;
	const gchar *persist_data_start;
	gchar *persist_data;
	const gchar *session_start;
	gchar *session;
	gchar *captcha_url;
	const gchar *extra_challenge_params;
	gchar *extra_challenge;
	if (len && g_strstr_len(response, len, "captcha"))
	{
		purple_debug_info("facebook", "captcha page: %s\n", response);

		purple_connection_update_progress(fba->pc, _("Handling Captcha"), 2, 4);
		
		persist_data_start = "<input type=\"hidden\" id=\"captcha_persist_data\" name=\"captcha_persist_data\" value=\"";
		persist_data = g_strstr_len(response, len, persist_data_start);
		if (persist_data)
		{
			persist_data += strlen(persist_data_start);
			fba->persist_data = g_strndup(persist_data, strchr(persist_data, '"') - persist_data);
		}
		
		session_start = "<input type=\"hidden\" id=\"captcha_session\" name=\"captcha_session\" value=\"";
		session = g_strstr_len(response, len, session_start);
		if (session)
		{
			session += strlen(session_start);
			fba->captcha_session = g_strndup(session, strchr(session, '"') - session);
		}
		
		
		extra_challenge_params = "<input type=\"hidden\" id=\"extra_challenge_params\" name=\"extra_challenge_params\" value=\"";
		extra_challenge = g_strstr_len(response, len, extra_challenge_params);
		if (extra_challenge)
		{
			extra_challenge += strlen(extra_challenge_params);
			fba->extra_challenge = g_strndup(extra_challenge, strchr(extra_challenge, '"') - extra_challenge);
			extra_challenge = purple_unescape_html(fba->extra_challenge);
			g_free(fba->extra_challenge);
			fba->extra_challenge = extra_challenge;
		}
		
		if (!fba->extra_challenge || !fba->persist_data || !fba->captcha_session)
		{
			purple_debug_info("facebook", "captcha response: %s\n", response);
			g_free(fba->extra_challenge);
			g_free(fba->persist_data);
			g_free(fba->captcha_session);
			fba->extra_challenge = NULL;
			fba->persist_data = NULL;
			fba->captcha_session = NULL;
			purple_connection_error_reason(fba->pc, PURPLE_CONNECTION_ERROR_OTHER_ERROR,
				"Could not authenticate captcha.  Logging into the Facebook website may fix this.");
			return;
		}
		
		captcha_url = g_strdup_printf("/challenge?k=" FACEBOOK_CAPTCHA_SITE "&%s",
				fba->extra_challenge?fba->extra_challenge:"");
		
		fb_post_or_get(fba, FB_METHOD_GET | FB_METHOD_SSL, "api-secure.recaptcha.net",
			captcha_url, NULL, fb_login_captcha_cb, NULL, FALSE);
		
		g_free(captcha_url);
		
		return;
	}

	purple_connection_update_progress(fba->pc, _("Authenticating"), 2, 3);

	/* Look for our uid */
	user_cookie = g_hash_table_lookup(fba->cookie_table, "c_user");
	if (user_cookie == NULL) {
		/*
		 * Server didn't set the c_user cookie, so we must have given
		 * them a bad username or password
		 */
		purple_connection_error_reason(fba->pc,
				PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
				_("Incorrect username or password."));
		return;
	}
	fba->uid = atoll(user_cookie);
	purple_debug_info("facebook", "uid %" G_GINT64_FORMAT "\n", fba->uid);

	/* ok, we're logged in now! */
	purple_connection_set_state(fba->pc, PURPLE_CONNECTED);

	/* This will kick off our long-poll message retrieval loop */
	fb_get_post_form_id(fba);
	fb_check_friend_requests(fba);

	/* periodically check for people adding you to their facebook friend list */
	fba->friend_request_timer = purple_timeout_add_seconds(60 * 5,
			fb_check_friend_requests, fba);

	/* periodically check for new notifications */
	fba->notifications_timer = purple_timeout_add_seconds(60,
			(GSourceFunc)fb_get_notifications_feed, fba);

	/* Periodically check for new messages.  NOTE: This MUST exist,
	 * regardless of other other mechanisms for checking messages.  This
	 * is because the code needs a failsafe checker in case other one of
	 * the other retrieval mechanisms dies due to a bad request, etc.
	 * Without such a failsafe, a user will receive no messages, which is
	 * one of hardest bugs to debug and get reports about.  Hence, the
	 * importance of this loop.
	 * That said, there is room for tweaking this loop and possibly even
	 * setting it such that it is the primary or only message checker.
	 * The key is that the method must NEVER die until logout.
	 */
	fba->perpetual_messages_timer = purple_timeout_add_seconds(15,
			(GSourceFunc)fb_get_messages_failsafe, fba);

	/* init blist subsystem */
	fb_blist_init(fba);

	/* init conversation subsystem */
	fb_conversation_init(fba);
}

static void fb_login(PurpleAccount *account)
{
	FacebookAccount *fba;
	gchar *postdata, *encoded_username, *encoded_password, *encoded_charset_test;
	const gchar* const *languages;
	const gchar *locale;
	
	/* Create account and initialize state */
	fba = g_new0(FacebookAccount, 1);
	fba->account = account;
	fba->pc = purple_account_get_connection(account);
	fba->uid = -1;
	fba->last_messages_download_time = time(NULL) - 60; /* 60 secs is a safe buffer */
	fba->cookie_table = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, g_free);
	fba->hostname_ip_cache = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, g_free);
	fba->sent_messages_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, NULL);
	fba->auth_buddies = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, NULL);

	g_hash_table_replace(fba->cookie_table, g_strdup("test_cookie"),
			g_strdup("1"));

	account->gc->proto_data = fba;

	/* Error localized in libpurple jabber.c */
	if (!purple_ssl_is_supported()) {
		purple_connection_error_reason (purple_account_get_connection(account),
				PURPLE_CONNECTION_ERROR_NO_SSL_SUPPORT,
				_("Server requires TLS/SSL for login.  No TLS/SSL support found."));
		return;
	}

	purple_connection_set_state(fba->pc, PURPLE_CONNECTING);
	purple_connection_update_progress(fba->pc, _("Connecting"), 1, 3);

	encoded_username = g_strdup(purple_url_encode(
			purple_account_get_username(fba->account)));
	encoded_password = g_strdup(purple_url_encode(
			purple_account_get_password(fba->account)));
	encoded_charset_test = g_strdup(purple_url_encode("€,´,€,´,水,Д,Є"));
	languages = g_get_language_names();
	locale = languages[0];
	if (locale == NULL || g_str_equal(locale, "C"))
		locale = "en_US";

	g_hash_table_replace(fba->cookie_table, g_strdup("lsd"), g_strdup("abcde"));
	postdata = g_strdup_printf(
			"charset_test=%s&locale=%s&email=%s&pass=%s&pass_placeHolder=Password&persistent=1&login=Login&charset_test=%s&lsd=abcde",
			encoded_charset_test, locale, encoded_username, encoded_password, encoded_charset_test);
	g_free(encoded_username);
	g_free(encoded_password);
	g_free(encoded_charset_test);

	fb_post_or_get(fba, FB_METHOD_POST | FB_METHOD_SSL, "login.facebook.com",
			"/login.php?login_attempt=1&_fb_noscript=1", postdata, fb_login_cb, NULL, FALSE);
	g_free(postdata);
}

static void fb_close(PurpleConnection *pc)
{
	FacebookAccount *fba;
	gchar *postdata;
	GSList *buddies;

	purple_debug_info("facebook", "disconnecting account\n");

	g_return_if_fail(pc != NULL);
	g_return_if_fail(pc->proto_data != NULL);
	
	fba = pc->proto_data;

	if ( fba == NULL )		//VOXOX - JRT - 2009.10.12 - Prevent crash when network is lost or resuming from hibernation.
		return;

	purple_debug_info("facebook", "unloading plugin\n");

	/* destroy blist subsystem */
	fb_blist_destroy(fba);

	/* destroy conversation subsystem */
	fb_conversation_destroy(fba);
	buddies = purple_find_buddies(fba->account, NULL);
	while(buddies) {
		PurpleBuddy *b = buddies->data;
		fb_buddy_free(b);
		buddies = g_slist_delete_link(buddies, buddies);
	}

	/* Tell Facebook that we've logged out. */
	/*
	 * TODO
	 * This doesn't actually work because the request is non-blocking
	 * and we're in the process of logging out.  So we start making a
	 * connection but then libpurple immediately cancels the attempt
	 * and frees everything.
	 *
	 * There are two ways to fix this:
	 * 1. We could make this request, but not pass in fba or reference
	 *    any other data.  The request could complete normally even
	 *    after this account has logged out, since it really doesn't
	 *    need access to the PurpleConnection or the FacebookAccount.
	 *
	 * 2. The close prpl callback could be changed in libpurple so that
	 *    protocol plugins can have a chance to make network requests
	 *    and do other long cleanup operations.  So the call to
	 *    prpl->close() would become asynchronous.  It tells the
	 *    protocol plugin to begin the shutdown sequence, and the
	 *    protocol plugin tells the core when it's finished.
	 */
	if (fba->post_form_id)
		postdata = g_strdup_printf(
				"visibility=false&post_form_id=%s&"
				"fb_dtsg=%s&post_form_id_source=AsyncRequest&"
				"__a=1",
				fba->post_form_id, fba->dtsg);
	else
		postdata = g_strdup("visibility=false");
	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/chat/settings.php",
			postdata, NULL, NULL, FALSE);
	g_free(postdata);

	if (fba->friend_request_timer) {
		purple_timeout_remove(fba->friend_request_timer);
	}
	if (fba->notifications_timer) {
		purple_timeout_remove(fba->notifications_timer);
	}
	if (fba->new_messages_check_timer) {
		purple_timeout_remove(fba->new_messages_check_timer);
	}
	if (fba->perpetual_messages_timer) {
		purple_timeout_remove(fba->perpetual_messages_timer);
	}

	purple_debug_info("facebook", "destroying %d incomplete connections\n",
			g_slist_length(fba->conns));

	while (fba->conns != NULL)
		fb_connection_destroy(fba->conns->data);

	while (fba->dns_queries != NULL) {
		PurpleDnsQueryData *dns_query = fba->dns_queries->data;
		purple_debug_info("facebook", "canceling dns query for %s\n",
					purple_dnsquery_get_host(dns_query));
		fba->dns_queries = g_slist_remove(fba->dns_queries, dns_query);
		purple_dnsquery_destroy(dns_query);
	}
	
	if (fba->resending_messages != NULL) {
		fb_cancel_resending_messages(fba);
	}

	//VOXOX - JRT - 2009.10.13 - I know we are deleting fba anyway, but since other threads may also be accessing this,
	//							 it helps reduce crashes due to freed memory ptrs.  (There is no thread syncing to control this better).
	g_hash_table_destroy(fba->cookie_table);

	g_hash_table_destroy(fba->hostname_ip_cache);

	g_hash_table_destroy(fba->auth_buddies);

	g_free(fba->post_form_id);
	g_free(fba->dtsg);
	g_free(fba->channel_number);
	g_free(fba->last_status_message);
	g_free(fba->extra_challenge);
	g_free(fba->captcha_session);
	g_free(fba->persist_data);
	g_free(fba);

}

static unsigned int fb_send_typing(PurpleConnection *pc, const gchar *name,
		PurpleTypingState state)
{
	int typing_state;
	gchar *postdata;
	FacebookAccount *fba = pc->proto_data;
	gchar *encoded_name;

	g_return_val_if_fail(fba != NULL, 0);
	g_return_val_if_fail(fba->post_form_id != NULL, 0);

	typing_state = (state == PURPLE_TYPING) ? 1 : 0;

	/* Don't send typing notifications to self */
	if (atoll(name) != fba->uid)
	{
		encoded_name = g_strdup(purple_url_encode(name));
		postdata = g_strdup_printf("typ=%d&to=%s&post_form_id=%s",
				typing_state, encoded_name, fba->post_form_id);
		g_free(encoded_name);
		fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/chat/typ.php",
				postdata, NULL, NULL, FALSE);
		g_free(postdata);
	} else {
		serv_got_typing(pc, name, 10, state);
	}

	/* 7 is the number of seconds before sending the new typing state.  It
	 * corresponds with the default value that Facebook waits. */
	return 7;
}

static void fb_set_status_ok_cb(gpointer data, const gchar *status_text)
{
	PurpleConnection *pc = data;
	FacebookAccount *fba = pc->proto_data;
	gchar *postdata;
	gchar *status_tmp;
	gchar *status_text_new;

	g_return_if_fail(fba->post_form_id != NULL);

	status_text_new = g_strstrip(g_strdup(status_text));

	/* don't set the status if it's idential to what we've already set */
	if (fba->last_status_message && g_str_equal(fba->last_status_message,
				status_text_new)) {
		g_free(status_text_new);
		return;
	}

	g_free(fba->last_status_message);
	fba->last_status_message = status_text_new;

	if (*status_text_new != '\0')
	{
		status_tmp = g_strdup(purple_url_encode(status_text_new));
		postdata = g_strdup_printf("profile_id=%" G_GINT64_FORMAT "&status=%s&post_form_id=%s",
				fba->uid, status_tmp, fba->post_form_id);
		g_free(status_tmp);
	}
	else
		postdata = g_strdup_printf("profile_id=%" G_GINT64_FORMAT "&clear=1&post_form_id=%s",
				fba->uid, fba->post_form_id);

	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/updatestatus.php",
			postdata, NULL, NULL, FALSE);

	g_free(postdata);
}

static void fb_set_status_p(PurpleAccount *account, PurpleStatus *status)
{
	const gchar *message;
	gchar *stripped;
	FacebookAccount *fba = account->gc->proto_data;

	/* if "away" set idle */
	if (fba && purple_status_type_get_primitive(purple_status_get_type(status)) == PURPLE_STATUS_AWAY)
	{
		fba->is_idle = TRUE;	
	}
	//VOXOX - CJC - 2009.07.06 We always want to do this
	///* first check that we actually want to set this through Pidgin */
	//if (!purple_account_get_bool(account,
	//			"facebook_set_status_through_pidgin", FALSE))
	//{
	//	return;
	//}

	

	//message = purple_status_get_attr_string(status, "message");
	//if (message == NULL)
	//	message = "";
	message = purple_status_get_attr_string(status, "message");
	if (message == NULL)
		//VOXOX CHANGE CJC DO NOT SET MESSAGE IF BLANK
		return;

	stripped = purple_markup_strip_html(message);
	fb_set_status_ok_cb(account->gc, stripped);
	g_free(stripped);
}

static void fb_buddy_free(PurpleBuddy *buddy)
{
	FacebookBuddy *fbuddy = buddy->proto_data;
	if (fbuddy != NULL)
	{
		buddy->proto_data = NULL;

		g_free(fbuddy->name);
		g_free(fbuddy->status);
		g_free(fbuddy->thumb_url);
		g_free(fbuddy);
	}
}

#if PURPLE_MAJOR_VERSION >= 2 && PURPLE_MINOR_VERSION >= 5
static GHashTable *fb_get_account_text_table(PurpleAccount *account)
{
	GHashTable *table;

	table = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(table, "login_label", (gpointer)_("Email Address..."));

	return table;
}
#endif

/******************************************************************************/
/* Plugin functions */
/******************************************************************************/

static gboolean plugin_load(PurplePlugin *plugin)
{
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
	return TRUE;
}

static void fb_set_status_cb(PurplePluginAction *action)
{
	PurpleConnection *pc = action->context;
	FacebookAccount *fba = pc->proto_data;
	gchar *uid_str;

	uid_str = g_strdup_printf("%" G_GINT64_FORMAT, fba->uid);

	purple_request_input(pc, NULL, _("Set your Facebook status"),
			purple_account_get_alias(pc->account), "is ",
			FALSE, FALSE, NULL, _("OK"),
			G_CALLBACK(fb_set_status_ok_cb), _("Cancel"),
			NULL, pc->account, uid_str, NULL, pc);

	g_free(uid_str);
}

static void fb_display_plugin_info(PurplePluginAction *action)
{
	purple_notify_info(action->context, _("About Facebook for Pidgin"),
			_("Version"), FACEBOOK_PLUGIN_VERSION);
}

static void fb_refresh_blist(PurplePluginAction *action)
{
	PurpleConnection *pc;
	FacebookAccount *fba;

	pc = (PurpleConnection *) action->context;
	fba = pc->proto_data;

	fb_get_buddy_list(fba);
}

static GList *fb_actions(PurplePlugin *plugin, gpointer context)
{
	GList *m = NULL;
	PurplePluginAction *act;

	act = purple_plugin_action_new(_("About Facebook for Pidgin"),
			fb_display_plugin_info);
	m = g_list_append(m, act);

	act = purple_plugin_action_new(_("Set Facebook status..."),
			fb_set_status_cb);
	m = g_list_append(m, act);

	act = purple_plugin_action_new(_("Search for buddies..."),
			fb_search_users);
	m = g_list_append(m, act);

	// TODO: remove, this is for testing.  REMOVE.
	act = purple_plugin_action_new(_("Refresh buddy list..."),
			fb_refresh_blist);
	m = g_list_append(m, act);

	return m;
}

static GList *fb_node_menu(PurpleBlistNode *node)
{
	GList *m = NULL;
	PurpleMenuAction *act;
	PurpleBuddy *buddy;
	
	if(PURPLE_BLIST_NODE_IS_BUDDY(node))
	{
		buddy = (PurpleBuddy *)node;
		
		act = purple_menu_action_new(_("_Poke"),
				PURPLE_CALLBACK(fb_blist_poke_buddy),
				NULL, NULL);
		m = g_list_append(m, act);
	}
	return m;
}

static void fb_set_idle(PurpleConnection *gc, int time)
{
	FacebookAccount *fba = gc->proto_data;
	
	g_return_if_fail(fba != NULL);
	
	if (time)
	{
		fba->is_idle = TRUE;
	} else {
		fba->is_idle = FALSE;
	}
}

static void plugin_init(PurplePlugin *plugin)
{
	PurpleAccountOption *option;
	PurplePluginInfo *info = plugin->info;
	PurplePluginProtocolInfo *prpl_info = info->extra_info;

	/* Add options to the advanced screen in the account settings */
	option = purple_account_option_bool_new(
		_("Show history in new conversations"),
		"facebook_show_history", TRUE);
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);

	option = purple_account_option_bool_new(
		_("Use Facebook friend-lists as Pidgin groups"),
		"facebook_use_groups", TRUE);
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);
	option = purple_account_option_bool_new(
		_("Hide myself in the Buddy List"),
		"facebook_hide_self", TRUE);
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);

	option = purple_account_option_bool_new(
		_("Set Facebook status through Pidgin status"),
		"facebook_set_status_through_pidgin", FALSE);
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);

	option = purple_account_option_bool_new(
		_("Show Facebook notifications as e-mails in Pidgin"),
		"facebook_get_notifications", TRUE);
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);
	
	option = purple_account_option_string_new(
		_("Notifications RSS Feed URL"),
		"notifications_feed_url", "");
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);

	option = purple_account_option_bool_new(
		_("Edit Facebook friends from Pidgin"),
		"facebook_manage_friends", FALSE);
	prpl_info->protocol_options = g_list_append(
		prpl_info->protocol_options, option);
}

static PurplePluginProtocolInfo prpl_info = {
	/* options */
	OPT_PROTO_MAIL_CHECK,

	NULL,                   /* user_splits */
	NULL,                   /* protocol_options */
	/* NO_BUDDY_ICONS */    /* icon_spec */
	{"jpg", 0, 0, 50, 50, -1, PURPLE_ICON_SCALE_SEND}, /* icon_spec */
	fb_list_icon,           /* list_icon */
	NULL,                   /* list_emblems */
	fb_status_text,         /* status_text */
	fb_tooltip_text,        /* tooltip_text */
	fb_statuses,            /* status_types */
	fb_node_menu,           /* blist_node_menu */
	NULL,                   /* chat_info */
	NULL,                   /* chat_info_defaults */
	fb_login,               /* login */
	fb_close,               /* close */
	fb_send_im,             /* send_im */
	NULL,                   /* set_info */
	fb_send_typing,         /* send_typing */
	fb_get_info,            /* get_info */
	fb_set_status_p,        /* set_status */
	fb_set_idle,            /* set_idle */
	NULL,                   /* change_passwd */
	fb_add_buddy,           /* add_buddy */
	NULL,                   /* add_buddies */
	fb_buddy_remove,        /* remove_buddy */
	NULL,                   /* remove_buddies */
	NULL,                   /* add_permit */
	NULL,                   /* add_deny */
	NULL,                   /* rem_permit */
	NULL,                   /* rem_deny */
	NULL,                   /* set_permit_deny */
	NULL,                   /* join_chat */
	NULL,                   /* reject chat invite */
	NULL,                   /* get_chat_name */
	NULL,                   /* chat_invite */
	NULL,                   /* chat_leave */
	NULL,                   /* chat_whisper */
	NULL,                   /* chat_send */
	NULL,                   /* keepalive */
	NULL,                   /* register_user */
	NULL,                   /* get_cb_info */
	NULL,                   /* get_cb_away */
	NULL,                   /* alias_buddy */
	fb_group_buddy_move,    /* group_buddy */
	fb_group_rename,        /* rename_group */
	fb_buddy_free,          /* buddy_free */
	fb_conversation_closed, /* convo_closed */
	purple_normalize_nocase,/* normalize */
	NULL,                   /* set_buddy_icon */
	fb_group_remove,        /* remove_group */
	NULL,                   /* get_cb_real_name */
	NULL,                   /* set_chat_topic */
	NULL,                   /* find_blist_chat */
	NULL,                   /* roomlist_get_list */
	NULL,                   /* roomlist_cancel */
	NULL,                   /* roomlist_expand_category */
	NULL,                   /* can_receive_file */
	NULL,                   /* send_file */
	NULL,                   /* new_xfer */
	NULL,                   /* offline_message */
	NULL,                   /* whiteboard_prpl_ops */
	NULL,                   /* send_raw */
	NULL,                   /* roomlist_room_serialize */
	NULL,                   /* unregister_user */
	NULL,                   /* send_attention */
	NULL,                   /* attention_types */
#if PURPLE_MAJOR_VERSION >= 2 && PURPLE_MINOR_VERSION >= 5
	sizeof(PurplePluginProtocolInfo), /* struct_size */
	fb_get_account_text_table, /* get_account_text_table */
#else
	(gpointer) sizeof(PurplePluginProtocolInfo)
#endif
};

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	2,						/* major_version */
	3, 						/* minor version */
	PURPLE_PLUGIN_PROTOCOL, 			/* type */
	NULL, 						/* ui_requirement */
	0, 						/* flags */
	NULL, 						/* dependencies */
	PURPLE_PRIORITY_DEFAULT, 			/* priority */
	FACEBOOK_PLUGIN_ID,				/* id */
	"Facebook", 					/* name */
	FACEBOOK_PLUGIN_VERSION, 			/* version */
	N_("Facebook Protocol Plugin"), 		/* summary */
	N_("Facebook Protocol Plugin"), 		/* description */
	"Eion Robb <eionrobb@gmail.com>", 		/* author */
	"http://pidgin-facebookchat.googlecode.com/",	/* homepage */
	plugin_load, 					/* load */
	plugin_unload, 					/* unload */
	NULL, 						/* destroy */
	NULL, 						/* ui_info */
	&prpl_info, 					/* extra_info */
	NULL, 						/* prefs_info */
	fb_actions, 					/* actions */

							/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

PURPLE_INIT_PLUGIN(facebook, plugin_init, info);
