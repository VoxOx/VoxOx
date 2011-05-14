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

#include "fb_messages.h"
#include "fb_connection.h"
#include "fb_conversation.h"
#include "fb_blist.h"
#include "fb_util.h"

#include "conversation.h"

typedef struct _FacebookOutgoingMessage FacebookOutgoingMessage;

struct _FacebookOutgoingMessage {
	FacebookAccount *fba;
	gchar *who;
	GTimeVal time;
	gchar *message;
	gint msg_id;
	guint retry_count;
	guint resend_timer;
};

static gboolean fb_send_im_fom(FacebookOutgoingMessage *msg);
static gboolean fb_resend_im_fom(FacebookOutgoingMessage *msg);
static gboolean fb_get_new_messages(FacebookAccount *fba);

static FacebookOutgoingMessage *fb_msg_create(FacebookAccount *fba)
{
	FacebookOutgoingMessage *msg;
	
	msg = g_new0(FacebookOutgoingMessage, 1);
	msg->fba = fba;
	
	return msg;
}

static void fb_msg_destroy(FacebookOutgoingMessage *msg)
{
	if (msg->resend_timer) {
		purple_timeout_remove(msg->resend_timer);
	}
	g_free(msg->who);
	g_free(msg->message);
	g_free(msg);	
}

void fb_cancel_resending_messages(FacebookAccount *fba)
{
	while (fba->resending_messages != NULL) {
		FacebookOutgoingMessage *msg = fba->resending_messages->data;
		fba->resending_messages = g_slist_remove(fba->resending_messages, msg);
		fb_msg_destroy(msg);
	}	
}

static void parse_message(PurpleConnection *pc, FacebookAccount *fba,
	JsonObject *messageObj, const char* from, const char* to)
{
	gint64 message_time;
	const gchar *message;

	purple_debug_info("facebook", "message from %s to %s\n", from, to);

	message = json_node_get_string(
		json_object_get_member(messageObj, "text"));

	message_time = fb_time_kludge(json_node_get_int(
				json_object_get_member(messageObj, "time")));

	fb_conversation_handle_message(fba, from, to, message_time, message, TRUE);			
}

static void parse_new_messages(PurpleConnection *pc, FacebookAccount *fba, JsonArray *messages)
{
	int i;
	PurpleBuddy *buddy;
	gint typing;//VOXOX - CJC - 2009.07.06 
	purple_debug_info("facebook", "parsing new messages\n");

	for (i = 0; i < json_array_get_length(messages); i++) {
		const gchar *type;
		gchar *from, *to;

		JsonObject *object = json_node_get_object(json_array_get_element(messages, i));
		type = json_node_get_string(json_object_get_member(object, "type"));

		from = g_strdup_printf("%" G_GINT64_FORMAT, (gint64)json_node_get_int(json_object_get_member(object, "from")));
		to = g_strdup_printf("%" G_GINT64_FORMAT, (gint64)json_node_get_int(json_object_get_member(object, "to")));
		
		/* Use the in-line buddy name if the buddy list hasn't been downloaded yet */
		buddy = purple_find_buddy(pc->account, from);
		if (buddy == NULL || buddy->server_alias == NULL || buddy->alias == NULL)
		{
			if (json_object_has_member(object, "from_name"))
			{
				const gchar *from_name = json_node_get_string(json_object_get_member(
					object, "from_name"));
				fb_blist_set_alias(fba, from, from_name);
			}
		}

		if (from && to && g_str_equal(type, "msg")) {
			JsonObject *messageObj = json_node_get_object(json_object_get_member(object, "msg"));
			parse_message(pc, fba, messageObj, from, to);
		} else if (from && g_str_equal(type, "typ")) {
			purple_debug_info("facebook", "handling typing notification\n");

			typing = json_node_get_int(json_object_get_member(object, "st"));
			if (typing == 0) {
				serv_got_typing(pc, from, 10, PURPLE_NOT_TYPING);
			} else {
				serv_got_typing(pc, from, 10, PURPLE_TYPING);
			}
		}

		/*
		 * we've received something from a buddy, assume they're online
		 * only if it's not from ourselves
		 */
		if (from && fba->uid != atoll(from)) {
			purple_prpl_got_user_status(
				fba->account, from, 
				purple_primitive_get_id_from_type(PURPLE_STATUS_AVAILABLE), NULL);

		}
	
		g_free(from);
		g_free(to);

		fba->message_fetch_sequence++;
	}
}
								  
