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
#include "CDtmfThemeManager.h"
#include <model/dtmf/DtmfThemeManager.h>

#include <presentation/PFactory.h>

#include <control/CWengoPhone.h>

#include <model/WengoPhone.h>

#include <util/Logger.h>
#include <thread/ThreadEvent.h>

CDtmfThemeManager::CDtmfThemeManager(DtmfThemeManager & dtmfThemeManager, CWengoPhone & cWengoPhone)
	: _dtmfThemeManager(dtmfThemeManager),
	_cWengoPhone(cWengoPhone) {

	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CDtmfThemeManager::initPresentationThreadSafe, this));
	PFactory::postEvent(event);
}

CDtmfThemeManager::~CDtmfThemeManager() {
	//delete _dtmfThemeManager;
}

void CDtmfThemeManager::initPresentationThreadSafe() {
	PFactory::getFactory().createPresentationDtmfThemeManager(*this);
}

Presentation * CDtmfThemeManager::getPresentation() const {
	return 0;
}

CWengoPhone & CDtmfThemeManager::getCWengoPhone() const {
	return _cWengoPhone;
}

void CDtmfThemeManager::playTone(const std::string & themeName, const std::string & key) const {

	//get the theme
	const DtmfTheme* theme = _dtmfThemeManager.getDtmfTheme(themeName);

	//play the tone
	if (theme) {
		theme->playTone(key);
	} else {
		LOG_WARN("This theme does not exist !");
	}
}

StringList CDtmfThemeManager::getThemeList() const {
	return _dtmfThemeManager.getThemeList();
}

const DtmfTheme * CDtmfThemeManager::getDtmfTheme(const std::string & themeName) const {
	return _dtmfThemeManager.getDtmfTheme(themeName);
}

bool CDtmfThemeManager::refreshDtmfThemes() {
	return _dtmfThemeManager.refreshDtmfThemes();
}
