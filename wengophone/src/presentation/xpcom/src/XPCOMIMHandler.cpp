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

#include "XPCOMIMHandler.h"

#include "ListenerList.h"

#include <imwrapper/IMWrapperFactory.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccount.h>
#include <imwrapper/IMConnect.h>
#include <imwrapper/IMChat.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMPresence.h>

#include <model/phonecall/SipAddress.h>

#include <util/Logger.h>

void XPCOMIMHandler::connected() {
	if (!_account) {
		_account = new IMAccount(_wengoLogin, _wengoPassword, EnumIMProtocol::IMProtocolSIPSIMPLE);
	}

	if (!_connect) {
		_connect = IMWrapperFactory::getFactory().createIMConnect(*_account);
	}

	if (!_chat) {
		_chat = IMWrapperFactory::getFactory().createIMChat(*_account);
		_chatSession = new IMChatSession(*_chat);
		_chatSession->messageReceivedEvent +=
				boost::bind(&XPCOMIMHandler::messageReceivedEventHandler, this, _1, _2, _3);
	}

	if (!_presence) {
		_presence = IMWrapperFactory::getFactory().createIMPresence(*_account);
		//VOXOX CHANGE SUPPORT STATUS MESSAGE
		_presence->presenceStateChangedEvent +=
				boost::bind(&XPCOMIMHandler::presenceStateChangedEventHandler, this, _1, _2, _3, _4, _5);
	}
}

void XPCOMIMHandler::messageReceivedEventHandler(IMChatSession & sender, const IMContact & from, const std::string & message) {
	SipAddress sipUri(from.getContactId());
	std::string sipAddress = sipUri.getSipAddress();
	std::string userName = sipUri.getUserName();
	std::string displayName = sipUri.getDisplayName();

	ListenerList & listenerList = ListenerList::getInstance();

	for (unsigned i = 0; i < listenerList.size(); i++) {
		Listener * listener = listenerList[i];
		listener->chatMessageReceivedEvent(message, sipAddress, userName, displayName, NULL);
	}
}

void XPCOMIMHandler::presenceStateChangedEventHandler(IMPresence & sender, EnumPresenceState::PresenceState state,
		const std::string & note, const std::string & statusMessage, const std::string & from) {

	SipAddress sipUri(from);
	std::string sipAddress = sipUri.getSipAddress();
	std::string userName = sipUri.getUserName();
	std::string displayName = sipUri.getDisplayName();

	ListenerList & listenerList = ListenerList::getInstance();

	for (unsigned i = 0; i < listenerList.size(); i++) {
		Listener * listener = listenerList[i];
		//VOXOX CHANGE SUPPORT STATUS MESSAGE
		listener->presenceStateChangedEvent(state, note, statusMessage, sipAddress, userName, displayName, NULL);
	}
}

void XPCOMIMHandler::subscribeToPresenceOf(const std::string & sipAddress) {
	if (!_presence) {
		LOG_DEBUG("cannot call subscribeToPresenceOf(): _presence is NULL");
		return;
	}

	_presence->subscribeToPresenceOf(sipAddress);
}

void XPCOMIMHandler::publishMyPresence(EnumPresenceState::PresenceState state, const std::string & note) {
	if (!_presence) {
		LOG_DEBUG("cannot call publishMyPresence(): _presence is NULL");
		return;
	}

	_presence->changeMyPresence(state);
}

int XPCOMIMHandler::sendChatMessage(const std::string & sipAddress, const std::string & message) {
	if (!_chatSession) {
		LOG_DEBUG("cannot call sendMessage(): _chatSession is NULL");
		return 0;
	}

	_chatSession->removeAllIMContact();
	_chatSession->addIMContact(IMContact(*_account, sipAddress));
	_chatSession->sendMessage(message);

	return 1;
}
