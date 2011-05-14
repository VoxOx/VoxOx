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

#ifndef WENGOPHONEIMWRAPPERTEST_H
#define WENGOPHONEIMWRAPPERTEST_H

#include <model/account/SipAccount.h>

#include <string>
#include <vector>

class WengoPhone;
class Command;
class IMAccount;

class WengoPhoneIMWrapperTest {
public:

	WengoPhoneIMWrapperTest();

	~WengoPhoneIMWrapperTest();

	/**
	 * Entry point of this class.
	 */
	int run();

	void quit() {
		_running = false;
	}

private:

	void loginStateChangedEventHandler(SipAccount & sender, SipAccount::LoginState state);

	void noAccountAvailableEventHandler(WengoPhone & sender);

	void proxyNeedsAuthenticationEventHandler(SipAccount & sender, const std::string & proxyUrl, unsigned proxyPort);

	void wrongProxyAuthenticationEventHandler(SipAccount & sender, const std::string & proxyUrl, unsigned proxyPort,
		const std::string & proxyLogin, const std::string proxyPassword);

	void newIMAccountAddedEventHandler(WengoPhone & sender, IMAccount & imAccount);

	void initWrappers();

	void initCommands(WengoPhone & wengoPhone);

	void displayMenu() const;

	bool _running;

	typedef std::vector< Command * > CommandList;
	CommandList _commands;

};

#endif /*WENGOPHONEIMWRAPPERTEST_H*/
