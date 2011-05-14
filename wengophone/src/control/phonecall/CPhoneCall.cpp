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
#include "CPhoneCall.h"

#include <presentation/PPhoneCall.h>
#include <presentation/PFactory.h>

#include <control/CWengoPhone.h>

#include <model/WengoPhone.h>
#include <model/phonecall/PhoneCall.h>

#include <util/Logger.h>
#include <thread/ThreadEvent.h>

CPhoneCall::CPhoneCall(PhoneCall & phoneCall, CWengoPhone & cWengoPhone)
	: _phoneCall(phoneCall),
	_cWengoPhone(cWengoPhone) {

	_pPhoneCall = NULL;
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::initPresentationThreadSafe, this));
	PFactory::postEvent(event);
}

CPhoneCall::~CPhoneCall() {
	delete _pPhoneCall;
}

void CPhoneCall::initPresentationThreadSafe() {
	_pPhoneCall = PFactory::getFactory().createPresentationPhoneCall(*this);

	_phoneCall.stateChangedEvent += boost::bind(&CPhoneCall::stateChangedEventHandler, this, _1, _2);
	_phoneCall.videoFrameReceivedEvent += boost::bind(&CPhoneCall::videoFrameReceivedEventHandler, this, _1, _2, _3);
}

Presentation * CPhoneCall::getPresentation() const {
	return _pPhoneCall;
}

CWengoPhone & CPhoneCall::getCWengoPhone() const {
	return _cWengoPhone;
}

std::string CPhoneCall::getPeerSipAddress() const {
	return _phoneCall.getPeerSipAddress().getSipAddress();
}

std::string CPhoneCall::getPeerUserName() const {
	return _phoneCall.getPeerSipAddress().getUserName();
}

std::string CPhoneCall::getPeerDisplayName() const {
	return _phoneCall.getPeerSipAddress().getDisplayName();
}

int CPhoneCall::getDuration() const {
	return _phoneCall.getDuration();
}

void CPhoneCall::stateChangedEventHandler(PhoneCall & sender, EnumPhoneCallState::PhoneCallState state) {
	typedef ThreadEvent1<void (EnumPhoneCallState::PhoneCallState), EnumPhoneCallState::PhoneCallState> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::stateChangedEventHandlerThreadSafe, this, _1), state);
	PFactory::postEvent(event);
}

void CPhoneCall::stateChangedEventHandlerThreadSafe(EnumPhoneCallState::PhoneCallState state) {
	if (!_pPhoneCall) {
		return;
	}

	if (state == EnumPhoneCallState::PhoneCallStateClosed) {
		// the peer has closed the call
		unbindAndClose();
		////
	} else {
		_pPhoneCall->stateChangedEvent(state);
	}
}

void CPhoneCall::videoFrameReceivedEventHandler(PhoneCall & sender, piximage * remoteVideoFrame, piximage * localVideoFrame) {
	typedef ThreadEvent2<void (piximage *, piximage *), piximage *, piximage *> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::videoFrameReceivedEventHandlerThreadSafe, this, _1, _2), remoteVideoFrame, localVideoFrame);
	PFactory::postEvent(event);
}

void CPhoneCall::videoFrameReceivedEventHandlerThreadSafe(piximage * remoteVideoFrame, piximage * localVideoFrame) {
	if (_pPhoneCall) {
		_pPhoneCall->videoFrameReceivedEvent(remoteVideoFrame, localVideoFrame);
		std::string sipAddress = _pPhoneCall->getCPhoneCall().getPeerSipAddress();
	}
}

void CPhoneCall::hangUp() {
	// here we're called from the presentation thread
	unbindAndClose();
	////

	// let's go to the model thread
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::hangUpThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
	////
}

void CPhoneCall::hangUpThreadSafe() {
	_phoneCall.close();
}

void CPhoneCall::accept() {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::acceptThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}

void CPhoneCall::acceptThreadSafe() {
	_phoneCall.accept();
}

void CPhoneCall::hold() {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::holdThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}

void CPhoneCall::holdThreadSafe() {
	_phoneCall.hold();
}

void CPhoneCall::resume() {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::resumeThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}

void CPhoneCall::resumeThreadSafe() {
	_phoneCall.resume();
}

void CPhoneCall::blindTransfer(const std::string & phoneNumber) {
	typedef ThreadEvent1<void (std::string), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CPhoneCall::blindTransferThreadSafe, this, _1), phoneNumber);
	WengoPhone::getInstance().postEvent(event);
}

void CPhoneCall::blindTransferThreadSafe(std::string phoneNumber) {
	_phoneCall.blindTransfer(phoneNumber);
}

CodecList::AudioCodec CPhoneCall::getAudioCodecUsed() {
	return _phoneCall.getAudioCodecUsed();
}

CodecList::VideoCodec CPhoneCall::getVideoCodecUsed() {
	return _phoneCall.getVideoCodecUsed();
}

EnumPhoneCallState::PhoneCallState CPhoneCall::getState() const {
	return _phoneCall.getState();
}

void CPhoneCall::unbindAndClose() {
	_phoneCall.videoFrameReceivedEvent -= boost::bind(&CPhoneCall::videoFrameReceivedEventHandler, this, _1, _2, _3);
	_phoneCall.stateChangedEvent -= boost::bind(&CPhoneCall::stateChangedEventHandler, this, _1, _2);
	if(_pPhoneCall){//VOXOX CHANGE by Rolando - 2009.05.22 - if condition needed to prevent to close a pphonecall already closed
		_pPhoneCall->close();
	}
	_pPhoneCall = NULL;
}

bool CPhoneCall::isCallEncrypted() const {
	return _phoneCall.isCallEncrypted();
}
