/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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
#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "CommandClient.h"

#include "CommandServer.h"

#include <util/SafeDelete.h>

#include <cutil/global.h>

#include <socket/ClientSocket.h>

#ifdef OS_WINDOWS
#include <windows.h>
#endif

CommandClient::CommandClient() {
	_socket = new ClientSocket();
}

CommandClient::~CommandClient() {
#ifdef OS_WINDOWS
	// FIXME: This Sleep() has been copied from main.cpp when CommandClient was
	// created. Not sure it's really necessary.
	Sleep(1000);
	WSACleanup();
#endif
	OWSAFE_DELETE(_socket);
}

bool CommandClient::connect() {
	// FIXME: Modify ClientSocket::connect() to return a bool
	_socket->connect("127.0.0.1", CommandServer::Port);
	return true;
}

void CommandClient::call(const std::string& number) {
	_socket->write(CommandServer::QueryCall + number);
}

void CommandClient::bringToFront() {
	_socket->write(CommandServer::QueryBringToFront);
}
