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

#include "XPCOMPhoneLine.h"

#include <Listener.h>
#include "ListenerList.h"
#include "XPCOMIMHandler.h"

#include <control/phoneline/CPhoneLine.h>

#include <util/Logger.h>

XPCOMPhoneLine::XPCOMPhoneLine(CPhoneLine & cPhoneLine)
	: _cPhoneLine(cPhoneLine) {
	PhoneLineList::getInstance().add(this);
}

int XPCOMPhoneLine::makeCall(const std::string & phoneNumber) {
	return _cPhoneLine.makeCall(phoneNumber);
}

void XPCOMPhoneLine::phoneLineStateChangedEvent(EnumPhoneLineState::PhoneLineState state) {
	/*ListenerList & listenerList = ListenerList::getInstance();
	for (unsigned i = 0; i < listenerList.size(); i++) {
		Listener * listener = listenerList[i];

		listener->phoneLineStateChangedEvent(state, lineId, NULL);
	}*/
}
