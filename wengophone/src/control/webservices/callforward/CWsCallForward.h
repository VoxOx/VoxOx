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

#ifndef CWSCALLFORWARD_H
#define CWSCALLFORWARD_H

#include <model/webservices/callforward/WsCallForward.h>

#include <util/Trackable.h>

class CWengoPhone;

/**
 *
 * @ingroup control
 * @author Mathieu Stute
 */
class CWsCallForward : public Trackable {

public:

	CWsCallForward(CWengoPhone & cWengoPhone, WsCallForward & wsCallForward);

	~CWsCallForward();

	/**
	 * @see WsCallForward
	 */
	Event<void (WsCallForward & sender, int id, WsCallForward::WsCallForwardStatus status)> wsCallForwardEvent;

	/**
	 * @see WsCallForward
	 */
	void disableCallForward();

	/**
	 * @see WsCallForward
	 */
	void forwardToVoiceMail();

	/**
	 * @see WsCallForward
	 */
	void forwardToNumber(const std::string & number1, const std::string & number2, const std::string & number3);

	CWengoPhone& getCWengoPhone();

private:

	WsCallForward & _wsCallForward;

	CWengoPhone& _cWengoPhone;
};

#endif /* CWSCALLFORWARD_H */
