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

#ifndef OWXPCOMIMHANDLER_H
#define OWXPCOMIMHANDLER_H

#include <Listener.h>

#include <imwrapper/EnumPresenceState.h>

#include <util/NonCopyable.h>

#include <string>

class IMAccount;
class IMContact;
class IMConnect;
class IMChat;
class IMChatSession;
class IMPresence;

class XPCOMIMHandler : NonCopyable {
public:

	/** Singleton. */
	static XPCOMIMHandler & getInstance() {
		static XPCOMIMHandler imHandler;

		return imHandler;
	}

	XPCOMIMHandler() {
		_account = NULL;
		_connect = NULL;
		_chat = NULL;
		_presence = NULL;
	}

	void setWengoLoginPassword(const std::string & wengoLogin, const std::string & wengoPassword) {
		_wengoLogin = wengoLogin;
		_wengoPassword = wengoPassword;
	}

	void connected();

	void subscribeToPresenceOf(const std::string & sipAddress);

	void publishMyPresence(EnumPresenceState::PresenceState state, const std::string & note);

	int sendChatMessage(const std::string & sipAddress, const std::string & message);

private:

	void messageReceivedEventHandler(IMChatSession & sender, const IMContact & from, const std::string & message);

	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	void presenceStateChangedEventHandler(IMPresence & sender, EnumPresenceState::PresenceState state,
					const std::string & note, const std::string & statusMessage, const std::string & from);

	std::string _wengoLogin;
	std::string _wengoPassword;

	IMAccount * _account;
	IMConnect * _connect;
	IMChat * _chat;
	IMChatSession * _chatSession;
	IMPresence * _presence;
};

#endif	//OWXPCOMIMHANDLER_H
