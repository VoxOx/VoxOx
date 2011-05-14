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

#ifndef OWSOCKETCLIENT_H
#define OWSOCKETCLIENT_H

#include <socket/owsocketdll.h>

#include <util/Event.h>

#include <string>

/**
 * Connect to a ServerSocket.
 *
 * @author Julien Bossart
 * @author Mathieu Stute
 */
class ClientSocket {
public:

	//To finish
	enum Error {
		/** No error. */
		NoError,

		/** Unknow error. */
		UnknownError
	};

	/**
	 * Connection status event.
	 *
	 * @param sender this class
	 * @param error error status
	 */
	Event<void (ClientSocket & sender, Error error)> connectionStatusEvent;

	/**
	 * Data has been received.
	 *
	 * @param sender this class
	 * @param data received data
	 */
	Event<void (ClientSocket & sender, const std::string & data)> dataReceivedEvent;

	OWSOCKET_API ClientSocket();

	OWSOCKET_API ~ClientSocket();

	/**
	 * Connects to a server.
	 *
	 * @param ip remote ip
	 * @param port remote port
	 */
	OWSOCKET_API void connect(const std::string & ip, int port);

	/**
	 * Sends data to the server.
	 *
	 * @param data data to be written
	 * @return true if writing has succeeded
	 */
	OWSOCKET_API bool write(const std::string & data);

	/**
	 * Sends data to the server.
	 *
	 * @param data data to be written
	 * @return true if writing has succeeded
	 */
	OWSOCKET_API bool disconnect();

private:

	std::string _ip;

	int _port;
};

#endif	//OWSOCKETCLIENT_H
