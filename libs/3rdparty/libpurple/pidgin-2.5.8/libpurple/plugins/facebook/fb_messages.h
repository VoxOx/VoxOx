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

#ifndef FACEBOOK_MESSAGES_H
#define FACEBOOK_MESSAGES_H

#include "libfacebook.h"

gboolean fb_get_post_form_id(FacebookAccount *fba);
gboolean fb_reconnect(FacebookAccount *fba);
int fb_send_im(PurpleConnection *pc, const gchar *who, const gchar *message,
		PurpleMessageFlags flags);

void fb_cancel_resending_messages(FacebookAccount *fba);

#endif /* FACEBOOK_MESSAGES_H */
