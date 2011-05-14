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
#include "../include/phapifilesessionmanager/PhApiSendFileSession.h"

#include <PhApiSFPEvent.h>
#include <PhApiSFPWrapper.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMContact.h>

#include <util/Logger.h>
#include <util/File.h>

PhApiSendFileSession::PhApiSendFileSession(UserProfile & userProfile) 
: ISendFileSession(), _userProfile(userProfile) {
	_initialized = false;
	_currentCallID = -1;

	PhApiSFPEvent::inviteToTransferEvent += boost::bind(&PhApiSendFileSession::inviteToTransferEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::waitingForAnswerEvent += boost::bind(&PhApiSendFileSession::waitingForAnswerEventHandler, this, _1, _2, _3);
	PhApiSFPEvent::transferCancelledEvent += boost::bind(&PhApiSendFileSession::transferCancelledEventHandler, this, _1, _2, _3, _4, _5);
	PhApiSFPEvent::transferCancelledByPeerEvent += boost::bind(&PhApiSendFileSession::transferCancelledByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::sendingFileBeginEvent += boost::bind(&PhApiSendFileSession::sendingFileBeginEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFinishedEvent += boost::bind(&PhApiSendFileSession::transferFromPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFinishedEvent += boost::bind(&PhApiSendFileSession::transferToPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFailedEvent += boost::bind(&PhApiSendFileSession::transferFromPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFailedEvent += boost::bind(&PhApiSendFileSession::transferToPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferProgressionEvent += boost::bind(&PhApiSendFileSession::transferProgressionEventHandler, this, _1, _2, _3);
	PhApiSFPEvent::transferPausedByPeerEvent += boost::bind(&PhApiSendFileSession::transferPausedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferPausedEvent += boost::bind(&PhApiSendFileSession::transferPausedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedByPeerEvent += boost::bind(&PhApiSendFileSession::transferResumedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedEvent += boost::bind(&PhApiSendFileSession::transferResumedEventHandler, this, _1, _2, _3, _4, _5, _6);
}

PhApiSendFileSession::PhApiSendFileSession(const PhApiSendFileSession & phApiSendFileSession)
: ISendFileSession(phApiSendFileSession), _userProfile(phApiSendFileSession._userProfile) {
	_initialized = phApiSendFileSession._initialized;
	_currentCallID = phApiSendFileSession._currentCallID;
	
	PhApiSFPEvent::inviteToTransferEvent += boost::bind(&PhApiSendFileSession::inviteToTransferEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::waitingForAnswerEvent += boost::bind(&PhApiSendFileSession::waitingForAnswerEventHandler, this, _1, _2, _3);
	PhApiSFPEvent::transferCancelledEvent += boost::bind(&PhApiSendFileSession::transferCancelledEventHandler, this, _1, _2, _3, _4, _5);
	PhApiSFPEvent::transferCancelledByPeerEvent += boost::bind(&PhApiSendFileSession::transferCancelledByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::sendingFileBeginEvent += boost::bind(&PhApiSendFileSession::sendingFileBeginEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFinishedEvent += boost::bind(&PhApiSendFileSession::transferFromPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFinishedEvent += boost::bind(&PhApiSendFileSession::transferToPeerFinishedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferFromPeerFailedEvent += boost::bind(&PhApiSendFileSession::transferFromPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferToPeerFailedEvent += boost::bind(&PhApiSendFileSession::transferToPeerFailedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferProgressionEvent += boost::bind(&PhApiSendFileSession::transferProgressionEventHandler, this, _1, _2, _3);
	PhApiSFPEvent::transferPausedByPeerEvent += boost::bind(&PhApiSendFileSession::transferPausedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferPausedEvent += boost::bind(&PhApiSendFileSession::transferPausedEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedByPeerEvent += boost::bind(&PhApiSendFileSession::transferResumedByPeerEventHandler, this, _1, _2, _3, _4, _5, _6);
	PhApiSFPEvent::transferResumedEvent += boost::bind(&PhApiSendFileSession::transferResumedEventHandler, this, _1, _2, _3, _4, _5, _6);
}

PhApiSendFileSession * PhApiSendFileSession::clone() const {
	return new PhApiSendFileSession(*this);
}

PhApiSendFileSession::~PhApiSendFileSession() {
}

void PhApiSendFileSession::start() {
	LOG_DEBUG("starting file transfer");
	if(!_initialized){
		_currentContact = _imContactSet.begin();
		_currentFile = _fileVector.begin();
		_initialized = true;
	}
	if(_account != NULL && _imContactSet.size()>0 && _fileVector.size()>0){
		//WengoAccount * wAccount = dynamic_cast<WengoAccount *>(_account);
		WengoAccount * wAccount = _userProfile.getWengoAccount();
		PhApiSFPWrapper & wrapper = PhApiSFPWrapper::getInstance();
		int vlid = wAccount->getVLineID();
		std::string fullIdentity = wAccount->getDisplayName()+std::string(" <")+wAccount->getSIPAddress()+std::string(">");
		std::string contactUri = std::string("sip:")+(*_currentContact).getContactId();
		std::string fileType = (*_currentFile).getExtension();
		if(fileType == String::null) {
			fileType = "*";
		}
		_currentCallID = wrapper.sendFile(vlid,
			fullIdentity,
			contactUri,
			(*_currentFile).getFullPath(),
			(*_currentFile).getFileName(),
			fileType,
			(*_currentFile).getSize());
	}
	LOG_DEBUG("end of transfer start");
}

void PhApiSendFileSession::pause() {
	LOG_DEBUG("pausing file transfer");
	if(_currentCallID > 0){
		PhApiSFPWrapper::getInstance().pauseTransfer(_currentCallID);
	}
	LOG_DEBUG("end of transfer pause");
}

void PhApiSendFileSession::resume() {
	LOG_DEBUG("resuming file transfer");
	if(_currentCallID > 0){
		PhApiSFPWrapper::getInstance().resumeTransfer(_currentCallID);
	}
	LOG_DEBUG("end of transfer resume");
}

void PhApiSendFileSession::stop() {
	LOG_DEBUG("stopping file transfer");
	if(_currentCallID > 0){
		PhApiSFPWrapper::getInstance().cancelTransfer(_currentCallID);
	}
	LOG_DEBUG("end of transfer stop");
}

void PhApiSendFileSession::inviteToTransferEventHandler(PhApiSFPWrapper & sender, int callID, std::string uri, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventInviteToTransfer, *_currentContact, *_currentFile);
	}
}

void PhApiSendFileSession::waitingForAnswerEventHandler(PhApiSFPWrapper & sender, int callID, std::string uri){
	if(callID == _currentCallID && (*_currentContact).getContactId().compare(extractContactIDFromURI(uri)) == 0){
		fileTransferEvent(*this, IFileSession::IFileSessionEventWaitingForAnswer, *_currentContact, *_currentFile);
	}
}

void PhApiSendFileSession::transferCancelledEventHandler(PhApiSFPWrapper & sender, int callID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferCancelled, *_currentContact, *_currentFile);
		moduleFinishedEvent(*this);
	}
}

void PhApiSendFileSession::transferCancelledByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferCancelledByPeer, *_currentContact, *_currentFile);
		moduleFinishedEvent(*this);
	}
}

void PhApiSendFileSession::sendingFileBeginEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferBegan, *_currentContact, *_currentFile);
	}
}

void PhApiSendFileSession::transferFromPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFinished, *_currentContact, *_currentFile);
		moduleFinishedEvent(*this);
	}
}

void PhApiSendFileSession::transferToPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFinished, *_currentContact, *_currentFile);
		// send one file to each contact, then go to the next file
		_currentCallID = -1;
		_currentContact++;
		if(_currentContact == _imContactSet.end()){
			_currentContact = _imContactSet.begin();
			if(_currentFile != _fileVector.end()){
				_currentFile++;
			}
		}
		if(_currentFile != _fileVector.end()){
			start();
		}else{
			moduleFinishedEvent(*this);
		}
	}
}

void PhApiSendFileSession::transferFromPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFailed, *_currentContact, *_currentFile);
		moduleFinishedEvent(*this);
	}
}

