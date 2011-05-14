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
 
#ifndef ADDWENGOACCOUNTCOMMAND_H
#define ADDWENGOACCOUNTCOMMAND_H

#include "Command.h"

#include <model/WengoPhone.h>

#include <iostream>

using namespace std;

class AddWengoAccountCommand : public Command {
public:
	
	AddWengoAccountCommand(WengoPhone & wengoPhone) 
		: Command(wengoPhone) {}

	virtual ~AddWengoAccountCommand() {}

	void execute() {
		string login;
		string password;

		cout << "Adding a Wengo account: " << endl;
		cout << "Please enter the Wengo login: ";
		cin >> login;
		cout << "Enter the Wengo password: ";
		cin >> password;

		_wengoPhone.addSipAccount(login, password, true);
	}

	/**
	 * @return the Command name.
	 */
	const string & name() const {
		static string commandName = "addsip";
		return commandName;
	}

	const string & desc() const {
		static string commandDesc = "Add a Wengo account";
		return commandDesc;
	}

};

#endif /*ADDWENGOACCOUNTCOMMAND_H*/
