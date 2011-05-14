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

#include "XPCOMSms.h"

#include <Listener.h>
#include "ListenerList.h"

#include <util/StringList.h>
#include <util/Logger.h>
#include <util/Event.h>

XPCOMSms * XPCOMSms::sms = NULL;

XPCOMSms::XPCOMSms(CSms & cSms)
	: _cSms(cSms) {

	sms = this;
	_cSms.smsStatusEvent += boost::bind(&XPCOMSms::smsStatusEventHandler, this, _1, _2, _3);
}

int XPCOMSms::sendSMS(const std::string & phoneNumber, const std::string & message) {
	return _cSms.sendSMS(phoneNumber, message);
}

void XPCOMSms::smsStatusEventHandler(Sms & sender, int smsId, EnumSmsState::SmsState state) {
	ListenerList & listenerList = ListenerList::getInstance();
	for (unsigned i = 0; i < listenerList.size(); i++) {
		Listener * listener = listenerList[i];

		listener->smsStatusEvent(smsId, state);
	}
}
