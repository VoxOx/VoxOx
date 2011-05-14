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
#include "QtMySpaceSettings.h"

#include "ui_MySpaceSettings.h"

#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>

#include <util/WebBrowser.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

static const std::string MYSPACE_CREATE_NEW_ACCOUNT_LINK_EN = "http://signups.myspace.com/index.cfm?fuseaction=signup";

QtMySpaceSettings::QtMySpaceSettings(UserProfile & userProfile, IMAccount & imAccount, QDialog * parent)
	: QtIMAccountPlugin(userProfile, imAccount, parent) {

	init();
}

QtMySpaceSettings::~QtMySpaceSettings() {
	OWSAFE_DELETE(_ui);
}

void QtMySpaceSettings::init() {
	_IMSettingsWidget = new QWidget(_parentDialog);

	_ui = new Ui::MySpaceSettings();
	_ui->setupUi(_IMSettingsWidget);

	SAFE_CONNECT(_ui->createAccountLabel, SIGNAL(linkActivated(const QString&)), SLOT(createAccountButtonClicked()));

	_ui->loginLineEdit->setText(QString::fromStdString(_imAccount.getLogin()));
	_ui->passwordLineEdit->setText(QString::fromStdString(_imAccount.getPassword()));
}

void QtMySpaceSettings::setIMAccount() {
	_imAccount.setLogin(_ui->loginLineEdit->text().toStdString());
	_imAccount.setPassword(_ui->passwordLineEdit->text().toStdString());

}

bool QtMySpaceSettings::isValid() const {
	return !_ui->loginLineEdit->text().isEmpty();
}


void QtMySpaceSettings::createAccountButtonClicked() {
	WebBrowser::openUrl(MYSPACE_CREATE_NEW_ACCOUNT_LINK_EN);
}
