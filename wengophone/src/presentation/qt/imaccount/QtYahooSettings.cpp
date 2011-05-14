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
#include "QtYahooSettings.h"

#include "ui_YahooSettings.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>

#include <util/WebBrowser.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

static const std::string YAHOO_FORGOT_PASSWORD_LINK_EN = "http://edit.yahoo.com/config/eval_forgot_pw?.src=pg&.done=http://messenger.yahoo.com/&.redir_from=MESSENGER";
static const std::string YAHOO_FORGOT_PASSWORD_LINK_FR = "http://edit.yahoo.com/config/eval_forgot_pw?.src=pg&.done=http://fr.messenger.yahoo.com&.intl=fr&.redir_from=MESSENGER";
static const std::string YAHOO_CREATE_NEW_ACCOUNT_LINK_EN = "http://login.yahoo.com/config/login?.done=http://messenger.yahoo.com&.src=pg";
static const std::string YAHOO_CREATE_NEW_ACCOUNT_LINK_FR = "https://edit.yahoo.com/config/eval_register?.intl=fr&.done=http://fr.messenger.yahoo.com&.src=pg";

QtYahooSettings::QtYahooSettings(UserProfile & userProfile, IMAccount & imAccount, QDialog * parent)
	: QtIMAccountPlugin(userProfile, imAccount, parent) {

	init();
}

QtYahooSettings::~QtYahooSettings() {
	OWSAFE_DELETE(_ui);
}

void QtYahooSettings::init() {
	_IMSettingsWidget = new QWidget(_parentDialog);

	_ui = new Ui::YahooSettings();
	_ui->setupUi(_IMSettingsWidget);
//VOXOX - CJC - 2009.06.02 
//	SAFE_CONNECT(_ui->forgotPasswordLabel, SIGNAL(linkActivated(const QString&)), SLOT(forgotPasswordButtonClicked()));

	SAFE_CONNECT(_ui->createAccountLabel, SIGNAL(linkActivated(const QString&)), SLOT(createAccountButtonClicked()));

	_ui->loginLineEdit->setText(QString::fromStdString(_imAccount.getLogin()));
	_ui->passwordLineEdit->setText(QString::fromStdString(_imAccount.getPassword()));
}

bool QtYahooSettings::isValid() const {
	return !_ui->loginLineEdit->text().isEmpty();
}

void QtYahooSettings::setIMAccount() {
	String login = _ui->loginLineEdit->text().toStdString();
	std::string password = _ui->passwordLineEdit->text().toStdString();

	static const String AT = "@";

	//Test if login ends with @
	if (login.contains(AT)) {
		std::string::size_type posAT = login.find(AT);
		if (posAT != std::string::npos) {
			login = login.substr(0, posAT);
		}
	}

	_imAccount.setLogin(login);
	_imAccount.setPassword(password);

	IMAccountParameters & params = _imAccount.getIMAccountParameters();
	params.set(IMAccountParameters::YAHOO_IS_JAPAN_KEY, _ui->useJapanServerCheckBox->isChecked());
	//FIXME to remove, must be done inside model
	params.set(IMAccountParameters::YAHOO_PORT_KEY, 23);
}

void QtYahooSettings::forgotPasswordButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string lang = config.getLanguage();

	if (lang == "fr") {
		WebBrowser::openUrl(YAHOO_FORGOT_PASSWORD_LINK_FR);
	} else {
		WebBrowser::openUrl(YAHOO_FORGOT_PASSWORD_LINK_EN);
	}
}

void QtYahooSettings::createAccountButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string lang = config.getLanguage();

	if (lang == "fr") {
		WebBrowser::openUrl(YAHOO_CREATE_NEW_ACCOUNT_LINK_FR);
	} else {
		WebBrowser::openUrl(YAHOO_CREATE_NEW_ACCOUNT_LINK_EN);
	}
}
