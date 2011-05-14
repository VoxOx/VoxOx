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

#include <socket/ServerSocket.h>

#include "SocketCommon.h"

#include <util/Logger.h>
#include <util/String.h>

#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0
#endif

Socket _mainSock;
static std::list<Socket> _clientSockList;
typedef std::list<Socket>::iterator SockListIterator;

ServerSocket::ServerSocket(const std::string & listeningIp, int port)
	: _listeningIp(listeningIp),
	_port(port) {
	_started = false;
}

void ServerSocket::init() {
	if (createMainListeningSocket()) {
		_started = true;
		start();
	}
}

bool ServerSocket::createMainListeningSocket() {
	struct sockaddr_in raddr;

	memset(&raddr, 0, sizeof(struct sockaddr_in));
	if (_listeningIp.empty()) {
		raddr.sin_addr.s_addr = htons(INADDR_ANY);
	}
	else {
		raddr.sin_addr.s_addr = inet_addr(_listeningIp.c_str());
	}
	raddr.sin_port = htons(_port);
	raddr.sin_family = AF_INET;

	_mainSock = 0;
	if ((_mainSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		LOG_DEBUG("cannot create main socket");
		return false;
	}

	int option = 1;
#ifdef OS_WINDOWS
	//FIXME don't know if this is the right way to do it
	setsockopt(_mainSock, SOL_SOCKET, SO_REUSEADDR, (const char *) &option, sizeof(int));
#else
	setsockopt(_mainSock, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));
#endif

	if (bind(_mainSock, (struct sockaddr *) &raddr, sizeof (raddr)) < 0) {
		perror("bind");
		LOG_DEBUG("cannot bind main socket");
		closesocket(_mainSock);
		_mainSock = 0;
		return false;
	}

	listen(_mainSock, 5);

	return true;
}

ServerSocket::~ServerSocket() {
	if (_started) {
		_started = false;

		if (_mainSock) {
			closesocket(_mainSock);
		}

		SockListIterator it;
		for (it = _clientSockList.begin(); it != _clientSockList.end(); it++) {
			closesocket(*it);
			_clientSockList.erase(it);
		}
	}
}

bool ServerSocket::closeAndRemoveFromList(const std::string & connectionId) {
	Socket sockId = (Socket) String(connectionId).toInteger();

	SockListIterator it;
	for (it = _clientSockList.begin(); it != _clientSockList.end(); it++) {
		if (*it == sockId) {
			closesocket(*it);
			_clientSockList.erase(it);
			return true;
		}
	}

	return false;
}

bool ServerSocket::checkConnectionId(const std::string & connectionId) {
	Socket sockId = (Socket) String(connectionId).toInteger();

	SockListIterator it;
	for (it = _clientSockList.begin(); it != _clientSockList.end(); it++) {
		if (*it == sockId) {
			return true;
		}
	}

	return false;
}

bool ServerSocket::writeToClient(const std::string & connectionId, const std::string & data) {
	Socket sockId = (Socket) String(connectionId).toInteger();
	Error error = UnknownError;

	if (checkConnectionId(connectionId)) {
		int size = data.length() + 1;
		for (int i = 0; size; size -= i) {
			i = send(sockId, data.c_str(), size, MSG_NOSIGNAL);
			if (i < 1) {
				break;
			}
		}

		if (size == 0) {
			error = NoError;
		}
	}

	closeAndRemoveFromList(connectionId);
	writeStatusEvent(*this, connectionId, error);
	return (error == NoError);
}

int ServerSocket::getHighestSocket() {
	int highest = 0;

	SockListIterator it;
	for (it = _clientSockList.begin(); it != _clientSockList.end(); it++) {
		if (((int) *it) > highest) {
			highest = (int) *it;
		}
	}

	if (((int) _mainSock) > highest) {
		highest = _mainSock;
	}

	return highest;
}

int ServerSocket::getRequest(int sockId, char * buff, unsigned buffsize) {
	struct timeval timeout;
	fd_set rfds;
	unsigned nbytes = 0;
	Socket sock = (Socket) sockId;

	memset(buff, 0, buffsize);

	while (true) {
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(sockId, &rfds);

		int ret = select(sock + 1, &rfds, 0, 0, &timeout);

		if (ret <= 0) {
			LOG_WARN("select error");
			return -1;
		}

		if (FD_ISSET(sock, &rfds)) {
			ret = recv(sock, buff + nbytes, 1, 0);

			if (ret <= 0) {
				LOG_WARN("recv error");
				return -1;
			} else {
				nbytes += ret;
			}

			if (nbytes == buffsize) {
				return nbytes;
			}

			if (strncmp("\0", buff + nbytes - 1, 1) == 0) {
				break;
			}
		}
	}

	return nbytes;
}

void ServerSocket::run() {
	Socket sock;
	struct sockaddr from;
	socklen_t fromlen = sizeof(from);
	fd_set rfds;
	//struct timeval to;
	char buff[512];

	serverStatusEvent(*this, NoError);

	while (_started) {
		SockListIterator it;

		FD_ZERO(&rfds);
		FD_SET(_mainSock, &rfds);

		for (it = _clientSockList.begin(); it != _clientSockList.end(); it++) {
			FD_SET(*it, &rfds);
		}

		int err = select(getHighestSocket() + 1, &rfds, 0, 0, 0);

		if (err && FD_ISSET(_mainSock, &rfds)) {
			sock = accept(_mainSock, &from, &fromlen);
			if (sock > 0) {
				_clientSockList.push_back(sock);
				connectionEvent(*this, String::fromNumber((int)sock));
			}
		}
		else if (err) {
			SockListIterator it;
			for (it = _clientSockList.begin(); it != _clientSockList.end(); it++) {
				if (FD_ISSET(*it, &rfds)) {
					if (getRequest(*it, buff, sizeof(buff)) == -1) {
						LOG_WARN("error reading socket");
						closesocket(*it);
						_clientSockList.erase(it);
					} else {
						incomingRequestEvent(*this, String::fromNumber((int) *it), std::string(buff));
					}
					break;
				}
			}
		}
	}

	serverStatusEvent(*this, UnknownError);
}
