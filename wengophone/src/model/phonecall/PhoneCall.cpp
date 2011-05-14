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
#include "PhoneCall.h"

#include "PhoneCallStateUnknown.h"
#include "PhoneCallStateClosed.h"
#include "PhoneCallStateDialing.h"
#include "PhoneCallStateError.h"
#include "PhoneCallStateHold.h"
#include "PhoneCallStateIncoming.h"
#include "PhoneCallStateTalking.h"
#include "PhoneCallStateResumed.h"
#include "PhoneCallStateRinging.h"
#include "PhoneCallStateRingingStart.h"
#include "PhoneCallStateRingingStop.h"

#include <model/phoneline/IPhoneLine.h>
#include <model/profile/UserProfile.h>
#include <model/wenbox/WenboxPlugin.h>

#include <util/Logger.h>

#include <ctime>

PhoneCall::PhoneCall(IPhoneLine & phoneLine, const SipAddress & sipAddress)
	: _phoneLine(phoneLine) {

	_duration = -1;
	_holdRequest = false;
	_resumeRequest = false;
	_conferenceCall = NULL;
	_callRejected = false;
	_timeStart = -1;
	_videoEnabled = false;

	_sipAddress = sipAddress;

	static PhoneCallStateUnknown stateUnknown;

	//Default state (PhoneCallStateUnknown)
	_state = &stateUnknown;

	_phoneCallStateList += &stateUnknown;

	static PhoneCallStateClosed stateClosed;
	_phoneCallStateList += &stateClosed;

	static PhoneCallStateDialing stateDialing;
	_phoneCallStateList += &stateDialing;

	static PhoneCallStateError stateError;
	_phoneCallStateList += &stateError;

	static PhoneCallStateHold stateHold;
	_phoneCallStateList += &stateHold;

	static PhoneCallStateIncoming stateIncoming;
	_phoneCallStateList += &stateIncoming;

	static PhoneCallStateTalking stateTalking;
	_phoneCallStateList += &stateTalking;

	static PhoneCallStateResumed stateResumed;
	_phoneCallStateList += &stateResumed;

	static PhoneCallStateRinging stateRinging;
	_phoneCallStateList += &stateRinging;

	static PhoneCallStateRingingStart stateRingingStart;
	_phoneCallStateList += &stateRingingStart;

	static PhoneCallStateRingingStop stateRingingStop;
	_phoneCallStateList += &stateRingingStop;
}

PhoneCall::~PhoneCall() {
	//Do not need to delete everything (_phoneCallStateList)
	//since states are static inside the constructor
	_state = NULL;
}

void PhoneCall::accept() {
	PhoneCallState::stopSoundIncomingCall();
	_phoneLine.acceptCall(_callId);
}

void PhoneCall::resume() {
	if (_state->getCode() == EnumPhoneCallState::PhoneCallStateHold) {
		_resumeRequest = false;
		_phoneLine.resumeCall(_callId);
	} else {
		_resumeRequest = true;
	}
}

void PhoneCall::hold() {
	if (_state->getCode() == EnumPhoneCallState::PhoneCallStateTalking ||
		_state->getCode() == EnumPhoneCallState::PhoneCallStateResumed) {

		_holdRequest = false;
		_phoneLine.holdCall(_callId);
	} else {
		_holdRequest = true;
	}
}

void PhoneCall::blindTransfer(const std::string & sipAddress) {
	_phoneLine.blindTransfer(_callId, sipAddress);
}

EnumPhoneCallState::PhoneCallState PhoneCall::getState() const {
	return _state->getCode();
}

void PhoneCall::setState(EnumPhoneCallState::PhoneCallState state) {
	LOG_DEBUG("PhoneCallState=" + String::fromNumber(state));

	for (unsigned i = 0; i < _phoneCallStateList.size(); i++) {
		PhoneCallState * callState = _phoneCallStateList[i];
		if (callState->getCode() == state) {
			if (_state->getCode() != callState->getCode()) {
				_state = callState;
				
				if(_state->getCode() == EnumPhoneCallState::PhoneCallStateIncoming)
				{
					if(_phoneLine.hasPendingCalls())
						_state->execute(*this,true);
					else
						_state->execute(*this);
				}
				else
					_state->execute(*this);
				
				LOG_DEBUG("call state changed callId=" + String::fromNumber(_callId) +
					" state=" + EnumPhoneCallState::toString(_state->getCode()));
				applyState(state);
				stateChangedEvent(*this, state);
				return;
			}
		}
	}

	//LOG_FATAL("unknown PhoneCallState=" + String::fromNumber(state));
}

void PhoneCall::applyState(EnumPhoneCallState::PhoneCallState state) {
	if (getVideoCodecUsed() == CodecList::VideoCodecError) {
		_videoEnabled = true;
	} else {
		_videoEnabled = false;
	}

	//This should not replace the state machine pattern PhoneCallState
	switch(state) {
	case EnumPhoneCallState::PhoneCallStateUnknown:
		break;

	case EnumPhoneCallState::PhoneCallStateError:
		break;

	case EnumPhoneCallState::PhoneCallStateResumed:
		break;

	case EnumPhoneCallState::PhoneCallStateTalking:
		if (_holdRequest) {
			hold();
		} else {
			//Start of the call, computes duration
			_timeStart = time(NULL);
		}
		break;

	case EnumPhoneCallState::PhoneCallStateDialing:
		break;

	case EnumPhoneCallState::PhoneCallStateRinging:
		break;

	case EnumPhoneCallState::PhoneCallStateClosed:
		//End of the call, computes duration
		if (_timeStart != -1) {
			_duration = time(NULL) - _timeStart;
		}

		if (!_callRejected) {
			//Call missed if incoming state + closed state without being rejected
			setState(EnumPhoneCallState::PhoneCallStateMissed);
		}
		break;

	case EnumPhoneCallState::PhoneCallStateIncoming:
		break;

	case EnumPhoneCallState::PhoneCallStateHold:
		if (_resumeRequest) {
			resume();
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

void PhoneCall::close() {
	if (_state->getCode() != EnumPhoneCallState::PhoneCallStateClosed) {
		if (_state->getCode() == EnumPhoneCallState::PhoneCallStateIncoming) {
			_callRejected = true;
			_phoneLine.rejectCall(_callId);
		} else if (_state->getCode() != EnumPhoneCallState::PhoneCallStateError) {
			_phoneLine.closeCall(_callId);
		}
		setState(EnumPhoneCallState::PhoneCallStateClosed);
	}
}

WenboxPlugin & PhoneCall::getWenboxPlugin() const {
	return *_phoneLine.getUserProfile().getWenboxPlugin();
}

void PhoneCall::videoFrameReceived(piximage * remoteVideoFrame, piximage * localVideoFrame) {
	videoFrameReceivedEvent(*this, remoteVideoFrame, localVideoFrame);
}

void PhoneCall::playSoundFile(const std::string & soundFile) {
	_phoneLine.playSoundFile(_callId, soundFile);
}

CodecList::AudioCodec PhoneCall::getAudioCodecUsed() {
	return _phoneLine.getAudioCodecUsed(_callId);
}

CodecList::VideoCodec PhoneCall::getVideoCodecUsed() {
	return _phoneLine.getVideoCodecUsed(_callId);
}

bool PhoneCall::isCallEncrypted() const {
	return _phoneLine.isCallEncrypted(_callId);
}
