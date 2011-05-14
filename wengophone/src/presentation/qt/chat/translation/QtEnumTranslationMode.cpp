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
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2010.01.17
*/



#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtEnumTranslationMode.h"

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<QtEnumTranslationMode::Mode, QString> TranslationMode;
static TranslationMode _mode;

static void init() {
	if (!_mode.empty()) {
		return;
	}
	_mode[QtEnumTranslationMode::None] = "None";
	_mode[QtEnumTranslationMode::Outgoing] = "Outgoing";
	_mode[QtEnumTranslationMode::Incoming] = "Incoming";
	_mode[QtEnumTranslationMode::Both] = "Both";
	
}

QString QtEnumTranslationMode::toString(Mode translationMode) {
	init();
	QString tmp = _mode[translationMode];
	if (tmp == "") {
		LOG_FATAL("unknown mode");
	}
	return tmp;
}


QtEnumTranslationMode::Mode QtEnumTranslationMode::toTranslationMode(const QString & translationMode){
	init();
	for (TranslationMode::const_iterator it = _mode.begin();
		it != _mode.end();
		++it) {

		if ((*it).second == translationMode) {
			return (*it).first;
		}
	}

	LOG_FATAL("unknown Translation Mode");
	return None;
}