static void got_new_messages(FacebookAccount *fba, gchar *data,
		gsize data_len, gpointer userdata)
{
	PurpleConnection *pc;//VOXOX - CJC - 2009.07.06 
	JsonParser *parser;
	GList *conversations;//VOXOX - CJC - 2009.07.06 
	JsonObject *objnode;//VOXOX - CJC - 2009.07.06 
	
	pc= userdata;//VOXOX - CJC - 2009.07.06 

	purple_debug_misc("facebook", "got new messages:\n%s\n", data);

	/* for (;;);{"t":"msg","c":"p_800753867","ms":[{"type":"msg",
		"msg":{"text":"yes","time":1211176515861,"clientTime":1211176514750,
		"msgID":"367146364"},"from":596176850,"to":800753867,
		"from_name":"Jeremy Lawson","to_name":"Eion Robb",
		"from_first_name":"Jeremy","to_first_name":"Eion"}]} */
	/* for (;;);{"t":"refresh"} */
	/* for (;;);{"t":"msg","c":"p_800753867","ms":[{"type":"msg",
		"msg":{"text":"porn head","time":1211177326689,"clientTime":1211177325,
		"msgID":"-1992480367"},"from":800753867,"to":596176850,
		"from_name":"Eion Robb","to_name":"Jeremy Lawson",
		"from_first_name":"Eion","to_first_name":"Jeremy"}]} */
	/* for (;;);{"t":"msg","c":"p_800753867","ms":[{"type":"typ","st":1,
		"from":596176850,"to":800753867},{"type":"msg","msg":{"text":"nubile!",
		"time":1211177334019,"clientTime":1211177326690,"msgID":"696260545"},
		"from":596176850,"to":800753867,"from_name":"Jeremy Lawson",
		"to_name":"Eion Robb","from_first_name":"Jeremy","to_first_name":"Eion"},
		{"type":"msg","msg":{"text":"test2","time":1211177336688,
		"clientTime":1211177326691,"msgID":"1527815367"},"from":596176850,
		"to":800753867,"from_name":"Jeremy Lawson","to_name":"Eion Robb",
		"from_first_name":"Jeremy","to_first_name":"Eion"},{"type":"msg",
		"msg":{"text":"ahhhhhhh!","time":1211177344361,"clientTime":1211177326692,
		"msgID":"4028916254"},"from":596176850,"to":800753867,
		"from_name":"Jeremy Lawson","to_name":"Eion Robb",
		"from_first_name":"Jeremy","to_first_name":"Eion"}]} */
	/* for (;;);{"t":"msg","c":"p_800753867","ms":[{"type":"msg",
		"msg":{"text":"2","time":1211178167261,"clientTime":1211178164656,
		"msgID":"3382240259"},"from":596176850,"to":800753867,
		"from_name":"Jeremy Lawson","to_name":"Eion Robb",
		"from_first_name":"Jeremy","to_first_name":"Eion"}]} */
	/* for (;;);{"t":"refresh", "seq":1} */

	parser = fb_get_parser(data, data_len);
	if (!parser)
	{
		/* Sometimes proxies will return incorrect data, so we just shrug 
		 * it off.
		 * TODO: Only do this for proxies.  And when we do it, keep track
		 * of consecutive failures in the case something is actually
		 * wrong with Facebook.  Eventually this condition should cause
		 * failure */
		/* Continue looping, waiting for more messages */
		purple_debug_error("facebook",
				"got data back, but it's not even json\n");
				
		fb_get_new_messages(fba);
		return;
	}

	objnode = fb_get_json_object(parser, NULL);

	if (json_object_has_member(objnode, "t")) {
		const gchar* command = json_node_get_string(json_object_get_member(objnode, "t"));
		if (g_str_equal(command, "refresh")) {
			if (json_object_has_member(objnode, "seq")) {
				fba->message_fetch_sequence = json_node_get_int(
					json_object_get_member(objnode, "seq"));
			}

			/* grab history items for all open conversations */
			conversations = purple_get_conversations();
			while (conversations != NULL) {
				PurpleConversation *conv =
					(PurpleConversation *)conversations->data;
				if (fb_conversation_is_fb(conv)) {
					purple_debug_info("facebook",
						"checking for dropped messages with %s\n",
						conv->name);
					fb_history_fetch(fba, conv->name, FALSE);
				}
				conversations = conversations->next;
			}

			/* refresh means that the channel is invalid */
			fb_reconnect(fba);
			json_parser_free(parser);
			return;
		} else if (g_str_equal(command, "continue")) {
			/* continue means that the server wants us to remake the connection.
 			 * continue the loop and wait for messages. noop. */
		} else if (g_str_equal(command, "msg")) {
			parse_new_messages(pc, fba,
				json_node_get_array(json_object_get_member(objnode, "ms")));
		}
	}

	json_parser_free(parser);

	/* Continue looping, waiting for more messages */
	fb_get_new_messages(fba);
}

