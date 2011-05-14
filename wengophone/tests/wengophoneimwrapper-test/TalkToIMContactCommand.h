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
 
#ifndef TALKTOIMCONTACTCOMMAND_H
#define TALKTOIMCONTACTCOMMAND_H

#include "Command.h"

#include <model/WengoPhone.h>
#include <model/chat/ChatHandler.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMAccount.h>

#include <iostream>

using namespace std;

class TalkToIMContactCommand : public Command {
public:

	TalkToIMContactCommand(WengoPhone & wengoPhone) 
		: Command(wengoPhone) {}

	virtual ~TalkToIMContactCommand() {}

	void execute() {
		IMAccountHandler & imAccountHandler = _wengoPhone.getIMAccountHandler();
		unsigned account, i;
		IMAccountHandler::const_iterator it;
		string contactId;

		cout << "Choose the IMAccount to use: " << endl;
		for (it = imAccountHandler.begin(), i = 0 ; it != imAccountHandler.end() ; it++, i++) {
			cout << i << " - " <<  (*it).getLogin() << endl;
		}
		cin >> account;	

		if (account > imAccountHandler.size()) {
			cout << "Wrong number" << endl;
			return;
		}

		it = imAccountHandler.begin();
		for (unsigned j = 0 ; j < account ; j++) {
			it++;
		}

		_imAccount = new IMAccount(*it);

		IMContactSet imContactSet;
		string command;
		char buffer[512];
		cout << "Add contacts to the chat. Type 'start' to start the chat: " << endl;
		while (command != "start") {

			cout << "contact id: ";
			cin.getline(buffer, sizeof(buffer));
			command = string(buffer);

			if (!command.empty() && command != "start")
			{
				imContactSet.insert(IMContact(*_imAccount, command));
			}
		}

		_chatFinished = false;
		_wengoPhone.getChatHandler().newIMChatSessionCreatedEvent +=
			boost::bind(&TalkToIMContactCommand::newIMChatSessionCreatedEventHandler, this, _1, _2);
		_wengoPhone.getChatHandler().createSession(*_imAccount, imContactSet);

		while (!_chatFinished) {
			Thread::msleep(100);
		}
	}

	const string & name() const {
		static string commandName = "talk";
		return commandName;
	}

	const string & desc() const {
		static string commandDesc = "Talk to an IMContact";
		return commandDesc;
	}

	void newIMChatSessionCreatedEventHandler(ChatHandler & chatHandler, IMChatSession & imChatSession) {
		string contactId;
		string command;
		char buffer[512];

		imChatSession.messageReceivedEvent +=
			boost::bind(&TalkToIMContactCommand::messageReceivedEventHandler, this, _1, _2, _3);
		imChatSession.statusMessageReceivedEvent +=
			boost::bind(&TalkToIMContactCommand::statusMessageReceivedEventHandler, this, _1, _2, _3);
		
		cout << "Commands are: quit, add" << endl;
		
		//while (command != "quit") {
		//	cout << "> ";
		//	cin.getline(buffer, sizeof(buffer));
		//	command = string(buffer);

		//	if (command == "add") {
		//		cout << "Please enter the contact id to talk to: ";
		//		cin >> contactId;
		//
		//		imChatSession.addIMContact(IMContact(*_imAccount, contactId));

		//		continue;
		//	}

		//	if (command != "quit") {
		//		imChatSession.sendMessage(command);
		//	}
		//}
		//_chatFinished = true;
	}

	void messageReceivedEventHandler(IMChatSession & sender, const IMContact & from, const std::string & message) {
		cout << "Message received from " << from.getContactId() << ": " << message << endl;
	}

	void statusMessageReceivedEventHandler(IMChatSession & sender, IMChat::StatusMessage status, const std::string & message) {
		cout << "** Status message: " << (int)status << " " << message << endl;
	}

	void contactAddedEventHandler(IMChatSession & sender, const IMContact & imContact) {
		cout << "** Contact added to IMChatSession: " << imContact.getContactId() << endl;
	}

	void contactRemovedEventHandler(IMChatSession & sender, const IMContact & imContact) {
		cout << "** Contact removed from IMChatSession: " << imContact.getContactId() << endl;
	}

private:

	IMAccount * _imAccount;

	bool _chatFinished;

};

#endif /*TALKTOIMCONTACTCOMMAND_H*/
