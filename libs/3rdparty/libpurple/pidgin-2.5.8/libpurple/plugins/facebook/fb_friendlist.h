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

#ifndef FACEBOOK_FRIENDLIST_H
#define FACEBOOK_FRIENDLIST_H

#include "libfacebook.h"

#include "fb_json.h"

#define DEFAULT_GROUP_NAME "Facebook"

/* Friend list modification methods */
void fb_group_buddy_move(PurpleConnection *pc, const char *who,
	const char *old_group, const char *new_group);
void fb_group_rename(PurpleConnection *pc, const char *old_name,
	PurpleGroup *group, GList *moved_buddies);
void fb_group_remove(PurpleConnection *pc, PurpleGroup *group);
void fb_buddy_remove(PurpleConnection *pc, PurpleBuddy *buddy,
	PurpleGroup *group);

/* Friend list fetch methods */
const gchar *fb_get_list_id(FacebookAccount *fba, const gchar *list_name);
gboolean fb_process_friend_lists(FacebookAccount *fba, JsonObject *buddy_list);
GList *fb_get_buddies_friend_list (FacebookAccount *fba,
		const gchar *uid, JsonArray *friend_list_ids);

void fb_friendlist_init(FacebookAccount *fba);
void fb_friendlist_destroy(FacebookAccount *fba);

#endif /* FACEBOOK_FRIENDLIST_H */
