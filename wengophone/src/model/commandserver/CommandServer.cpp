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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "CommandServer.h"

#include "ContactInfo.h"

#include <model/chat/ChatHandler.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>
#include <model/WengoPhone.h>

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccountList.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMContactSet.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

CommandServer * CommandServer::_commandServerInstance = NULL;
const std::string CommandServer::QueryStatus = "1|s";
const std::string CommandServer::QueryCall = "1|o|call/";
const std::string CommandServer::QuerySms = "1|o|sms/";
const std::string CommandServer::QueryChat = "1|o|chat/";
const std::string CommandServer::QueryAddContact = "1|o|addc/";
const std::string CommandServer::QueryBringToFront = "1|o|bringToFront";

const std::string NICKNAME_STR = "pseudo";
const std::string SIP_STR = "sip";
const std::string FIRSTNAME_STR = "firstname";
const std::string LASTNAME_STR = "lastname";
const std::string COUNTRY_STR = "country";
const std::string CITY_STR = "city";
const std::string STATE_STR = "state";
const std::string GROUP_STR = "group";
const std::string WDEALSERVICETITLE_STR = "title";
const std::string URL_STR = "url";

CommandServer::CommandServer(WengoPhone & wengoPhone)
	: _wengoPhone(wengoPhone) {

	_serverSocket = new ServerSocket("127.0.0.1", Port);
	_serverSocket->serverStatusEvent += boost::bind(&CommandServer::serverStatusEventHandler, this, _1, _2);
	_serverSocket->connectionEvent += boost::bind(&CommandServer::connectionEventHandler, this, _1, _2);
	_serverSocket->incomingRequestEvent += boost::bind(&CommandServer::incomingRequestEventHandler, this, _1, _2, _3);
	_serverSocket->writeStatusEvent += boost::bind(&CommandServer::writeStatusEventHandler, this, _1, _2, _3);
	_serverSocket->init();
}

CommandServer::~CommandServer() {
	OWSAFE_DELETE(_serverSocket);
}

CommandServer & CommandServer::getInstance(WengoPhone & wengoPhone) {
	if (!_commandServerInstance) {
		_commandServerInstance = new CommandServer(wengoPhone);
	}

	return *_commandServerInstance;
}

void CommandServer::serverStatusEventHandler(ServerSocket & sender, ServerSocket::Error error) {
	if (error == ServerSocket::NoError) {
		LOG_DEBUG("connected");
	} else {
		LOG_WARN("not connected");
	}
}

void CommandServer::connectionEventHandler(ServerSocket & sender, const std::string & connectionId) {
	LOG_DEBUG("client connection=" + connectionId);
}

