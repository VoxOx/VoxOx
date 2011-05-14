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
#include "QtAIMSettings.h"

#include "ui_AIMSettings.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>

#include <util/WebBrowser.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

static const std::string AIM_FORGOT_PASSWORD_LINK_EN = "http://www.aol.co.uk/aim/faqs/AIM_FAQ_Passwords.htm";
static const std::string AIM_FORGOT_PASSWORD_LINK_FR = "http://www.aim.aol.fr/oubli.htm";
static const std::string AIM_CREATE_NEW_ACCOUNT_LINK_EN = "https://my.screenname.aol.com/_cqr/login/login.psp?mcState=initialized&seamless=n&createSn=1&sitedomain=www.aim.com&siteState=http%3A//www.aim.com/get_aim/congratsd2.adp&triedAimAuth=y&promo=380464";
static const std::string AIM_CREATE_NEW_ACCOUNT_LINK_FR = "https://reg.my.screenname.aol.com/_cqr/registration/initRegistration.psp?mcState=initialized&seamless=n&createSn=1&siteId=aimregPROD-fr&siteState=http%3A%2F%2Faim%2Eaol%2Efr%2Fupgrade%2Ejsp&mcAuth=%2FBcAG0RtoewAAPdvAHyUj0RtoigIxtPj0NfjWwgAAA%3D%3D";

QtAIMSettings::QtAIMSettings(UserProfile & userProfile, IMAccount & imAccount, QDialog * parent)
	: QtIMAccountPlugin(userProfile, imAccount, parent) {

	init();
}

QtAIMSettings::~QtAIMSettings() {
	OWSAFE_DELETE(_ui);
}

void QtAIMSettings::init() {
	_IMSettingsWidget = new QWidget(_parentDialog);

	_ui = new Ui::AIMSettings();
	_ui->setupUi(_IMSettingsWidget);
//VOXOX - CJC - 2009.06.02 
//	SAFE_CONNECT(_ui->forgotPasswordLabel, SIGNAL(linkActivated(const QString&)), SLOT(forgotPasswordButtonClicked()));

	SAFE_CONNECT(_ui->createAccountLabel, SIGNAL(linkActivated(const QString&)), SLOT(createAccountButtonClicked()));

	_ui->loginLineEdit->setText(QString::fromStdString(_imAccount.getLogin()));
	_ui->passwordLineEdit->setText(QString::fromStdString(_imAccount.getPassword()));
}

void QtAIMSettings::setIMAccount() {
	_imAccount.setLogin(_ui->loginLineEdit->text().toStdString());
	_imAccount.setPassword(_ui->passwordLineEdit->text().toStdString());

	IMAccountParameters & params = _imAccount.getIMAccountParameters();
	//FIXME to remove, must be done inside model
	params.set(IMAccountParameters::OSCAR_PORT_KEY, 443);
}

bool QtAIMSettings::isValid() const {
	return !_ui->loginLineEdit->text().isEmpty();
}

void QtAIMSettings::forgotPasswordButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string lang = config.getLanguage();

	if (lang == "fr") {
		WebBrowser::openUrl(AIM_FORGOT_PASSWORD_LINK_FR);
	} else {
		WebBrowser::openUrl(AIM_FORGOT_PASSWORD_LINK_EN);
	}
}

void QtAIMSettings::createAccountButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string lang = config.getLanguage();

	if (lang == "fr") {
		WebBrowser::openUrl(AIM_CREATE_NEW_ACCOUNT_LINK_FR);
	} else {
		WebBrowser::openUrl(AIM_CREATE_NEW_ACCOUNT_LINK_EN);
	}
}
