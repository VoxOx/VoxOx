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

#ifndef CSMS_H
#define CSMS_H

#include <model/webservices/sms/WsSms.h>

#include <util/Trackable.h>

#include <string>

class CWengoPhone;
class PSms;

/**
 *
 * @ingroup control
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class CSms : public Trackable {
public:

	/**
	 * @see Sms::smsStatusEvent
	 */
	Event <void (WsSms & sender, int smsId, EnumSmsState::SmsState state)> smsStatusEvent;

	CSms(WsSms & sms, CWengoPhone & cWengoPhone);

	~CSms();

	CWengoPhone & getCWengoPhone() const {
		return _cWengoPhone;
	}

	/**
	 * @see Sms::sendSMS()
	 */
	int sendSMS(const std::string & phoneNumber, const std::string & message);

private:

	WsSms & _wsSms;

	CWengoPhone & _cWengoPhone;

	PSms * _pSms;
};

#endif	//CSMS_H
