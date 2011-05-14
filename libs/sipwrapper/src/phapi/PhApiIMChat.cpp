/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include "PhApiIMChat.h"

#include "PhApiWrapper.h"

#include <imwrapper/IMChatSession.h>

PhApiIMChat * PhApiIMChat::PhApiIMChatHack = NULL;

PhApiIMChat::PhApiIMChat(IMAccount & account, PhApiWrapper & phApiWrapper)
	: IMChat(account),
	_phApiWrapper(phApiWrapper) 
{
	PhApiIMChatHack = this;

	_phApiWrapper.messageReceivedEvent		   += boost::bind(&PhApiIMChat::messageReceivedEventHandler,		 this, _1, _2, _3, _4);
	_phApiWrapper.statusMessageReceivedEvent   += boost::bind(&PhApiIMChat::statusMessageReceivedEventHandler,	 this, _1, _2, _3, _4);
	_phApiWrapper.typingStateChangedEvent	   += boost::bind(&PhApiIMChat::typingStateChangedEventHandler,		 this, _1, _2, _3, _4);
	_phApiWrapper.contactAddedEvent			   += boost::bind(&PhApiIMChat::contactAddedEventHandler,			 this, _1, _2, _3);
	_phApiWrapper.contactRemovedEvent		   += boost::bind(&PhApiIMChat::contactRemovedEventHandler,			 this, _1, _2, _3);
	_phApiWrapper.newIMChatSessionCreatedEvent += boost::bind(&PhApiIMChat::newIMChatSessionCreatedEventHandler, this, _1, _2);
}

PhApiIMChat::~PhApiIMChat() {
}

void PhApiIMChat::messageReceivedEventHandler(PhApiWrapper & sender, 
	IMChatSession & chatSession, const std::string & from, const std::string & message) {

	if (!message.empty()) {
		messageReceivedEvent(*this, chatSession, from, message);
	}
}

void PhApiIMChat::statusMessageReceivedEventHandler(PhApiWrapper & sender,
	IMChatSession & chatSession, StatusMessage status, const std::string & message) {

	statusMessageReceivedEvent(*this, chatSession, status, message);
}

void PhApiIMChat::typingStateChangedEventHandler(PhApiWrapper & sender,
	IMChatSession & chatSession, const std::string & contactId, IMChat::TypingState state) {
	typingStateChangedEvent(*this, chatSession, contactId, state);
}

void PhApiIMChat::newIMChatSessionCreatedEventHandler(PhApiWrapper & sender, IMChatSession & imChatSession) {
	newIMChatSessionCreatedEvent(*this, imChatSession);
}

void PhApiIMChat::contactAddedEventHandler(PhApiWrapper & sender,
	IMChatSession & imChatSession, const std::string & contactId) {

//	IMContact imContact(_imAccount, contactId);
	IMContact imContact(getIMAccount(), contactId);	//VOXOX - JRT - 2009.06.15 

	if (imChatSession.getIMContactSet().find(imContact) == imChatSession.getIMContactSet().end()) {
		((IMContactSet&)imChatSession.getIMContactSet()).insert(imContact);
		contactAddedEvent(*this, imChatSession, imContact);
	}
}

void PhApiIMChat::contactRemovedEventHandler(PhApiWrapper & sender,
	IMChatSession & imChatSession, const std::string & contactId) {

	contactRemovedEvent(*this, imChatSession, contactId);
}

void PhApiIMChat::sendMessage(IMChatSession & chatSession, const std::string & message) {

	if (!message.empty()) {
		_phApiWrapper.sendMessage(chatSession, message);
	}
}

void PhApiIMChat::changeTypingState(IMChatSession & chatSession, IMChat::TypingState state) {
	_phApiWrapper.changeTypingState(chatSession, state);
}

void PhApiIMChat::createSession(const IMContactSet & imContactSet, IMChat::IMChatType imChatType, const std::string& userChatRoomName ) {	//VOXOX - JRT - 2009.06.15 
	_phApiWrapper.createSession(*this, imContactSet, imChatType, userChatRoomName);
}
////VOXOX - CJC - 2009.05.07 Not supported by PhApiImChat
void PhApiIMChat::createChatToEmailSession(const IMContactSet & imContactSet,const std::string & relatedContactId) {
}
//VOXOX - CJC - 2009.05.19 Not supported
void PhApiIMChat::createChatToSMSSession(const IMContactSet & imContactSet,const std::string & relatedContactId) {
}

//VOXOX CHANGE CJC ADD SUPPORT FOR 
void PhApiIMChat::createSessionAndSendMessage(const IMContactSet & imContactSet, const std::string & message) {
	//TODO: Define SendMessage
}

//VOXOX CHANGE CJC ADD SUPPORT FOR CALLING SKYPE CONTACTS
void PhApiIMChat::callSkypeContact(const IMContactSet & imContactSet) {
	//TODO: THIS SHOULD NOT BE CALLED
}
	
void PhApiIMChat::closeSession(IMChatSession & chatSession) {
	_phApiWrapper.closeSession(chatSession);
}

void PhApiIMChat::addContact(IMChatSession & chatSession, const std::string & contactId) {
	_phApiWrapper.addContact(chatSession, contactId);
}

void PhApiIMChat::removeContact(IMChatSession & chatSession, const std::string & contactId) {
	_phApiWrapper.removeContact(chatSession, contactId);
}
