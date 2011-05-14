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
 
#ifndef ADDIMACCOUNTCOMMAND_H
#define ADDIMACCOUNTCOMMAND_H

#include "Command.h"

#include <model/WengoPhone.h>

#include <iostream>

using namespace std;

class AddIMAccountCommand : public Command {
public:

	AddIMAccountCommand(WengoPhone & wengoPhone) 
		: Command(wengoPhone) {}

	virtual ~AddIMAccountCommand() {}

	void execute() {
		unsigned protocol;
		string login;
		string password;

		cout << "Adding an IMAccount: " << endl;
		cout << "Please enter the number of the desired protocol: " << endl;
		cout << "\t1: msn" << endl;
		cout << "\t2: yahoo!" << endl;
		cout << "\t3: jabber" << endl;
		cout << endl;
		cin >> protocol;
		cout << "Enter your login: ";
		cin >> login;
		cout << "Enter your password: ";
		cin >> password;
		
		switch (protocol) {
		case 1:
			_wengoPhone.addIMAccount(IMAccount(login, password,
				EnumIMProtocol::IMProtocolMSN));
			break;
		case 2:
			_wengoPhone.addIMAccount(IMAccount(login, password,
				EnumIMProtocol::IMProtocolYahoo));
			break;
		case 3:
			_wengoPhone.addIMAccount(IMAccount(login, password,
				EnumIMProtocol::IMProtocolJabber));
			break;
		
		default:
			cout << "Unknown protocol" << endl;
		}
	}

	const string & name() const {
		static string commandName = "addim";
		return commandName;
	}

	const string & desc() const {
		static string commandDesc = "Add an IMAccount";
		return commandDesc;
	}
};

#endif /*ADDIMACCOUNTCOMMAND_H*/
