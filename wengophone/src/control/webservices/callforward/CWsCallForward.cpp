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
#include "CWsCallForward.h"

#include <control/CWengoPhone.h>

CWsCallForward::CWsCallForward(CWengoPhone & cWengoPhone, WsCallForward & wsCallForward)
	: _wsCallForward(wsCallForward), _cWengoPhone(cWengoPhone) {

	_wsCallForward.wsCallForwardEvent += wsCallForwardEvent;
}

CWsCallForward::~CWsCallForward() {
}

CWengoPhone& CWsCallForward::getCWengoPhone() {
	return _cWengoPhone;
}

void CWsCallForward::disableCallForward() {
	_wsCallForward.disableCallForward();
}

void CWsCallForward::forwardToVoiceMail() {
	_wsCallForward.forwardToVoiceMail();
}

void CWsCallForward::forwardToNumber(const std::string & number1,
	const std::string & number2, const std::string & number3) {
	_wsCallForward.forwardToNumber(number1, number2, number3);
}
