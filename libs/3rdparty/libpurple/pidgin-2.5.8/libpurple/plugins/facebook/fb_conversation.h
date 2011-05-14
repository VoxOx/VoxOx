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

#ifndef FACEBOOK_CONVERSATION_H
#define FACEBOOK_CONVERSATION_H

#include "libfacebook.h"

void fb_conversation_init(FacebookAccount *fba);
void fb_conversation_destroy(FacebookAccount *fba);

void fb_conversation_closed(PurpleConnection *gc, const char *who);
gboolean fb_conversation_is_fb(PurpleConversation *conv);

void fb_history_fetch(FacebookAccount *fba, const char *who,
		gboolean display_all);
void fb_conversation_handle_message(FacebookAccount *fba, const char *from,
	const char *to, gint64 message_time, const gchar *message_orig,
	gboolean log);

#endif /* FACEBOOK_CONVERSATION_H */