/* Message fetch loop.  An invariant here is that get_new_messages()
 * is the only function which can create a new timer for
 * new_messages_check_timer.  The timer always calls got_new_messages(),
 * and got_new_messages always calls get_new_messages (either directly
 * or indirectly).  To start the loops off, get_new_messages should be
 * called ONCE and only ONCE.  After that the timers will take care of
 * themselves until final cleanup.
 */
static gboolean fb_get_new_messages(FacebookAccount *fba)
{
	time_t now;
	gchar *fetch_url;
	gchar *fetch_server;
	const gchar *channel_number;

	fba->new_messages_check_timer = 0;

	now = time(NULL);
	if (fba->last_messages_download_time > now - 3) {
		/*
		 * Wait a bit before fetching more messages, to make sure we
		 * never hammer their servers.
		 *
		 * TODO: This could be smarter.  Like, allow 3 requests per
		 *       10 seconds or something.
		 */
		fba->new_messages_check_timer = purple_timeout_add_seconds(
				3 - (now - fba->last_messages_download_time),
				(GSourceFunc)fb_get_new_messages, fba);
		return FALSE;
	}

	if (fba->channel_number == NULL)
	{
		return FALSE;
	} else {
		channel_number = fba->channel_number;
	}

	purple_debug_info("facebook", "getting new messages\n");

	fetch_server = g_strdup_printf("%d.%s.facebook.com", 0, channel_number);
	/* use the current time in the url to get past any transparent proxy caches */
	fetch_url = g_strdup_printf("/x/%lu/%s/p_%" G_GINT64_FORMAT "=%d", (gulong)time(NULL), (fba->is_idle?"false":"true"), fba->uid, fba->message_fetch_sequence);

	fb_post_or_get(fba, FB_METHOD_GET, fetch_server, fetch_url, NULL, got_new_messages, fba->pc, TRUE);
	fba->last_messages_download_time = now;

	g_free(fetch_url);
	g_free(fetch_server);

	return FALSE;
}

