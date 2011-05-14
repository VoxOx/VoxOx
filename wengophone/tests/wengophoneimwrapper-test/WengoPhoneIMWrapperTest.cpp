/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include "WengoPhoneIMWrapperTest.h"

#include "AddWengoAccountCommand.h"
#include "AddIMAccountCommand.h"
#include "TalkToIMContactCommand.h"
#include "QuitCommand.h"

#include <model/WengoPhone.h>
#include <model/account/SipAccount.h>
#include <model/connect/ConnectHandler.h>
#include <imwrapper/IMWrapperFactory.h>
#include <GaimIMFactory.h>
#include <sipwrapper/SipWrapperFactory.h>
#include <PhApiFactory.h>

#include <util/Logger.h>

#include <string>
#include <iostream>
using namespace std;

WengoPhoneIMWrapperTest::WengoPhoneIMWrapperTest() {
	_running = true;
}

WengoPhoneIMWrapperTest::~WengoPhoneIMWrapperTest() {
	for (CommandList::const_iterator it = _commands.begin() ; it != _commands.end() ; it++) {
		delete (*it);
	}
}

void WengoPhoneIMWrapperTest::loginStateChangedEventHandler(SipAccount & sender, SipAccount::LoginState state) {
	switch (state) {
	case SipAccount::LoginStateConnected:
		LOG_DEBUG(sender.getUsername() + " is connected");
		break;
	case SipAccount::LoginStateDisconnected:
		LOG_DEBUG(sender.getUsername() + " has been disconnected");
		break;
	case SipAccount::LoginStateNetworkError:
		LOG_DEBUG("a network error occured while attempting to connect " + sender.getUsername());
		break;
	case SipAccount::LoginStatePasswordError:
		LOG_DEBUG("wrong login/password for " + sender.getUsername());
		break;
	case SipAccount::LoginStateReady:
		LOG_DEBUG(sender.getUsername() + " is ready to connect");
		break;
	default:
		LOG_FATAL("this state is unknown");
	}
}

void WengoPhoneIMWrapperTest::noAccountAvailableEventHandler(WengoPhone & sender) {
}

void WengoPhoneIMWrapperTest::proxyNeedsAuthenticationEventHandler(SipAccount & sender, const std::string & proxyUrl, unsigned proxyPort) {
	string url = proxyUrl;
	int port = proxyPort;
	string login;
	string password;
	string input;

	cout << "Proxy settings needed for " << proxyUrl << ":" << proxyPort << endl;
	cout << "Is proxy URL and proxy port valid? (y, n)" << endl;
	cin >> input;
	if ((input == "n") || (input == "N")) {
		cout << "Please enter proxy url: ";
		cin >> url;
		cout << "Please enter proxy port: ";
		cin >> port;
	}
	cout << "Please enter proxy login: ";
	cin >> login;
	cout << "Please enter proxy password: ";
	cin >> password;
	
	sender.setProxySettings(url, port, login, password);
}

void WengoPhoneIMWrapperTest::wrongProxyAuthenticationEventHandler(SipAccount & sender, const std::string & proxyUrl, unsigned proxyPort,
	const std::string & proxyLogin, const std::string proxyPassword) {
	string url = proxyUrl;
	int port = proxyPort;
	string login;
	string password;
	string input;

	cout << "Wrong proxy login/password. Please enter again parameter for " << proxyUrl << ":" << proxyPort << endl;
	cout << "Is proxy URL and proxy port valid? (y, n)" << endl;
	cin >> input;
	if ((input == "n") || (input == "N")) {
		cout << "Please enter proxy url: ";
		cin >> url;
		cout << "Please enter proxy port: ";
		cin >> port;
	}
	cout << "Please enter proxy login: ";
	cin >> login;
	cout << "Please enter proxy password: ";
	cin >> password;
	
	sender.setProxySettings(url, port, login, password);
}

void WengoPhoneIMWrapperTest::newIMAccountAddedEventHandler(WengoPhone & sender, IMAccount & imAccount) {
	sender.getConnectHandler().connect(imAccount);
}

void WengoPhoneIMWrapperTest::initWrappers() {
	//IM implementation
	IMWrapperFactory * imFactory = NULL;

	//SIP implementation
	SipWrapperFactory * sipFactory = NULL;


	PhApiFactory * phApiFactory = new PhApiFactory();
	sipFactory = phApiFactory;
	//imFactory = phApiFactory;
	imFactory = new GaimIMFactory();

	SipWrapperFactory::setFactory(sipFactory);
	IMWrapperFactory::setFactory(imFactory);
}

void WengoPhoneIMWrapperTest::initCommands(WengoPhone & wengoPhone) {
	_commands.push_back(new AddWengoAccountCommand(wengoPhone));
	_commands.push_back(new AddIMAccountCommand(wengoPhone));
	_commands.push_back(new TalkToIMContactCommand(wengoPhone));
	_commands.push_back(new QuitCommand(wengoPhone, *this));
}

int WengoPhoneIMWrapperTest::run() {
	string command;

	initWrappers();

	WengoPhone * wengoPhone = new WengoPhone();

	initCommands(*wengoPhone);

	wengoPhone->loginStateChangedEvent += 
		boost::bind(&WengoPhoneIMWrapperTest::loginStateChangedEventHandler, this, _1, _2);
	wengoPhone->noAccountAvailableEvent += 
		boost::bind(&WengoPhoneIMWrapperTest::noAccountAvailableEventHandler, this, _1);
	wengoPhone->proxyNeedsAuthenticationEvent += 
		boost::bind(&WengoPhoneIMWrapperTest::proxyNeedsAuthenticationEventHandler, this, _1, _2, _3);
	wengoPhone->wrongProxyAuthenticationEvent += 
		boost::bind(&WengoPhoneIMWrapperTest::wrongProxyAuthenticationEventHandler, this, _1, _2, _3, _4, _5);
	wengoPhone->newIMAccountAddedEvent += 
		boost::bind(&WengoPhoneIMWrapperTest::newIMAccountAddedEventHandler, this, _1, _2);

	wengoPhone->start();

	while (_running) {
		displayMenu();
		cout << "Enter the desired command: ";
		cin >> command;

		CommandList::const_iterator it = _commands.begin();

		while ((it != _commands.end()) && !(*it)->canHandle(command)) {
			it++;
		}

		if (it != _commands.end()) {
			(*it)->execute();
		} else {
			cout << "Command not recognized" << endl;
		}
	}

	return EXIT_SUCCESS;
}

void WengoPhoneIMWrapperTest::displayMenu() const {
	cout << endl << endl << endl << endl << endl;
	for (CommandList::const_iterator it = _commands.begin() ; it != _commands.end() ; it++) {
		cout << "- " << (*it)->name() << ": " << (*it)->desc() << endl;
	}
	cout << endl;
}
