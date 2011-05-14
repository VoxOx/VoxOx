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
#include "HistoryMemento.h"

#include <util/String.h>
#include <util/Logger.h>

HistoryMemento::HistoryMemento() {
	_state = None;
	_callId = -1;
	_duration = 0;
	_date = Date();
	_time = Time();
}

HistoryMemento::HistoryMemento(State state, const std::string & peer, int callId, const std::string & data) {
	_state = state;
	_peer = peer;
	_callId = callId;
	_duration = 0;
	_date = Date();
	_time = Time();
	_data = data;
}

HistoryMemento::HistoryMemento(State state, Date date, Time time, const std::string & peer, int callId, const std::string & data) {
	_state = state;
	_peer = peer;
	_callId = callId;
	_duration = 0;
	_date = date;
	_time = time;
	_data = data;
}

HistoryMemento::~HistoryMemento() {
}

HistoryMemento::State HistoryMemento::getState() const {
	return _state;
}

std::string HistoryMemento::getPeer() const {
	return _peer;
}

int HistoryMemento::getDuration() const {
	return _duration;
}

Date HistoryMemento::getDate() const {
	return _date;
}

Time HistoryMemento::getTime() const {
	return _time;
}

int HistoryMemento::getCallId() const {
	return _callId;
}

std::string HistoryMemento::getData() const {
	return _data;
}

void HistoryMemento::updateDuration(int duration) {
	_duration = duration;
}

void HistoryMemento::updateState(State state) {
	_state = state;
}

bool HistoryMemento::canReplay() const {
	return ((_state == OutgoingCall) || (_state == OutgoingSmsOk) || 
			(_state == IncomingCall) || (_state == MissedCall) ||
			(_state == RejectedCall));
}

bool HistoryMemento::isCallMemento() const {
	return ((_state == OutgoingCall) || (_state == IncomingCall) ||
			(_state == MissedCall)  || (_state == RejectedCall));
}

bool HistoryMemento::isSMSMemento() const {
	return ((_state == OutgoingSmsOk) || (_state == OutgoingSmsNok));
}

bool HistoryMemento::isChatSessionMemento() const {
	return (_state == ChatSession);
}

std::string HistoryMemento::toString() const {
	std::string toReturn;
	static const std::string separator = "\n\t- ";

	toReturn += "Peer: " + _peer + separator;
	toReturn += "date: " + _date.toString() + separator;
	toReturn += "time: " + _time.toString() + separator;
	toReturn += "duration: " + String::fromNumber(_duration) + separator;
	toReturn += "state: " + stateToString(_state) + separator;
	toReturn += "callid: " + String::fromNumber(_callId) + separator;
	toReturn += "data: " + _data;

	return toReturn;
}

std::string HistoryMemento::stateToString(State state) {
	switch(state) {
	case OutgoingCall:
		return "StateOutgoingCall";

	case IncomingCall:
		return "StateIncomingCall";

	case OutgoingSmsOk:
		return "StateOutgoingSMSOK";

	case OutgoingSmsNok:
		return "StateOutgoingSMSNOK";

	case RejectedCall:
		return "StateRejectedCall";

	case MissedCall:
		return "StateMissedCall";

	case None:
		return "StateNone";

	case Any:
		return "StateAny";

	case ChatSession:
		return "StateChat";

	default:
		LOG_FATAL("unknown state=" + String::fromNumber(state));
		return "";
	}
}