static void fb_send_im_cb(FacebookAccount *fba, gchar *data, gsize data_len, gpointer user_data)
{
	FacebookOutgoingMessage *msg = user_data;
	JsonParser *parser;
	JsonObject *object;
	PurpleConversation *conv;
	gchar *error = NULL;

	purple_debug_misc("facebook", "sent im response: %s\n", data);
	
	parser = fb_get_parser(data, data_len);
	if (!parser) {
		// We didn't get data, but this isn't necessarily fatal.
		purple_debug_warning("facebook", "bad data while parsing sent IM\n");
		return;
	}
	object = fb_get_json_object(parser, &error);
	
	if (error)
	{
		purple_debug_error("facebook", "sent im error: %s\n", error);
		/* there was an error, either report it or retry */
		if (msg->retry_count++ < FB_MAX_MSG_RETRY)
		{
			msg->resend_timer = purple_timeout_add_seconds(1, (GSourceFunc)fb_resend_im_fom, msg);
			fba->resending_messages = g_slist_prepend(fba->resending_messages, msg);
			json_parser_free(parser);
			return;
		}
		else
		{
			conv = purple_conversation_new(PURPLE_CONV_TYPE_IM,
					fba->account, msg->who);
			purple_conversation_write(conv, NULL, error,
					PURPLE_MESSAGE_ERROR, msg->time.tv_sec);
		}
		
		g_hash_table_remove(fba->sent_messages_hash, msg->message);
	}

	json_parser_free(parser);
	fb_msg_destroy(msg);
}

static gboolean fb_send_im_fom(FacebookOutgoingMessage *msg)
{
	gchar *encoded_message;
	gchar *postdata;
	gchar *jstime;
	
	jstime = g_strdup_printf("%ld%ld", msg->time.tv_sec, (msg->time.tv_usec/1000));

	encoded_message = g_strdup(purple_url_encode(msg->message));
	postdata = g_strdup_printf("msg_text=%s&msg_id=%d&to=%s&client_time=%s&post_form_id=%s",
			encoded_message, msg->msg_id, msg->who, jstime,
			msg->fba->post_form_id ? msg->fba->post_form_id : "0");
	g_free(encoded_message);
	g_free(jstime);

	fb_post_or_get(msg->fba, FB_METHOD_POST, NULL, "/ajax/chat/send.php", postdata, fb_send_im_cb, msg, FALSE);
	g_free(postdata);

	return FALSE;
}

static gboolean fb_resend_im_fom(FacebookOutgoingMessage *msg)
{
	msg->fba->resending_messages = g_slist_remove(msg->fba->resending_messages, msg);

	return fb_send_im_fom(msg);
}

int fb_send_im(PurpleConnection *pc, const gchar *who, const gchar *message, PurpleMessageFlags flags)
{
	FacebookOutgoingMessage *msg;
	FacebookAccount *fba = pc->proto_data;

	msg = fb_msg_create(fba);

	/* convert html to plaintext, removing trailing spaces */
	msg->message = purple_markup_strip_html(message);
	if (strlen(msg->message) > 999)
	{
		fb_msg_destroy(msg);
		return -E2BIG;
	}

	msg->msg_id = g_random_int();
	msg->who = g_strdup(who);
	g_get_current_time(&msg->time);
	msg->retry_count = 0;

	//save that we're sending the message
	g_hash_table_insert(fba->sent_messages_hash, strdup(msg->message), NULL);

	fb_send_im_fom(msg);

	/* Return 1 so UI will display message */
	return 1;
}

