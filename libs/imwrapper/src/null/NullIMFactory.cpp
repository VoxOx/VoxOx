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

#include "NullIMFactory.h"

#include "NullIMChat.h"
#include "NullIMConnect.h"
#include "NullIMPresence.h"
#include "NullIMContactList.h"

NullIMFactory::NullIMFactory() {
}

IMConnect * NullIMFactory::createIMConnect(IMAccount & account) {
	return new NullIMConnect(account);
}

IMChat * NullIMFactory::createIMChat(IMAccount & account) {
	return new NullIMChat(account);
}

IMPresence * NullIMFactory::createIMPresence(IMAccount & account) {
	return new NullIMPresence(account);
}

IMContactList * NullIMFactory::createIMContactList(IMAccount & account) {
	return new NullIMContactList(account);
}
