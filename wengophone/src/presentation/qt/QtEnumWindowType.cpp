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
#include "QtEnumWindowType.h"

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<QtEnumWindowType::Type, QString> UniversalWindowType;
static UniversalWindowType _universalWindowType;

static void init() {
	if (!_universalWindowType.empty()) {
		return;
	}
	_universalWindowType[QtEnumWindowType::WizardWindow] = "Wizard";
	_universalWindowType[QtEnumWindowType::ConfigWindow] = "Config";
	_universalWindowType[QtEnumWindowType::ContactManagerWindow] = "ContactManager";
	_universalWindowType[QtEnumWindowType::ProfileWindow] = "Profile";
	_universalWindowType[QtEnumWindowType::ChatHistoryWindow] = "ChatHistory";	
	_universalWindowType[QtEnumWindowType::UnknownWindow] = "Unknown";
	
}


QString QtEnumWindowType::toString(Type windowType) {
	init();
	QString tmp = _universalWindowType[windowType];
	if (tmp == "") {
		LOG_FATAL("unknown phone type");
	}
	return tmp;
}

QtEnumWindowType::Type QtEnumWindowType::toWindowType(const QString & windowType){
	init();
	for (UniversalWindowType::const_iterator it = _universalWindowType.begin();
		it != _universalWindowType.end();
		++it) {

		if ((*it).second == windowType) {
			return (*it).first;
		}
	}

	LOG_FATAL("unknown UniversalWindowType");
	return UnknownWindow;
}