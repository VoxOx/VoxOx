/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "../include/phapifilesessionmanager/PhApiReceiveFileSession.h"

#include <PhApiSFPEvent.h>

#include <util/Logger.h>
#include <util/File.h>

PhApiReceiveFileSession::PhApiReceiveFileSession(){}

PhApiReceiveFileSession::PhApiReceiveFileSession(int callID, std::string contactID, std::string fileName, unsigned int fileSize) 
: IReceiveFileSession() {
	_imContact = IMContact(EnumIMProtocol::IMProtocolWengo, contactID);
	_fileName = fileName;
	_fileSize = fileSize;
	_currentCallID = callID;

	PhApiSFPEvent::transferCancelledEvent += boost::bind(&PhApiReceiveFileSession::transferCancelledEventHandler, this, _1, _2, _3, _4, _5);
	PhApiSFPEvent::transferCancelledByPeerEvent += boost::bind(&PhApiReceiveFileSession::transferCancelledByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::sendingFileBeginEvent += boost::bind(&PhApiReceiveFileSession::sendingFileBeginEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFinishedEvent += boost::bind(&PhApiReceiveFileSession::transferFromPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFinishedEvent += boost::bind(&PhApiReceiveFileSession::transferToPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFailedEvent += boost::bind(&PhApiReceiveFileSession::transferFromPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFailedEvent += boost::bind(&PhApiReceiveFileSession::transferToPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferProgressionEvent += boost::bind(&PhApiReceiveFileSession::transferProgressionEventHandler, this, _1, _2, _3);
	PhApiSFPEvent::transferPausedByPeerEvent += boost::bind(&PhApiReceiveFileSession::transferPausedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferPausedEvent += boost::bind(&PhApiReceiveFileSession::transferPausedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedByPeerEvent += boost::bind(&PhApiReceiveFileSession::transferResumedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedEvent += boost::bind(&PhApiReceiveFileSession::transferResumedEventHandler, this, _1, _2, _3, _4, _5, _6);
}

PhApiReceiveFileSession::PhApiReceiveFileSession(const PhApiReceiveFileSession & phApiReceiveFileSession)
: IReceiveFileSession(phApiReceiveFileSession) {
	_imContact = phApiReceiveFileSession._imContact;
	_fileName = phApiReceiveFileSession._fileName;
	_fileSize = phApiReceiveFileSession._fileSize;
	_currentCallID = phApiReceiveFileSession._currentCallID;

	PhApiSFPEvent::transferCancelledEvent += boost::bind(&PhApiReceiveFileSession::transferCancelledEventHandler, this, _1, _2, _3, _4, _5);
	PhApiSFPEvent::transferCancelledByPeerEvent += boost::bind(&PhApiReceiveFileSession::transferCancelledByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::sendingFileBeginEvent += boost::bind(&PhApiReceiveFileSession::sendingFileBeginEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFinishedEvent += boost::bind(&PhApiReceiveFileSession::transferFromPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFinishedEvent += boost::bind(&PhApiReceiveFileSession::transferToPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFailedEvent += boost::bind(&PhApiReceiveFileSession::transferFromPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFailedEvent += boost::bind(&PhApiReceiveFileSession::transferToPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferProgressionEvent += boost::bind(&PhApiReceiveFileSession::transferProgressionEventHandler, this, _1, _2, _3);
	PhApiSFPEvent::transferPausedByPeerEvent += boost::bind(&PhApiReceiveFileSession::transferPausedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferPausedEvent += boost::bind(&PhApiReceiveFileSession::transferPausedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedByPeerEvent += boost::bind(&PhApiReceiveFileSession::transferResumedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedEvent += boost::bind(&PhApiReceiveFileSession::transferResumedEventHandler, this, _1, _2, _3, _4, _5, _6);
}

PhApiReceiveFileSession * PhApiReceiveFileSession::clone() const {
	return new PhApiReceiveFileSession(*this);
}

PhApiReceiveFileSession::~PhApiReceiveFileSession(){}

void PhApiReceiveFileSession::start() {
	LOG_DEBUG("starting file transfer");
	PhApiSFPWrapper::getInstance().receiveFile(_currentCallID, File::convertPathSeparators(_filePath+_fileName));
	LOG_DEBUG("end of transfer start");
}

void PhApiReceiveFileSession::pause(){
	LOG_DEBUG("pausing file transfer");
	if(_currentCallID > 0){
		PhApiSFPWrapper::getInstance().pauseTransfer(_currentCallID);
	}
	LOG_DEBUG("end of transfer pause");
}

void PhApiReceiveFileSession::resume(){
	LOG_DEBUG("resuming file transfer");
	if(_currentCallID > 0){
		PhApiSFPWrapper::getInstance().resumeTransfer(_currentCallID);
	}
	LOG_DEBUG("end of transfer resume");
}

void PhApiReceiveFileSession::stop(){
	LOG_DEBUG("stopping file transfer");
	if(_currentCallID > 0){
		PhApiSFPWrapper::getInstance().cancelTransfer(_currentCallID);
	}
	LOG_DEBUG("end of transfer stop");
}

void PhApiReceiveFileSession::transferCancelledEventHandler(PhApiSFPWrapper & sender, int callID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID){
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferCancelled, _imContact, File(getFilePath()+getFileName()));
		moduleFinishedEvent(*this);
	}
}

void PhApiReceiveFileSession::transferCancelledByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferCancelledByPeer, _imContact, File(getFilePath()+getFileName()));
		moduleFinishedEvent(*this);
	}
}

void PhApiReceiveFileSession::sendingFileBeginEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferBegan, _imContact, File(getFilePath()+getFileName()));
	}
}

void PhApiReceiveFileSession::transferFromPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFinished, _imContact, File(getFilePath()+getFileName()));
		moduleFinishedEvent(*this);
	}
}

void PhApiReceiveFileSession::transferToPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFinished, _imContact, File(getFilePath()+getFileName()));
		moduleFinishedEvent(*this);
	}
}

void PhApiReceiveFileSession::transferFromPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFailed, _imContact, File(getFilePath()+getFileName()));
		moduleFinishedEvent(*this);
	}
}

void PhApiReceiveFileSession::transferToPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFailed, _imContact, File(getFilePath()+getFileName()));
		moduleFinishedEvent(*this);
	}
}

void PhApiReceiveFileSession::transferProgressionEventHandler(PhApiSFPWrapper & sender, int callID, int percentage){
	if(callID == _currentCallID){
		fileTransferProgressionEvent(*this, _imContact, File(getFilePath()+getFileName()), percentage);
	}
}

void PhApiReceiveFileSession::transferPausedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferPausedByPeer, _imContact, File(getFilePath()+getFileName()));
	}
}

void PhApiReceiveFileSession::transferPausedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferPaused, _imContact, File(getFilePath()+getFileName()));
	}
}

void PhApiReceiveFileSession::transferResumedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferResumedByPeer, _imContact, File(getFilePath()+getFileName()));
	}
}

void PhApiReceiveFileSession::transferResumedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferResumed, _imContact, File(getFilePath()+getFileName()));
	}
}
