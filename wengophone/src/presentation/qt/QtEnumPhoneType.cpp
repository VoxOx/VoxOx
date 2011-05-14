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
#include "QtEnumPhoneType.h"

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<QtEnumPhoneType::Type, QString> PhoneType;
static PhoneType _type;

static void init() {
	if (!_type.empty()) {
		return;
	}
	_type[QtEnumPhoneType::Voxox] = "voxox";
	_type[QtEnumPhoneType::Work] = "work";
	_type[QtEnumPhoneType::Mobile] = "mobile";
	_type[QtEnumPhoneType::Home] = "home";
	_type[QtEnumPhoneType::Other] = "other";
	_type[QtEnumPhoneType::Fax] = "fax";
	
}

QString QtEnumPhoneType::toString(Type phoneType) {
	init();
	QString tmp = _type[phoneType];
	if (tmp == "") {
		LOG_FATAL("unknown phone type");
	}
	return tmp;
}

QString QtEnumPhoneType::getTypeIconPath(Type phoneType) {
	init();
	QString tmp = _type[phoneType];

	if (tmp == "") {
		LOG_FATAL("unknown phone type");
	}
	return QString(":pics/types/telephones/%1.png").arg(tmp);
}

QtEnumPhoneType::Type QtEnumPhoneType::toPhoneType(const QString & phoneType) {
	init();
	for (PhoneType::const_iterator it = _type.begin();
		it != _type.end();
		++it) {

		if ((*it).second == phoneType) {
			return (*it).first;
		}
	}

	LOG_FATAL("unknown phoneType");
	return Unknown;
}