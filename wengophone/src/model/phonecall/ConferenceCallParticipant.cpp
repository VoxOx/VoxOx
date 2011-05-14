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
#include "ConferenceCallParticipant.h"

#include "ConferenceCall.h"
#include "PhoneCall.h"
#include "PhoneCallState.h"

#include <model/phoneline/IPhoneLine.h>

#include <util/Logger.h>

ConferenceCallParticipant::ConferenceCallParticipant(ConferenceCall & conferenceCall, PhoneCall & phoneCall)
	: _conferenceCall(conferenceCall),
	_phoneCall(phoneCall) {

	_waitForHoldState = false;

	_phoneCall.stateChangedEvent += boost::bind(&ConferenceCallParticipant::phoneCallStateChangedEventHandler, this, _1, _2);

	joinConference();
}

void ConferenceCallParticipant::joinConference() {
	int callId = _phoneCall.getCallId();

	if (_phoneCall.getState() == EnumPhoneCallState::PhoneCallStateHold) {
		_waitForHoldState = false;
		_conferenceCall.join(callId);
	} else {
		_waitForHoldState = true;

		//FIXME hack, do not hold the first phone call of the conference, user has to do it himself
		/*if (_conferenceCall.getPhoneCallList().size() > 1) {
			_phoneCall.hold();
		}*/
	}
}

void ConferenceCallParticipant::phoneCallStateChangedEventHandler(PhoneCall & sender, EnumPhoneCallState::PhoneCallState state) {
	switch(state) {

	case EnumPhoneCallState::PhoneCallStateUnknown:
		break;

	case EnumPhoneCallState::PhoneCallStateError:
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
		break;

	case EnumPhoneCallState::PhoneCallStateIncoming:
		break;

	case EnumPhoneCallState::PhoneCallStateHold:
 		if (_waitForHoldState) {
			joinConference();
		}
		break;

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
