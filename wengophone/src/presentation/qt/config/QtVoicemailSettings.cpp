/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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
#include "QtVoicemailSettings.h"

#include "ui_VoicemailSettings.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>

#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>

#include <QtGui/QtGui>

QtVoicemailSettings::QtVoicemailSettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone) {

	_ui = new Ui::VoicemailSettings();
	_ui->setupUi(this);

	readConfig();
}

QtVoicemailSettings::~QtVoicemailSettings() {
	OWSAFE_DELETE(_ui);
}

QString QtVoicemailSettings::getName() const {
	return tr("Voicemail");
}

QString QtVoicemailSettings::getTitle() const {
	return tr("Voicemail Settings");
}

QString QtVoicemailSettings::getDescription() const {
	return tr("Voicemail Settings");
}

QString QtVoicemailSettings::getIconName() const {
	return "voicemail";
}

void QtVoicemailSettings::saveConfig() {
	UserProfile * userprofile = _cWengoPhone.getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
	if (userprofile) {
		SipAccount * sipaccount = userprofile->getSipAccount();
		if (sipaccount) {
			sipaccount->setVoicemailNumber(_ui->voicemailNumber->text().toStdString());
		}
	}
}

void QtVoicemailSettings::readConfig() {
	std::string number = "123";

	UserProfile * userprofile = _cWengoPhone.getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
	if (userprofile) {
		SipAccount * sipaccount = userprofile->getSipAccount();
		if (sipaccount) {
			number = sipaccount->getVoicemailNumber();
		}
	} else {
		setEnabled(false);
	}

	_ui->voicemailNumber->setText(QString::fromStdString(number));
}
