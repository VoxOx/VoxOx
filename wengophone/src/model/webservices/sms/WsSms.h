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

#ifndef OWWSSMS_H
#define OWWSSMS_H

#include "EnumSmsState.h"

#include <model/webservices/WengoWebService.h>

class UserProfile;

/**
 * Wengo SMS web service.
 *
 * Permits to send SMS.
 *
 * @author Mathieu Stute
 */
class WsSms : public WengoWebService {
public:

	/** Maximum SMS length = 150 characters. */
	static const unsigned SMS_MAX_LENGTH = 150;

	/**
	 * Default constructor.
	 *
	 * @param wengoAccount the WengoAccount used for web services
	 */
	WsSms(WengoAccount * wengoAccount, UserProfile & userProfile);

	virtual ~WsSms() {}

	/**
	 * Callback to check if the SMS was received or not.
	 *
	 * @param sender this class
	 * @param smsId SMS unique identifier
	 * @param state SMS status (ok or error)
	 */
	Event<void (WsSms & sender, int smsId, EnumSmsState::SmsState state)> smsStatusEvent;

	/**
	 * Sends a SMS given a destination phone number and a message.
	 *
	 * @param phoneNumber phone that will receive the SMS
	 * @param message SMS message
	 * @return unique SMS ID
	 */
	int sendSMS(const std::string & phoneNumber, const std::string & message);

private:

	void answerReceived(const std::string & answer, int requestId);

	UserProfile & _userProfile;
};

#endif //OWWSSMS_H
