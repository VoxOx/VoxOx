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

#include <model/WengoPhone.h>
#include <model/account/SipAccount.h>
#include <imwrapper/IMWrapperFactory.h>
#include <sipwrapper/SipWrapperFactory.h>
#include <PhApiFactory.h>
#include <cutil/global.h>

#include <util/Logger.h>

#if defined(CC_MSVC)
	#include <MemoryDump/MemoryDump.h>
#endif

#include <string>
#include <iostream>
using namespace std;

WengoPhone * wengoPhone = NULL;

void loginStateChangedEventHandler(SipAccount & sender, SipAccount::LoginState state) {
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

void noAccountAvailableEventHandler(UserProfile & sender) {
	sender.addSipAccount("jerome.leleu@wengo.fr", "b1234b", true);	
}

void proxyNeedsAuthenticationEventHandler(SipAccount & sender, const std::string & proxyUrl, unsigned proxyPort) {
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

void wrongProxyAuthenticationEventHandler(SipAccount & sender, const std::string & proxyUrl, unsigned proxyPort,
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

int main(int argc, char * argv[]) {

#if defined(CC_MSVC)
	new MemoryDump("WengoPhoneNG");
#endif

	//IM implementation
	IMWrapperFactory * imFactory = NULL;

	//SIP implementation
	SipWrapperFactory * sipFactory = NULL;

	PhApiFactory * phApiFactory = new PhApiFactory();
	sipFactory = phApiFactory;
	imFactory = phApiFactory;

	SipWrapperFactory::setFactory(sipFactory);
	IMWrapperFactory::setFactory(imFactory);

	wengoPhone = new WengoPhone();

	UserProfile & mProfile = wengoPhone->getCurrentUserProfile();
	mProfile.loginStateChangedEvent += &loginStateChangedEventHandler;
	mProfile.noAccountAvailableEvent += &noAccountAvailableEventHandler;
	mProfile.proxyNeedsAuthenticationEvent += &proxyNeedsAuthenticationEventHandler;
	mProfile.wrongProxyAuthenticationEvent += &wrongProxyAuthenticationEventHandler;

	wengoPhone->start();

	wengoPhone->join();

	return EXIT_SUCCESS;
}
