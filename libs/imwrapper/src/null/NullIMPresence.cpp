/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "NullIMPresence.h"

#include <util/Logger.h>

NullIMPresence::NullIMPresence(IMAccount & account)
	: IMPresence(account) {
}

void NullIMPresence::changeMyPresence(EnumPresenceState::PresenceState state,
		const std::string & note) {
}

void NullIMPresence::changeMyNickname(const std::string & nickname) {
}

void NullIMPresence::subscribeToPresenceOf(const std::string & contactId) {
}

void NullIMPresence::blockContact(const std::string & contactId) {
}

void NullIMPresence::unblockContact(const std::string & contactId) {
}