void PhApiSendFileSession::transferToPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferFailed, *_currentContact, *_currentFile);
		moduleFinishedEvent(*this);
	}
}

void PhApiSendFileSession::transferProgressionEventHandler(PhApiSFPWrapper & sender, int callID, int percentage){
	if(callID == _currentCallID){
		fileTransferProgressionEvent(*this, *_currentContact, *_currentFile, percentage);
	}
}

void PhApiSendFileSession::transferPausedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferPausedByPeer, *_currentContact, *_currentFile);
	}
}

void PhApiSendFileSession::transferPausedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferPaused, *_currentContact, *_currentFile);
	}
}

void PhApiSendFileSession::transferResumedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferResumedByPeer, *_currentContact, *_currentFile);
	}
}

void PhApiSendFileSession::transferResumedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize){
	if(callID == _currentCallID) {
		fileTransferEvent(*this, IFileSession::IFileSessionEventFileTransferResumed, *_currentContact, *_currentFile);
	}
}

std::string PhApiSendFileSession::extractContactIDFromURI(std::string uri) const{
	std::string::size_type usernameStartIndex = uri.find(":");
	std::string::size_type usernameEndIndex = uri.find("@");
	if(usernameStartIndex == std::string::npos || usernameStartIndex == std::string::npos || usernameStartIndex > usernameEndIndex){
		return "";
	}
	return uri.substr(usernameStartIndex, usernameEndIndex-usernameStartIndex+1);
}
