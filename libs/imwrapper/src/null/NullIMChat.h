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

#ifndef NULLIMCHAT_H
#define NULLIMCHAT_H

#include "NullIMFactory.h"

#include <imwrapper/IMChat.h>

class IMAccount;
class IMContactSet;

/**
 * Null IM chat (stub).
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class NullIMChat : public IMChat {
	friend class NullIMFactory;
public:

	void createSession(IMContactSet & imContactList);

private:

	NullIMChat(IMAccount & account);

	void closeSession(IMChatSession & chatSession);

	void sendMessage(IMChatSession & chatSession, const std::string & message);

	void addContact(IMChatSession & chatSession, const std::string & contactId);

	void removeContact(IMChatSession & chatSession, const std::string & contactId);
};

#endif	//NULLIMCHAT_H
