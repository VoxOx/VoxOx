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
#include <filesessionmanager/ReceiveFileSession.h>

#include <filesessionmanager/IReceiveFileSession.h>

#include <util/SafeDelete.h>

ReceiveFileSession::ReceiveFileSession(UserProfile & userProfile,
	IReceiveFileSession * fileSessionImp)
	: Session(userProfile) {

	_currentFileSessionImp = fileSessionImp;
	_currentFileSessionImp->moduleFinishedEvent +=
		boost::bind(&ReceiveFileSession::moduleFinishedEventHandler, this, _1);
	_currentFileSessionImp->fileTransferEvent +=
		boost::bind(&ReceiveFileSession::fileTransferEventHandler, this, _1, _2, _3, _4);
	_currentFileSessionImp->fileTransferProgressionEvent +=
		boost::bind(&ReceiveFileSession::fileTransferProgressionEventHandler, this, _1, _2, _3, _4);
}

ReceiveFileSession::ReceiveFileSession(const ReceiveFileSession & receiveFileSession)	
	: Session(receiveFileSession) {
	if (receiveFileSession._currentFileSessionImp) {
		_currentFileSessionImp = receiveFileSession._currentFileSessionImp->clone();
		_currentFileSessionImp->moduleFinishedEvent +=
			boost::bind(&ReceiveFileSession::moduleFinishedEventHandler, this, _1);
		_currentFileSessionImp->fileTransferEvent +=
			boost::bind(&ReceiveFileSession::fileTransferEventHandler, this, _1, _2, _3, _4);
		_currentFileSessionImp->fileTransferProgressionEvent +=
			boost::bind(&ReceiveFileSession::fileTransferProgressionEventHandler, this, _1, _2, _3, _4);
	} else {
		_currentFileSessionImp = NULL;
	}
}

ReceiveFileSession::~ReceiveFileSession() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	OWSAFE_DELETE(_currentFileSessionImp);
}

void ReceiveFileSession::start() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->start();
	}
}

void ReceiveFileSession::pause() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->pause();
	}
}

void ReceiveFileSession::resume() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->resume();
	}
}

void ReceiveFileSession::stop() {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->stop();
	}
}

IMContact ReceiveFileSession::getIMContact() const {
	if (_currentFileSessionImp) {
		return _currentFileSessionImp->getIMContact();
	} else {
		return IMContact();
	}
}

void ReceiveFileSession::setFilePath(const std::string & path) {
	if (_currentFileSessionImp) {
		_currentFileSessionImp->setFilePath(path + File::getPathSeparator());
	}
}

std::string ReceiveFileSession::getFilePath() const {
	if (_currentFileSessionImp) {
		return _currentFileSessionImp->getFilePath();
	} else {
		return String::null;
	}
}

std::string ReceiveFileSession::getFileName() const {
	if (_currentFileSessionImp) {
		return _currentFileSessionImp->getFileName();
	} else {
		return String::null;
	}
}

unsigned ReceiveFileSession::getFileSize() const {
	if (_currentFileSessionImp) {
		return _currentFileSessionImp->getFileSize();
	} else {
		return 0;
	}
}

void ReceiveFileSession::moduleFinishedEventHandler(CoIpModule & sender) {
	moduleFinishedEvent(*this);
}

void ReceiveFileSession::fileTransferEventHandler(IFileSession & sender,
	IFileSession::IFileSessionEvent event, IMContact imContact, File sentFile) {

	_lastEvent = event;

	fileTransferEvent(*this, event, imContact, sentFile);
}

void ReceiveFileSession::fileTransferProgressionEventHandler(IFileSession & sender,
	IMContact imContact, File sentFile, int percentage) {

	fileTransferProgressionEvent(*this, imContact, sentFile, percentage);
}
