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
#include <filesessionmanager/SendFileSession.h>

#include <filesessionmanager/FileSessionManager.h>
#include <filesessionmanager/ISendFileSession.h>

#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactList.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMContactSet.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

SendFileSession::SendFileSession(FileSessionManager & fileSessionManager, UserProfile & userProfile)
	: Session(userProfile),
	_fileSessionManager(fileSessionManager) {
	_currentFileSessionImp = NULL;
}

SendFileSession::SendFileSession(const SendFileSession & sendFileSession) 
	: Session(sendFileSession),
	_fileSessionManager(sendFileSession._fileSessionManager) {

	_fileVector = sendFileSession._fileVector;

	if (sendFileSession._currentFileSessionImp) {
		_currentFileSessionImp = sendFileSession._currentFileSessionImp->clone();
		_currentFileSessionImp->moduleFinishedEvent +=
			boost::bind(&SendFileSession::moduleFinishedEventHandler, this, _1);
		_currentFileSessionImp->fileTransferEvent +=
			boost::bind(&SendFileSession::fileTransferEventHandler, this, _1, _2, _3, _4);
		_currentFileSessionImp->fileTransferProgressionEvent +=
			boost::bind(&SendFileSession::fileTransferProgressionEventHandler, this, _1, _2, _3, _4);
	} else {
		_currentFileSessionImp = NULL;
	}
}

SendFileSession::~SendFileSession() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	OWSAFE_DELETE(_currentFileSessionImp);
}

void SendFileSession::start() {
	// A FileSession will be created for each different protocols.

	// Check if a FileSession is not currently running.
	if (_currentFileSessionImp) {
		return;
	}

	// Check if we have files to send and contacts to send to
	if (_fileVector.empty() || _contactIdList.empty()) {
		moduleFinishedEvent(*this);
		return;
	}

	bool done = false;
	while (!done) {
		std::string contact = *_contactIdList.begin();
		ISendFileSession * fileSessionImp =
			_fileSessionManager.createFileSessionForContact(contact);

		if (fileSessionImp) {
			done = true;
			_currentFileSessionImp = fileSessionImp;
			_currentFileSessionImp->moduleFinishedEvent +=
				boost::bind(&SendFileSession::moduleFinishedEventHandler, this, _1);
			_currentFileSessionImp->fileTransferEvent +=
				boost::bind(&SendFileSession::fileTransferEventHandler, this, _1, _2, _3, _4);
			_currentFileSessionImp->fileTransferProgressionEvent +=
				boost::bind(&SendFileSession::fileTransferProgressionEventHandler, this, _1, _2, _3, _4);

			_currentFileSessionImp->setFileList(_fileVector);

			_currentFileSessionImp->start();
		} else {
			LOG_ERROR("cannot find any account usable with this contact."
				" Removing contact from list and checking next one.");
			_contactIdList.erase(_contactIdList.begin());
			done = _contactIdList.empty();
			if (done) {
				moduleFinishedEvent(*this);
				return;
			}
		}
	}
}

void SendFileSession::pause() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->pause();
	}
}

void SendFileSession::resume() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->resume();
	}
}

void SendFileSession::stop() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->stop();
	}
}

void SendFileSession::addFile(const File & file) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	_fileVector.push_back(file);
}

void SendFileSession::moduleFinishedEventHandler(CoIpModule & sender) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	//Removing Contacts from the _contactIdList
	ISendFileSession & fileSessionImp = dynamic_cast<ISendFileSession &>(sender);
	IMContactSet imContactSet = fileSessionImp.getIMContactSet();
	for (IMContactSet::const_iterator imIt = imContactSet.begin(); imIt != imContactSet.end(); ++imIt) {
		Contact * contact = _userProfile.getContactList().findContactThatOwns(*imIt);
		if (contact) {
//			std::string contactId = contact->getUUID();
			std::string contactId = contact->getKey();	//VOXOX - JRT - 2009.04.28 
			for (StringList::iterator strIt = _contactIdList.begin(); strIt != _contactIdList.end(); ++strIt) {
				if (contactId == (*strIt)) {
					_contactIdList.erase(strIt);
					break;
				}
			}
		}
	}
	////

	OWSAFE_DELETE(_currentFileSessionImp);
	start(); // Recall start to launch the next FileSession.
}


void SendFileSession::fileTransferEventHandler(IFileSession & sender,
	IFileSession::IFileSessionEvent event, IMContact imContact, File sentFile) {

	fileTransferEvent(*this, event, imContact, sentFile);
}

void SendFileSession::fileTransferProgressionEventHandler(IFileSession & sender,
	IMContact imContact, File sentFile, int percentage) {

	fileTransferProgressionEvent(*this, imContact, sentFile, percentage);
}
