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
#include "CConferenceCall.h"

#include <presentation/PFactory.h>
#include <presentation/PConferenceCall.h>

#include <control/CWengoPhone.h>

#include <model/WengoPhone.h>
#include <model/phonecall/ConferenceCall.h>

#include <util/Logger.h>
#include <thread/ThreadEvent.h>

CConferenceCall::CConferenceCall(ConferenceCall & conferenceCall, CWengoPhone & cWengoPhone)
	: _conferenceCall(conferenceCall),
	_cWengoPhone(cWengoPhone) {

	_pConferenceCall = NULL;
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CConferenceCall::initPresentationThreadSafe, this));
	PFactory::postEvent(event);
}

CConferenceCall::~CConferenceCall() {
}

void CConferenceCall::initPresentationThreadSafe() {
	_pConferenceCall = PFactory::getFactory().createPresentationConferenceCall(*this);

	_conferenceCall.stateChangedEvent += boost::bind(&CConferenceCall::stateChangedEventHandler, this, _1, _2);
	_conferenceCall.phoneCallAddedEvent += boost::bind(&CConferenceCall::phoneCallAddedEventHandler, this, _1, _2);
	_conferenceCall.phoneCallRemovedEvent += boost::bind(&CConferenceCall::phoneCallRemovedEventHandler, this, _1, _2);
}

Presentation * CConferenceCall::getPresentation() const {
	return _pConferenceCall;
}

CWengoPhone & CConferenceCall::getCWengoPhone() const {
	return _cWengoPhone;
}

void CConferenceCall::addPhoneCall(CPhoneCall & cPhoneCall) {
	//_conferenceCall.addPhoneCall(cPhoneCall.getPhoneCall());
}

void CConferenceCall::removePhoneCall(CPhoneCall & cPhoneCall) {
	//_conferenceCall.removePhoneCall(cPhoneCall.getPhoneCall());
}

void CConferenceCall::addPhoneNumber(const std::string & phoneNumber) {
	_conferenceCall.addPhoneNumber(phoneNumber);
}

void CConferenceCall::removePhoneNumber(const std::string & phoneNumber) {
	_conferenceCall.removePhoneNumber(phoneNumber);
}

void CConferenceCall::stateChangedEventHandler(ConferenceCall & sender, EnumConferenceCallState::ConferenceCallState state) {
	_pConferenceCall->stateChangedEvent(state);
}

void CConferenceCall::phoneCallAddedEventHandler(ConferenceCall & sender, PhoneCall & phoneCall) {
	//_pConferenceCall->phoneCallAddedEvent(phoneCall);
}

void CConferenceCall::phoneCallRemovedEventHandler(ConferenceCall & sender, PhoneCall & phoneCall) {
	//_pConferenceCall->phoneCallRemovedEvent(phoneCall);
}
