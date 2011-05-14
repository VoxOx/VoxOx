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
#include "ConferenceCall.h"

#include "ConferenceCallParticipant.h"
#include "PhoneCall.h"

#include <model/phoneline/IPhoneLine.h>
#include <model/account/SipAccount.h>

#include <sipwrapper/SipWrapper.h>

#include <util/Logger.h>

ConferenceCall::ConferenceCall(IPhoneLine & phoneLine)
	: _phoneLine(phoneLine) {

	//No conference started yet
	_confId = -1;
}

ConferenceCall::~ConferenceCall() {
}

void ConferenceCall::addPhoneCall(PhoneCall & phoneCall) {
	if (_confId == -1) {
		_confId = _phoneLine.getSipWrapper().createConference();
	}

	new ConferenceCallParticipant(*this, phoneCall);
	phoneCallAddedEvent(*this, phoneCall);

	std::string phoneNumber = phoneCall.getPeerSipAddress().getUserName();
	_phoneCallMap[phoneNumber] = &phoneCall;
}

void ConferenceCall::removePhoneCall(PhoneCall & phoneCall) {
	phoneCall.setConferenceCall(NULL);
	int callId = phoneCall.getCallId();

	if (_confId == -1) {
		_phoneLine.getSipWrapper().splitConference(_confId, callId);
	}

	phoneCallRemovedEvent(*this, phoneCall);
	std::string phoneNumber = phoneCall.getPeerSipAddress().getUserName();
	_phoneCallMap.erase(phoneNumber);
}

void ConferenceCall::addPhoneNumber(const std::string & phoneNumber) {
	//Searches for a already existing PhoneCall associated with this phone number
	PhoneCall * phoneCall = getPhoneCall(phoneNumber);
	if (!phoneCall) {
		phoneCall = _phoneCallMap[phoneNumber];
		if (!phoneCall) {
			EnumMakeCallError::MakeCallError error = _phoneLine.makeCall(phoneNumber, this);
			if (error != EnumMakeCallError::NoError) {
				//NULL means put the phone number in the queue
				phoneCall = NULL;
				makeConferenceCallErrorEvent(*this, error, phoneNumber);//VOXOX CHANGE by Rolando - 2009.05.29 - added to handle errors in a conference call

			} else {					
				phoneCall = _phoneLine.getActivePhoneCall();
			}
		}
	}

	_phoneCallMap[phoneNumber] = phoneCall;
	if (phoneCall) {
		phoneCall->stateChangedEvent += boost::bind(&ConferenceCall::phoneCallStateChangedEventHandler, this, _1, _2);
		addPhoneCall(*phoneCall);
	}
	LOG_DEBUG("phone number added=" + phoneNumber);
}

void ConferenceCall::removePhoneNumber(const std::string & phoneNumber) {
	for (PhoneCalls::iterator it = _phoneCallMap.begin(); it != _phoneCallMap.end(); ++it) {
		PhoneCall * phoneCall = (*it).second;
		if (phoneCall) {
			SipAddress sipAddress = phoneCall->getPeerSipAddress();
			if (sipAddress.getUserName() == phoneNumber) {
				removePhoneCall(*phoneCall);
			}
		}
	}

	if (_phoneCallMap.empty()) {
		stateChangedEvent(*this, EnumConferenceCallState::ConferenceCallStateStopped);
	}
}

PhoneCall * ConferenceCall::getPhoneCall(const std::string & phoneNumber) const {
	PhoneCall * phoneCall = NULL;

	IPhoneLine::PhoneCallList calls = _phoneLine.getPhoneCallList();

	for (unsigned i = 0; i < calls.size(); i++) {
		SipAddress sipAddress = calls[i]->getPeerSipAddress();
		if (sipAddress.getUserName() == phoneNumber) {
			return calls[i];
		}
	}

	return phoneCall;
}

void ConferenceCall::phoneCallStateChangedEventHandler(PhoneCall & sender, EnumPhoneCallState::PhoneCallState state) {
	switch (state) {

	case EnumPhoneCallState::PhoneCallStateUnknown:
		break;

	case EnumPhoneCallState::PhoneCallStateError:
		removePhoneCall(sender);
		break;

	case EnumPhoneCallState::PhoneCallStateResumed:
		break;

	case EnumPhoneCallState::PhoneCallStateTalking:
		break;

	case EnumPhoneCallState::PhoneCallStateDialing:
		break;

	case EnumPhoneCallState::PhoneCallStateRinging:
		break;

	case EnumPhoneCallState::PhoneCallStateClosed:
		removePhoneCall(sender);
		break;

	case EnumPhoneCallState::PhoneCallStateIncoming:
		break;

	case EnumPhoneCallState::PhoneCallStateHold: {
		//Takes randomly a phoneNumber that has no PhoneCall associated
		//and creates a PhoneCall
		for (PhoneCalls::iterator it = _phoneCallMap.begin(); it != _phoneCallMap.end(); ++it) {
			PhoneCall * phoneCall = (*it).second;
			if (!phoneCall) {
				std::string phoneNumber = (*it).first;
				addPhoneNumber(phoneNumber);
				break;
			}
		}

		break;
	}

	case EnumPhoneCallState::PhoneCallStateMissed:
		break;

	case EnumPhoneCallState::PhoneCallStateRedirected:
		break;

	case EnumPhoneCallState::PhoneCallStateRingingStart:
		break;

	case EnumPhoneCallState::PhoneCallStateRingingStop:
		break;

	default:
		LOG_FATAL("unknown PhoneCallState=" + EnumPhoneCallState::toString(state));
	}
}

ConferenceCall::PhoneCallList ConferenceCall::getPhoneCallList() const {
	PhoneCallList phoneCallList;

	for (PhoneCalls::const_iterator it = _phoneCallMap.begin(); it != _phoneCallMap.end(); ++it) {
		PhoneCall * phoneCall = (*it).second;
		if (phoneCall) {
			phoneCallList += phoneCall;
		}
	}
	return phoneCallList;
}

void ConferenceCall::join(int callId) {
	_phoneLine.getSipWrapper().joinConference(_confId, callId);
}