void got_reconnect_json(FacebookAccount *fba, gchar *data, gsize data_len, gpointer userdata)
{
	gchar *new_channel_number;
	const gchar *new_channel_host;
	JsonParser *parser;
	JsonObject *objnode;
	gint new_seq;//VOXOX - CJC - 2009.07.06 
	JsonObject *payload;//VOXOX - CJC - 2009.07.06 
	parser = fb_get_parser(data, data_len);

	if (!parser) {
		purple_debug_error("facebook", "couldn't parse reconnect data\n");
		purple_debug_info("facebook", "page content: %s\n", data);
		purple_connection_error_reason(fba->pc,
				PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
				_("Chat service currently unavailable"));
		return;
	}

	objnode = fb_get_json_object(parser, NULL);

	payload = json_node_get_object(json_object_get_member(objnode, "payload"));//VOXOX - CJC - 2009.07.06 
	
	/* eg {"host":"channel01"} */
	new_channel_host = json_node_get_string(json_object_get_member(payload, "host"));

	if (new_channel_host == NULL)
	{
		purple_debug_error("facebook", "couldn't find new channel number\n");
		purple_debug_info("facebook", "page content: %s\n", data);
		purple_connection_error_reason(fba->pc,
				PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
				_("Error fetching channel; did you log in elsewhere?"));
		json_parser_free(parser);
		return;
	}
	
	g_free(fba->channel_number);
	fba->channel_number = g_strdup(new_channel_host);
	
	new_seq = json_node_get_int(json_object_get_member(payload, "seq"));
	fba->message_fetch_sequence = new_seq;
	
	/*
	 * Now that we have a channel number we can start looping and
	 * waiting for messages
	 */
	fb_get_new_messages(fba);
	json_parser_free(parser);
}

gboolean fb_reconnect(FacebookAccount *fba)
{
	gchar *url = g_strdup_printf("/ajax/presence/reconnect.php?reason=7&post_form_id=%s&__a=1", fba->post_form_id);
	fb_post_or_get(fba, FB_METHOD_GET, NULL, url, NULL, got_reconnect_json, NULL, FALSE);
	g_free(url);
	
	return FALSE;
}

static void got_form_id_page(FacebookAccount *fba, gchar *data, gsize data_len, gpointer userdata)
{
	const gchar *start_text = "id=\"post_form_id\" name=\"post_form_id\" value=\"";
	const gchar *dtsg_start = "fb_dtsg:\"";
	const gchar *channel_start = "js\", \"channel";
	const gchar *channel_start2 = "js\",\"channel";
	gchar *post_form_id;
	gchar *channel = NULL;
	gchar *tmp = NULL;
	
	/* NULL data crashes on Windows */
	if (data == NULL)
		data = "(null)";
	
	tmp = g_strstr_len(data, data_len, start_text);
	if (tmp == NULL)
	{
		purple_debug_error("facebook", "couldn't find post_form_id\n");
		purple_debug_info("facebook", "page content: %s\n", data);
		/* Maybe they changed their HTML slightly? */
		purple_connection_error_reason(fba->pc,
				PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
				_("Error getting info from Facebook."));
		return;
	}
	tmp += strlen(start_text);
	post_form_id = g_strndup(tmp, strchr(tmp, '"') - tmp);

	g_free(fba->post_form_id);
	fba->post_form_id = post_form_id;
	tmp = g_strstr_len(data, data_len, dtsg_start);
	if (tmp != NULL)
	{
		tmp += strlen(dtsg_start);
		g_free(fba->dtsg);
		fba->dtsg = g_strndup(tmp, strchr(tmp, '"') - tmp);
	}

	tmp = g_strstr_len(data, data_len, channel_start);
	if (tmp != NULL)
	{
		tmp += 6;
	} else {
		tmp = g_strstr_len(data, data_len, channel_start2);
		if (tmp != NULL)
			tmp += 5;
	}
	if (tmp != NULL)
	{
		channel = g_strndup(tmp, strchr(tmp, '"') - tmp);
		g_free(fba->channel_number);
		fba->channel_number = channel;
	}

	tmp = g_strdup_printf("visibility=true&post_form_id=%s", post_form_id);
	fb_post_or_get(fba, FB_METHOD_POST, "apps.facebook.com", "/ajax/chat/settings.php", tmp, NULL, NULL, FALSE);
	g_free(tmp);
	
	if (channel == NULL)
	{
		/* Grab new channel number */
		fb_reconnect(fba);
	} else {
		fb_get_new_messages(fba);
	}
}

gboolean fb_get_post_form_id(FacebookAccount *fba)
{
	fb_post_or_get(fba, FB_METHOD_GET, NULL, "/presence/popout.php", NULL, got_form_id_page, NULL, FALSE);
	return FALSE;
}
