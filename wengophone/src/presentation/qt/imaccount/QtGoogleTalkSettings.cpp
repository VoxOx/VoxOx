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
#include "QtGoogleTalkSettings.h"

#include "ui_GoogleTalkSettings.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>

#include <util/WebBrowser.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

static const std::string GOOGLETALK_SERVER						= "talk.google.com";
static const std::string GOOGLETALK_LOGIN_EXTENSION				= "gmail.com";
static const std::string GOOGLETALK_FORGOT_PASSWORD_LINK_EN		= "https://www.google.com/accounts/ForgotPasswd?hl=en&continue=http%3A%2F%2Fmail.google.com";
static const std::string GOOGLETALK_FORGOT_PASSWORD_LINK_FR		= "https://www.google.com/accounts/ForgotPasswd?hl=fr&continue=http%3A%2F%2Fmail.google.com";
static const std::string GOOGLETALK_CREATE_NEW_ACCOUNT_LINK_EN	= "http://www.google.com/talk/index.html";
static const std::string GOOGLETALK_CREATE_NEW_ACCOUNT_LINK_FR	= "http://www.google.com/talk/intl/fr/";
static const int GOOGLETALK_PORT = 80;

QtGoogleTalkSettings::QtGoogleTalkSettings(UserProfile & userProfile, IMAccount & imAccount, QDialog * parent)
	: QtIMAccountPlugin(userProfile, imAccount, parent) {

	init();
}

QtGoogleTalkSettings::~QtGoogleTalkSettings() {
	OWSAFE_DELETE(_ui);
}

void QtGoogleTalkSettings::init() {
	_IMSettingsWidget = new QWidget(_parentDialog);

	_ui = new Ui::GoogleTalkSettings();
	_ui->setupUi(_IMSettingsWidget);
//VOXOX - CJC - 2009.06.02 
//	SAFE_CONNECT(_ui->forgotPasswordLabel, SIGNAL(linkActivated(const QString&)), SLOT(forgotPasswordButtonClicked()));

	SAFE_CONNECT(_ui->createAccountLabel, SIGNAL(linkActivated(const QString&)), SLOT(createAccountButtonClicked()));

	_ui->loginLineEdit->setText(QString::fromStdString(_imAccount.getLogin()));
	_ui->passwordLineEdit->setText(QString::fromStdString(_imAccount.getPassword()));
}

void QtGoogleTalkSettings::setIMAccount() {
	static const String AT = "@";
	String tmpLogin = _ui->loginLineEdit->text().toStdString();

	//Test if login ends with @gmail.com
	if (!tmpLogin.contains(AT)) {
		tmpLogin = tmpLogin + AT + GOOGLETALK_LOGIN_EXTENSION;
	}

	_imAccount.setLogin(tmpLogin);
	_imAccount.setPassword(_ui->passwordLineEdit->text().toStdString());

	IMAccountParameters & params = _imAccount.getIMAccountParameters();
	params.set(IMAccountParameters::JABBER_USE_TLS_KEY, true);
	params.set(IMAccountParameters::JABBER_CONNECTION_SERVER_KEY, GOOGLETALK_SERVER);
	params.set(IMAccountParameters::JABBER_PORT_KEY, GOOGLETALK_PORT);
}

bool QtGoogleTalkSettings::isValid() const {
	return !_ui->loginLineEdit->text().isEmpty();
}

void QtGoogleTalkSettings::forgotPasswordButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string lang = config.getLanguage();

	if (lang == "fr") {
		WebBrowser::openUrl(GOOGLETALK_FORGOT_PASSWORD_LINK_FR);
	} else {
		WebBrowser::openUrl(GOOGLETALK_FORGOT_PASSWORD_LINK_EN);
	}
}

void QtGoogleTalkSettings::createAccountButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string lang = config.getLanguage();

	if (lang == "fr") {
		WebBrowser::openUrl(GOOGLETALK_CREATE_NEW_ACCOUNT_LINK_FR);
	} else {
		WebBrowser::openUrl(GOOGLETALK_CREATE_NEW_ACCOUNT_LINK_EN);
	}
}