void CommandServer::incomingRequestEventHandler(ServerSocket & sender, const std::string & connectionId, const std::string & data) 
{
	LOG_DEBUG("incoming request connectionId=" + connectionId + " data=" + data);
	String query = String(data);

	if (query == QueryStatus) 
	{
		//Find the phoneline status and answer
		UserProfile * userprofile = _wengoPhone.getUserProfileHandler().getCurrentUserProfile();
		if (userprofile) {
			IPhoneLine * phoneLine = userprofile->getActivePhoneLine();
			if (phoneLine && phoneLine->isConnected()) {
				_serverSocket->writeToClient(connectionId, QueryStatus + "|1");
			} else {
				_serverSocket->writeToClient(connectionId, QueryStatus + "|0");
			}
		}

	} 
	else if (query == QueryBringToFront) 
	{
		bringMainWindowToFrontEvent();
	} 
	else if (query.beginsWith(QueryCall)) 
	{
		//Extract the number from query & place the call
		StringList l = query.split("/");

		if (l.size() == 2) 
		{
			LOG_DEBUG("call peer=" + l[1]);
			UserProfile * userprofile = _wengoPhone.getUserProfileHandler().getCurrentUserProfile();
			if (userprofile) 
			{
				IPhoneLine * phoneLine = userprofile->getActivePhoneLine();
				if (phoneLine && phoneLine->isConnected()) 
				{
					phoneLine->makeCall(l[1]);
					_serverSocket->writeToClient(connectionId, data + "|1");
					return;
				}
			}
		}

		_serverSocket->writeToClient(connectionId, data + "|0");
	} 
	else if (query.beginsWith(QuerySms)) 
	{
		LOG_WARN("not yet implemented");

	} 
	else if (query.contains(QueryChat)) 
	{
		UserProfile * userProfile = _wengoPhone.getUserProfileHandler().getCurrentUserProfile();
		if (userProfile) 
		{
			IPhoneLine * phoneLine = userProfile->getActivePhoneLine();
			if (phoneLine && phoneLine->isConnected()) 
			{
				// extract the nickname from 1|o|chat/pseudo=value&sip=value
				StringList l = query.split("/");
				std::string nickname;
				if (l.size() == 2) 
				{
					int sepPos = l[1].find("&");
					nickname = l[1].substr(7, sepPos - 7);
				}
				////

				// get THE Wengo account
				IMAccountList imAccountList = userProfile->getIMAccountManager().getIMAccountsOfProtocol(EnumIMProtocol::IMProtocolWengo);
				////

				// create the IMContactSet
				IMAccount *imAccount = userProfile->getIMAccountManager().getIMAccount( imAccountList.begin()->second );	//VOXOX - JRT - 2009.04.24 
				IMContact imContact(*imAccount, nickname);
				IMContactSet imContactSet;
				imContactSet.insert(imContact);
				////

				// create the chat session
				std::string temp = "";
				ChatHandler & chatHandler = userProfile->getChatHandler();
				chatHandler.createSession(imAccount->getKey(), imContactSet, IMChat::Chat, temp );	//VOXOX - JRT - 2009.04.24
				////
				
				OWSAFE_DELETE(imAccount);
			}
		}
		// failed
		_serverSocket->writeToClient(connectionId, QueryChat + "|0");

	} else if (query.beginsWith(QueryAddContact)) {

		UserProfile * userProfile = _wengoPhone.getUserProfileHandler().getCurrentUserProfile();
		if (userProfile) {

			ContactInfo contactInfo;
			String tmp = query.substr(QueryAddContact.size(), query.size() - 1);

			StringList args = tmp.split("&");
			for (unsigned i = 0; i < args.size(); i++) {

				String tmp = args[i];
				if (!tmp.size()) {
					continue;
				}

				StringList list = tmp.split("=");
				if ((!(list.size() == 2)) || list[0].empty()) {
					continue;
				}

				// remove the first and the last quote if any
				String value = list[1];

				if (list[0] == NICKNAME_STR) {
					contactInfo.wengoName = value;
				} else if (list[0] == SIP_STR) {
					contactInfo.sip = value;
				} else if (list[0] == FIRSTNAME_STR) {
					contactInfo.firstname = value;
				} else if (list[0] == LASTNAME_STR) {
					contactInfo.lastname = value;
				} else if (list[0] == COUNTRY_STR) {
					contactInfo.country = value;
				} else if (list[0] == CITY_STR) {
					contactInfo.city = value;
				} else if (list[0] == STATE_STR) {
					contactInfo.state = value;
				} else if (list[0] == GROUP_STR) {
					contactInfo.group = value;
				} else if (list[0] == WDEALSERVICETITLE_STR) {
					contactInfo.wdealServiceTitle = value;
				} else if (list[0] == URL_STR) {
					if (value.beginsWith("\"")) {
						value = value.substr(1, value.size() - 2);
					}
					contactInfo.website = value;
				}
			}

			showAddContactEvent(contactInfo);
		}

	} else {

		Config & config = ConfigManager::getInstance().getCurrentConfig();

		//"emulate" a http server. Needed for Flash sockets
		std::string tmp = "<?xml version=\"1.0\"?>\n"
			"<!DOCTYPE cross-domain-policy SYSTEM \"http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd\">\n"
			"<cross-domain-policy>\n";
		StringList domains = config.getCmdServerAuthorizedDomains();
		StringList::const_iterator it = domains.begin(), end = domains.end();
		for (;it!=end; ++it) {
			tmp += "<allow-access-from domain=\"" + *it + "\" to-ports=\"*\" />\n";
		}
		tmp += "<allow-access-from domain=\"localhost\" to-ports=\"*\" />\n"
				"</cross-domain-policy>";
		_serverSocket->writeToClient(connectionId, buildHttpForFlash(tmp));
	}
}

void CommandServer::writeStatusEventHandler(ServerSocket & sender, const std::string & writeId, ServerSocket::Error error) {
	if (error == ServerSocket::NoError) {
		LOG_DEBUG("writeId=" + writeId + " write success");
	} else {
		LOG_WARN("writeId=" + writeId + " write failed");
	}
}

std::string CommandServer::buildHttpForFlash(const std::string & xml) {

	std::string httpHeader =
		"HTTP/1.1 200 OK\n"
		"Date: Thu, 29 Jun 2006 11:02:50 GMT\n"
		"Server: Apache/2.0.46 (CentOS)\n"
		"Last-Modified: Thu, 29 Jun 2006 10:41:12 GMT\n"
		"Accept-Ranges: bytes\n"
		"Connection: close\n"
		"Content-Type: text/xml";

	//Add the xml Content-Length and the header separator
	httpHeader += "Content-Length: " + String::fromNumber(xml.size()) + "\n\n";

	return httpHeader + xml;
}
