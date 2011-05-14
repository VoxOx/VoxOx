/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* Represent Chat to email Message types 
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtEnumUMMode.h"

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<QtEnumUMMode::Mode, QString> UniversalMap;
static UniversalMap _universalMap;

static void init() {
	if (!_universalMap.empty()) {
		return;
	}
	_universalMap[QtEnumUMMode::UniversalMessageChat] = "Chat";
	_universalMap[QtEnumUMMode::UniversalMessageChatToEmail] = "Email";
	_universalMap[QtEnumUMMode::UniversalMessageChatToSMS] = "SMS";
	_universalMap[QtEnumUMMode::UniversalMessageFileTransfer] = "File Transfer";
	_universalMap[QtEnumUMMode::UniversalMessageFax] = "Fax";
	_universalMap[QtEnumUMMode::UniversalMessageContactProfile] = "Contact Profile";
	_universalMap[QtEnumUMMode::UniversalMessageCall] = "Call";
	_universalMap[QtEnumUMMode::UniversalMessageConferenceCall] = "Conference";//VOXOX CHANGE by Rolando - 2009.05.29 	
	_universalMap[QtEnumUMMode::UniversalMessageUnknown] = "Unknown";
	
}


QString QtEnumUMMode::toString(Mode umMode) {
	init();
	QString tmp = _universalMap[umMode];
	if (tmp == "") {
		LOG_FATAL("unknown mode");
	}
	return tmp;
}