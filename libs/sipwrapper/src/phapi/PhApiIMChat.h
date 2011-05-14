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

#ifndef PHAPIIMCHAT_H
#define PHAPIIMCHAT_H

#include "PhApiFactory.h"

#include <imwrapper/IMChat.h>
#include <imwrapper/IMChatSession.h>

class IMAccount;

/**
 * PhApi IM chat.
 *
 * @ingroup model
 * @author Mathieu Stute
 */
class PhApiIMChat : public IMChat {
	friend class PhApiFactory;
	friend class PhApiWrapper;
	friend class PhApiCallbacks;
public:

	void sendMessage	  (IMChatSession & chatSession, const std::string & message);
	void changeTypingState(IMChatSession & chatSession, TypingState state);

	void createSession				(const IMContactSet & imContactSet, IMChat::IMChatType imChatType, const std::string& userChatRoomName );	//VOXOX - JRT - 2009.06.15 
	void createChatToEmailSession	(const IMContactSet & imContactSet, const std::string & relatedContactId);
	void createChatToSMSSession		(const IMContactSet & imContactSet, const std::string & relatedContactId);
	void createSessionAndSendMessage(const IMContactSet & imContactSet, const std::string & message);

	//TODO: THIS SHOULD NOT BE HERE WHEN< ADDING JUST FOR COMPILING PURPUSES
	void callSkypeContact(const IMContactSet & imContactList);

	void closeSession(IMChatSession & chatSession);
	void addContact(IMChatSession & chatSession, const std::string & contactId);
	void removeContact(IMChatSession & chatSession, const std::string & contactId);


	static PhApiIMChat * PhApiIMChatHack;

private:

	PhApiIMChat(IMAccount & account, PhApiWrapper & phApiWrapper);

	~PhApiIMChat();

	void messageReceivedEventHandler(PhApiWrapper & sender, IMChatSession & chatSession, const std::string & from, const std::string & message);

	void statusMessageReceivedEventHandler(PhApiWrapper & sender, IMChatSession & chatSession, StatusMessage status, const std::string & message);

	void typingStateChangedEventHandler(PhApiWrapper & sender, IMChatSession & chatSession, const std::string & contactId, IMChat::TypingState state);

	void newIMChatSessionCreatedEventHandler(PhApiWrapper & sender, IMChatSession & imChatSession);

	void contactAddedEventHandler(PhApiWrapper & sender, IMChatSession & imChatSession, const std::string & contactId);

	void contactRemovedEventHandler(PhApiWrapper & sender, IMChatSession & imChatSession, const std::string & contactId);

	PhApiWrapper & _phApiWrapper;
};

#endif	//PHAPIIMCHAT_H
