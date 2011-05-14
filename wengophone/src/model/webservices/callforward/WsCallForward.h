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

#ifndef OWWSCALLFORWARD_H
#define OWWSCALLFORWARD_H

#include <model/webservices/WengoWebService.h>

class Settings;

/**
 * Call forward web service.
 *
 * Sets the call forward method on the server the user wants.
 *
 * @author Mathieu Stute
 */
class WsCallForward : public WengoWebService {
public:

	enum WsCallForwardStatus {
		/** The information could not be set. */
		WsCallForwardStatusAuthenticationError,

		/** The information could not be set. */
		WsCallForwardStatusUrlError,

		/** Set information successfully. */
		WsCallForwardStatusOk
	};

	/**
	 * Default constructor.
	 *
	 * @param wengoAccount the WengoAccount used for authentication
	 */
	WsCallForward(WengoAccount * wengoAccount);

	/**
	 * A anwser about unread voice mail has been received.
	 *
	 * @param sender this class
	 * @param id unique identifier of the request
	 * @param status the request status (ok or error)
	 */
	Event<void (WsCallForward & sender, int id, WsCallForwardStatus status)> wsCallForwardEvent;

	/**
	 * Disables call forward.
	 */
	void disableCallForward();

	/**
	 * Forwards all calls to voicemail.
	 */
	void forwardToVoiceMail();

	/**
	 * Forwards calls to number or nickname.
	 *
	 * @param number1 first forward number.
	 * @param number2 second forward number.
	 * @param number3 third forward number.
	 */
	void forwardToNumber(const std::string & number1, const std::string & number2, const std::string & number3);

private:

	/**
	 * @see WengoWebService
	 */
	void answerReceived(const std::string & answer, int requestId);
};

#endif	//OWWSCALLFORWARD_H
