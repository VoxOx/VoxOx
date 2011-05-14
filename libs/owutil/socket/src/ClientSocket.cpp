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

#include <socket/ClientSocket.h>

#include "SocketCommon.h"

#include <util/Logger.h>
#include <util/String.h>

Socket _mySock;

ClientSocket::ClientSocket() {
	_port = 0;
	_mySock = 0;
#ifdef OS_WINDOWS
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != NO_ERROR) {
		LOG_ERROR("Cannot initialize WinSock");
	}
#endif
}

ClientSocket::~ClientSocket() {
}

void ClientSocket::connect(const std::string & ip, int port) {
	_ip = ip;
	_port = port;
	Error error = NoError;

	if ((_mySock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		LOG_DEBUG("cannot create socket");
		connectionStatusEvent(*this, UnknownError);
		return;
	}

#ifdef SO_NOSIGPIPE
	int i = 1;
	setsockopt(_mySock, SOL_SOCKET, SO_NOSIGPIPE, &i, sizeof(i));
#endif

	struct sockaddr_in addr;
	addr.sin_port = (unsigned short) htons(_port);
	addr.sin_addr.s_addr = inet_addr(_ip.c_str());
	addr.sin_family = PF_INET;

	if (::connect(_mySock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		LOG_DEBUG("cannot connect to " + _ip);
		closesocket(_mySock);
		error = UnknownError;
	}

	connectionStatusEvent(*this, error);
}

bool ClientSocket::write(const std::string & data) {
	bool noError = false;

	int size = data.length() + 1;
	for (int i = 0; size; size -= i) {
		i = send(_mySock, data.c_str(), size, MSG_NOSIGNAL);
		if (i < 1) {
			break;
		}
	}

	if (size == 0) {
		fd_set rfds;
		struct timeval to;
		char buff[512];

		FD_ZERO(&rfds);
		FD_SET(_mySock, &rfds);
		to.tv_sec = 2;
		to.tv_usec = 0;
		int ret = select(_mySock + 1, &rfds, 0, 0, &to);
		if (ret && FD_ISSET(_mySock, &rfds)) {
			size = recv(_mySock, buff, sizeof(buff) - 1, 0);
			if (size) {
				buff[size] = 0;
				dataReceivedEvent(*this, buff);
			}
		}
	}
	else {
		noError = true;
	}

	return noError;
}

bool ClientSocket::disconnect() {
	closesocket(_mySock);
	return true;
}
