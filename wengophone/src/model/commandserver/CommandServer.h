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

#ifndef OWCOMMANDSERVER_H
#define OWCOMMANDSERVER_H

#include <socket/ServerSocket.h>
#include <util/Event.h>

class WengoPhone;
class ContactInfo;

/**
 * Command server. Listen for local connections,
 * implements a simple command protocol.
 *
 * Syntax for query string is:
 * <version>|s
 *          |o|<command>[/<params>]
 * - <version> is the version of the communication protocol ("1" for now)
 * - "s" stands for "status"
 * - "o" stands for "order"
 * - <command> is a string
 * - <params> are optional params for <command>
 *
 * @author Mathieu Stute
 */
class CommandServer {
public:
	static const int Port = 25902;

	static const std::string QueryStatus;

	static const std::string QueryCall;

	static const std::string QuerySms;

	static const std::string QueryChat;

	static const std::string QueryAddContact;

	static const std::string QueryBringToFront;

	/**
	 * A contact has to be added to the contact list.
	 *
	 * work around a f*c*i*g VS 2003 bug that produces an INTERNAL COMPILER ERROR.
	 *
	 * @param contactInfo contact informations (firstname, lastname, SIP address...)
	 */
	Event<void (ContactInfo contactInfo)> showAddContactEvent;

	Event<void (void)> bringMainWindowToFrontEvent;

	CommandServer(WengoPhone & wengoPhone);

	~CommandServer();

	static CommandServer & getInstance(WengoPhone & wengoPhone);

private:

	static std::string buildHttpForFlash(const std::string & xml);

	/**
	 * @see ServerSocket::serverStatusEvent
	 */
	void serverStatusEventHandler(ServerSocket & sender, ServerSocket::Error error);

	/**
	 * @see ServerSocket::connectionEvent
	 */
	void connectionEventHandler(ServerSocket & sender, const std::string & connectionId);

	/**
	 * @see ServerSocket::incomingRequestEvent
	 */
	void incomingRequestEventHandler(ServerSocket & sender, const std::string & connectionId, const std::string & data);

	/**
	 * @see ServerSocket::writeStatusEvent
	 */
	void writeStatusEventHandler(ServerSocket & sender, const std::string & writeId, ServerSocket::Error error);

	ServerSocket * _serverSocket;

	static CommandServer * _commandServerInstance;

	WengoPhone & _wengoPhone;
};

#endif	//OWCOMMANDSERVER_H
