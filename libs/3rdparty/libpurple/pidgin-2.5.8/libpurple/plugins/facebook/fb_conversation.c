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


#include "fb_conversation.h"
#include "fb_connection.h"
#include "fb_util.h"

#include "fb_json.h"

#include "conversation.h"
#include "signals.h"

/*****************************************************************************
 * MESSAGE PROCESSING                                                        *
 *****************************************************************************/
void fb_conversation_handle_message(FacebookAccount *fba, const char *from,
	const char *to, gint64 message_time, const gchar *message_orig,
	gboolean log)
{
	gchar *tmp, *message_text;

	if (!log) {
		purple_debug_info("facebook", "message with no logging\n");
	}

	// Process message.
	message_text = fb_strdup_withhtml(message_orig);
	tmp = message_text;
	message_text = fb_replace_styled_text(message_text);
	g_free(tmp);

	if (fba->uid != atoll(from) || fba->uid == atoll(to)) {
		purple_debug_info("facebook",
			"displaying received message %lld: %s\n",
			(long long int) message_time, message_text);
		// TODO/FIXME: cheat here by changing formatting colors.
		// Or add an option to just disable history on conv open.  TBD.
		serv_got_im(fba->pc, from, message_text,
			log? 
				PURPLE_MESSAGE_RECV :
				PURPLE_MESSAGE_RECV,
			message_time / 1000);
		if (message_time > fba->last_message_time) {
			fba->last_message_time = message_time;
		} else {
			purple_debug_warning("facebook",
					"displaying message out of sync\n");
		}
	} else if (!g_hash_table_remove(
		fba->sent_messages_hash, message_orig))
	{
		purple_debug_info("facebook",
			"displaying sent message %lld: %s\n",
			(long long int) message_time, message_text);

		serv_got_im(fba->pc, to, message_text,
			log? 
				PURPLE_MESSAGE_SEND :
				PURPLE_MESSAGE_SEND,
			message_time / 1000);
		if (message_time > fba->last_message_time) {
			fba->last_message_time = message_time;
		} else {
			purple_debug_warning("facebook",
					"displaying message out of sync\n");
		}
	}

	// Cleanup.
	g_free(message_text);
}


/*****************************************************************************
 * HISTORY CODE                                                              *
 *****************************************************************************/

static void fb_history_fetch_cb(FacebookAccount *fba, gchar *data,
	gsize data_len, gpointer userdata)
{
	JsonParser *parser;
	JsonObject *object, *payload;
	JsonArray *history;
	guint i;
	gint64 min_time;

	parser = fb_get_parser(data, data_len);

	if (!parser) {
		// We didn't get data, but this isn't necessarily fatal.
		purple_debug_warning("facebook",
			"bad data while fetching history\n");
		return;
	}

	min_time = atoll((char *) userdata);
	g_free(userdata);
	purple_debug_info("facebook", "history fetch with min time of %lld\n",
		       (long long int) min_time);	

	object = fb_get_json_object(parser, NULL);
	payload = json_node_get_object(
		json_object_get_member(object, "payload"));
	history = json_node_get_array(
		json_object_get_member(payload, "history"));

	purple_debug_info("facebook",
			"found %d history items to possibly render\n",
			json_array_get_length(history));

	for (i = 0; i < json_array_get_length(history); i++) {
		const gchar *type;
		JsonObject *message_obj;

		message_obj = json_node_get_object(
			json_array_get_element(history, i));
		type = json_node_get_string(json_object_get_member(
			message_obj, "type"));
	
		if (g_str_equal(type, "msg")) {
			gint64 message_time;
			const gchar *message;
			gchar *from;
			gchar *to;
			JsonObject *text_obj;

			from = g_strdup_printf("%" G_GINT64_FORMAT, (gint64)json_node_get_int(
				json_object_get_member(message_obj, "from")));
			to = g_strdup_printf("%" G_GINT64_FORMAT, (gint64)json_node_get_int(
				json_object_get_member(message_obj, "to")));

			text_obj = json_node_get_object(
				json_object_get_member(message_obj, "msg"));
			message = json_node_get_string(
				json_object_get_member(text_obj, "text"));

			message_time = fb_time_kludge(json_node_get_int(
				json_object_get_member(message_obj, "time")));

			if (message_time > min_time) {
				purple_debug_info("facebook",
					"displaying history message %lld\n",
					(long long int) message_time);
				fb_conversation_handle_message(
					fba, from, to, message_time, message,
					min_time != 0);
			}

			g_free(from);
			g_free(to);
		}
	}
	
	json_parser_free(parser);
}

void fb_history_fetch(FacebookAccount *fba, const char *who,
		gboolean display_all)
{
	gint64 min_time;//VOXOX - CJC - 2009.07.06 
	gchar *url;//VOXOX - CJC - 2009.07.06 
	g_return_if_fail(fba != NULL);
	
	purple_debug_info("facebook", "fetching history with %s\n", who);

	min_time = fba->last_message_time;
	if (display_all) {
		min_time = 0;
	}

	url = g_strdup_printf("/ajax/chat/history.php?id=%s", who);
	fb_post_or_get(
		fba, FB_METHOD_GET, NULL, url, NULL, fb_history_fetch_cb,
		g_strdup_printf("%lld", (long long int) min_time), FALSE);
	g_free(url);
}

/*****************************************************************************
 * GENERAL EVENTS CODE                                                       *
 *****************************************************************************/

void fb_conversation_closed(PurpleConnection *gc, const char *who)
{
	FacebookAccount *fba = gc->proto_data;
	gchar *postdata;

	g_return_if_fail(fba->post_form_id != NULL);

	/* notify server that we closed the chat window */
	/* close_chat=589039771&window_id=3168919846&
	 * post_form_id=c258fe42460c7e8b61e242a37ef05afc */
	postdata = g_strdup_printf("close_chat=%s&post_form_id=%s&fb_dtsg=%s&"
			"post_form_id_source=AsyncRequest&__a=1", who,
			fba->post_form_id, fba->dtsg);
	fb_post_or_get(fba, FB_METHOD_POST, NULL, "/ajax/chat/settings.php",
			postdata, NULL, NULL, FALSE);
	g_free(postdata);
}

static void fb_conversation_created(PurpleConversation *conv)
{
	PurpleAccount *account = purple_conversation_get_account(conv);

	if (!fb_conversation_is_fb(conv)) {
		return;
	}

	purple_debug_info("facebook", "conversation created with %s\n",
		conv->name);

	/*if (purple_account_get_bool(account, "facebook_show_history", TRUE))
	{
		fb_history_fetch(account->gc->proto_data, conv->name, TRUE); //VOXOX - CJC - 2009.07.06 Dont get status messages
	}*/
}

gboolean fb_conversation_is_fb(PurpleConversation *conv)
{
	PurpleAccount *account = purple_conversation_get_account(conv);
	const gchar *prpl = purple_account_get_protocol_id(account);
	return g_str_equal(prpl, FACEBOOK_PLUGIN_ID);
}

void fb_conversation_init(FacebookAccount *fba)
{
	fba->last_message_time = 0;

	purple_signal_connect(
		purple_conversations_get_handle(),
		"conversation-created",
		fba,
		PURPLE_CALLBACK(fb_conversation_created),
		NULL);
}

void fb_conversation_destroy(FacebookAccount *fba)
{
	purple_signal_disconnect(
		purple_conversations_get_handle(),
		"conversation-created",
		fba,
		PURPLE_CALLBACK(fb_conversation_created));
}


