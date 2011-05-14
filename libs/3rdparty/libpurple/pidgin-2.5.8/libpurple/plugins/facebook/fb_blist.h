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

#ifndef FACEBOOK_BLIST_H
#define FACEBOOK_BLIST_H

#include "libfacebook.h"

gboolean fb_get_buddy_list(gpointer data);
void fb_blist_poke_buddy(PurpleBlistNode *node, gpointer data);

void fb_blist_set_alias(FacebookAccount *fba, const char *id,
		const char *name);

void fb_blist_init(FacebookAccount *fba);
void fb_blist_destroy(FacebookAccount *fba);

#endif /* FACEBOOK_BLIST_H */
