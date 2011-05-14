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

#include "XPCOMPhoneCall.h"

#include <Listener.h>
#include "ListenerList.h"

#include <control/phonecall/CPhoneCall.h>

#include <util/Logger.h>

XPCOMPhoneCall::XPCOMPhoneCall(CPhoneCall & cPhoneCall)
	: _cPhoneCall(cPhoneCall) {
}

void XPCOMPhoneCall::phoneCallStateChangedEvent(EnumPhoneCallState::PhoneCallState state) {

	/*PhoneCallMap & phoneCallMap = PhoneCallMap::getInstance();
	phoneCallMap[callId] = this;

	ListenerList & listenerList = ListenerList::getInstance();
	for (unsigned i = 0; i < listenerList.size(); i++) {
		Listener * listener = listenerList[i];

		listener->phoneCallStateChangedEvent(state, lineId, callId, sipAddress, userName, displayName, NULL);
	}*/
}

void XPCOMPhoneCall::hangUp() {
	_cPhoneCall.hangUp();
}

bool XPCOMPhoneCall::canHangUp() const {
	return _cPhoneCall.canHangUp();
}

void XPCOMPhoneCall::pickUp() {
	_cPhoneCall.pickUp();
}

bool XPCOMPhoneCall::canPickUp() const {
	return _cPhoneCall.canPickUp();
}

void XPCOMPhoneCall::playTone(EnumTone::Tone tone) {
	_cPhoneCall.playTone(tone);
}
