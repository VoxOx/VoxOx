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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWenboxPlugin.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/callbar/QtCallBar.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/wenbox/CWenboxPlugin.h>

#include <thread/Timer.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtWenboxPlugin::QtWenboxPlugin(CWenboxPlugin & cWenboxPlugin)
	: QObject(NULL),
	_cWenboxPlugin(cWenboxPlugin) {

	_timer = new Timer();
	_timer->timeoutEvent += boost::bind(&QtWenboxPlugin::timeoutEventHandler, this);
}

QtWenboxPlugin::~QtWenboxPlugin() {
	_timer->stop();
	OWSAFE_DELETE(_timer);
}

void QtWenboxPlugin::phoneNumberBufferUpdatedEvent(const std::string & phoneNumberBuffer) {
	static const unsigned TIMEOUT = 3 * 1000;	//3 seconds

	//FIXME we should avoid to get QtWengoPhone via CWenboxPlugin and it should
	//be given in constructor.
	QtWengoPhone * qtWengoPhone = (QtWengoPhone *) _cWenboxPlugin.getCWengoPhone().getPresentation();
	qtWengoPhone->getQtCallBar().setPhoneComboBoxEditText(phoneNumberBuffer);

	_timer->stop();
	if (!phoneNumberBuffer.empty()) {
		_timer->start(TIMEOUT, TIMEOUT);
	}
}

void QtWenboxPlugin::timeoutEventHandler() {
	_timer->stop();

	//FIXME we should avoid to get QtWengoPhone via CWenboxPlugin and it should
	//be given in constructor.
	QtWengoPhone * qtWengoPhone = (QtWengoPhone *) _cWenboxPlugin.getCWengoPhone().getPresentation();

	CWengoPhone & cWengoPhone = _cWenboxPlugin.getCWengoPhone();
	cWengoPhone.getCUserProfileHandler().getCUserProfile()->makeCall(qtWengoPhone->getQtCallBar().getPhoneComboBoxCurrentText());
}
