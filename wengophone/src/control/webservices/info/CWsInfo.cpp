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
#include "CWsInfo.h"

CWsInfo::CWsInfo(WsInfo & wsInfo, CWengoPhone & cWengoPhone)
	: _wsInfo(wsInfo), _cWengoPhone(cWengoPhone) {

	_wsInfo.wsInfoWengosEvent += wsInfoWengosEvent;
	_wsInfo.wsInfoSmsCountEvent += wsInfoSmsCountEvent;
	_wsInfo.wsInfoActiveMailEvent += wsInfoActiveMailEvent;
	_wsInfo.wsInfoVoiceMailEvent += wsInfoVoiceMailEvent;
	_wsInfo.wsInfoLandlineNumberEvent += wsInfoLandlineNumberEvent;
	_wsInfo.wsCallForwardInfoEvent += wsCallForwardInfoEvent;
}

CWsInfo::~CWsInfo() {
}

void CWsInfo::getWengosCount() {
	_wsInfo.getWengosCount(true);
	_wsInfo.execute();
}

void CWsInfo::getSmsCount() {
	_wsInfo.getSmsCount(true);
	_wsInfo.execute();
}

void CWsInfo::getActiveMail() {
	_wsInfo.getActiveMail(true);
	_wsInfo.execute();
}

void CWsInfo::getUnreadVoiceMail() {
	_wsInfo.getUnreadVoiceMail(true);
	_wsInfo.execute();
}

void CWsInfo::getCallForwardInfo() {
	_wsInfo.getCallForwardInfo(true);
	_wsInfo.execute();
}

void CWsInfo::getLandlineNumber() {
	_wsInfo.getLandlineNumber(true);
	_wsInfo.execute();
}
