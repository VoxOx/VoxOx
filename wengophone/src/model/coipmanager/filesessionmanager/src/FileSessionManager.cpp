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
#include <filesessionmanager/FileSessionManager.h>

#include <filesessionmanager/SendFileSession.h>
#include <filesessionmanager/ISendFileSession.h>

#include "../implementation/phapi/include/phapifilesessionmanager/PhApiFileSessionManager.h"

#include <imwrapper/Account.h>
#include <imwrapper/IMContactSet.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>
#include <model/contactlist/Contact.h>

#include <util/Logger.h>

FileSessionManager::FileSessionManager(UserProfile & userProfile)
	: _userProfile(userProfile) {

	PhApiFileSessionManager * phapiManager = new PhApiFileSessionManager(_userProfile);
	phapiManager->newIReceiveFileSessionCreatedEvent += boost::bind(&FileSessionManager::newIReceiveFileSessionCreatedEventHandler, this, _1, _2);

	phapiManager->needUpgradeEvent += boost::bind(&FileSessionManager::needUpgradeEventHandler, this, _1);

	phapiManager->peerNeedsUpgradeEvent += boost::bind(&FileSessionManager::peerNeedsUpgradeEventHandler, this, _1, _2);

	_fileSessionManagerVector.push_back(phapiManager);

	
}

FileSessionManager::~FileSessionManager() {
	for (std::vector<IFileSessionManager *>::const_iterator it = _fileSessionManagerVector.begin();
		it != _fileSessionManagerVector.end();
		++it) {
		delete (*it);
	}
}

SendFileSession * FileSessionManager::createSendFileSession() {
	SendFileSession * newFileSession = new SendFileSession(*this, _userProfile);
	// HACK : Unique file transfer hack
	newFileSession->moduleFinishedEvent += boost::bind(&FileSessionManager::moduleFinishedEventHandler, this, _1);
	return newFileSession;
}

ISendFileSession * FileSessionManager::createFileSessionForContact(const std::string & contactId) {
	ISendFileSession * result = NULL;

//	Contact * contact = _userProfile.getContactList().getContact(contactId);
	Contact * contact = _userProfile.getContactList().getContactByUuid(contactId);	//VOXOX - JRT - 2009.04.20 - Use renamed method.
	if (contact) {

		ContactProfile contactProfile = *contact;
	
		for (std::vector<IFileSessionManager *>::const_iterator it = _fileSessionManagerVector.begin();
			it != _fileSessionManagerVector.end();
			++it) {
	
			for (IMContactSet::const_iterator contactIterator = contactProfile.getIMContactSet().begin();
				contactIterator != contactProfile.getIMContactSet().end();
				contactIterator++) {
		
				if ((*it)->isProtocolSupported((*contactIterator).getProtocol())) {
					result = (*it)->createSendFileSession();
					IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount((*contactIterator).getIMAccountId());
					result->setAccount(imAccount);
					IMContactSet imContactSet;
					imContactSet.insert(*contactIterator);
					result->setIMContactSet(imContactSet);
				}
			}
		}
	}

	if (!result) {
		LOG_ERROR("no FileSession implementation available");
	}

	return result;
}

void FileSessionManager::newIReceiveFileSessionCreatedEventHandler(IFileSessionManager & sender, IReceiveFileSession * iReceiveFileSession) {
	LOG_DEBUG("new IReceiveFileSession created event");

	ReceiveFileSession newFileSession(_userProfile, iReceiveFileSession);

	newReceiveFileSessionCreatedEvent(*this, newFileSession);
}

void FileSessionManager::needUpgradeEventHandler(IFileSessionManager & sender) {
	needUpgradeEvent(*this);
}

void FileSessionManager::peerNeedsUpgradeEventHandler(IFileSessionManager & sender, const std::string contactID) {
	peerNeedsUpgradeEvent(*this, contactID);
}

// HACK : Unique file transfer hack
void FileSessionManager::queueSession(SendFileSession * session) {
	_sendSessions.push(session);
	if(_sendSessions.size() == 1) {
		_sendSessions.front()->start();
	}
}

// HACK : Unique file transfer hack
void FileSessionManager::moduleFinishedEventHandler(CoIpModule & sender) {
	_sendSessions.pop();
	if(_sendSessions.size() > 0) {
		_sendSessions.front()->start();
	}
}
