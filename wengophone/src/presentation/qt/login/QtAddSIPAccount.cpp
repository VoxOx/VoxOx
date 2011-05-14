/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "QtAddSIPAccount.h"

#include "QtAddAccountUtils.h"
#include "QtLoginDialog.h"

#include "ui_AddSIPAccount.h"

#include <presentation/qt/QtWengoPhone.h>

#include <control/profile/CUserProfileHandler.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/profile/UserProfileHandler.h>
#include <model/webservices/url/WsUrl.h>

#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtAddSIPAccount::QtAddSIPAccount(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler)
	:ILogin(qtLoginDialog, cUserProfileHandler)
	, _reducedHeight(0) {

	_ui = new Ui::AddSIPAccount();
	_ui->setupUi(this);

	LANGUAGE_CHANGE(this);

	WidgetBackgroundImage::setBackgroundImage(_ui->loginLabel, ":pics/headers/login.png", WidgetBackgroundImage::AdjustHeight);

	// mandatory lines
	SAFE_CONNECT(_ui->login, SIGNAL(textChanged(QString)), SLOT(aMandatoryLineHasChanged()));
	SAFE_CONNECT(_ui->realm, SIGNAL(textChanged(QString)), SLOT(aMandatoryLineHasChanged()));
	////
		
	SAFE_CONNECT(_ui->advButton, SIGNAL(toggled(bool)), SLOT(showAdvancedSettings(bool)));
	SAFE_CONNECT(_ui->loginButton, SIGNAL(clicked()), SLOT(loginClicked()));
	SAFE_CONNECT(_ui->backButton, SIGNAL(clicked()), SLOT(goBack()));
	SAFE_CONNECT_RECEIVER(_ui->cancelButton, SIGNAL(clicked()), _loginDialog, SLOT(reject()));

	showAdvancedSettings(false);

	aMandatoryLineHasChanged();
}

QtAddSIPAccount::~QtAddSIPAccount() {
	OWSAFE_DELETE(_ui);
}

void QtAddSIPAccount::loginClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	std::string accountname = _ui->accountname->text().toStdString();
	std::string login = _ui->login->text().toStdString();
	std::string password = _ui->password->text().toStdString();
	std::string realm = _ui->realm->text().toStdString();
	std::string displayname = _ui->displayname->text().toStdString();
	std::string registerServer = _ui->registerServer->text().toStdString();
	int registerPort = config.getSipRegisterPort();
	std::string proxyServer = _ui->proxyServer->text().toStdString();
	int proxyPort = _ui->proxyPort->text().toInt();

	//fill ampty lines
	if (password.empty()) {
		password = login;
	}
	if (registerServer.empty()) {
		registerServer = realm;
	}
	if (proxyServer.empty()) {
		proxyServer = registerServer;
	}
	if (proxyPort==0) {
		proxyPort = 5060;
	}
	if (accountname.empty()) {
		accountname = login + "@" + registerServer;
	}
	if (displayname.empty()) {
		displayname = accountname;
	}
	////

	//create sip account
	SipAccount sipAccount(login, login, password, realm,
		registerServer, registerPort, proxyServer, proxyPort,
		true/*_ui->rememberPassword->isChecked()*/,
		_ui->activateSIMPLE->isChecked());
	sipAccount.setDisplayName(displayname);
	sipAccount.setVisibleName(accountname);
	////
	
	// Looking for the selected profile
	if (!_cUserProfileHandler.userProfileExists(sipAccount.getFullIdentity())) {
		// The selected profile does not exist. Creating a new one.
		UserProfile userProfile;

		//VOXOX CHANGE by Rolando 01-20-09 eliminated add test calls
		/*Config & config = ConfigManager::getInstance().getCurrentConfig();
		QtAddAccountUtils::addTestContacts(&userProfile,
			config.getSipAudioTestCall(),
			config.getSipVideoTestCall()
			);*/

		userProfile.setSipAccount(sipAccount, false /* needInitialization */);
		UserProfileFileStorage userProfileStorage(userProfile);
		userProfileStorage.save(sipAccount.getFullIdentity());
	}
	_cUserProfileHandler.setCurrentUserProfile(sipAccount.getFullIdentity(), sipAccount);
	
	_loginDialog->accept();
}

void QtAddSIPAccount::setInfoMessage() {
	setLoginLabel(tr("Please fill in the information which will<br/>allow your softphone to connect to your<br/>SIP account"));
}

void QtAddSIPAccount::setErrorMessage(const QString & message) {
	setLoginLabel(QString("<font color=\"red\">%1</font>").arg(message));
}

void QtAddSIPAccount::setLoginLabel(const QString & message) {
	QString loginLabel = QString("<span style=\"font-size:16pt;\">" + 
                                  tr("Configure your SIP profile") + "</span><br/>%1").arg(message);
	_ui->loginLabel->setText(loginLabel);
}

void QtAddSIPAccount::languageChanged() {
	_ui->retranslateUi(this);
}

void QtAddSIPAccount::load(const SipAccount & sipAccount) {
	if (sipAccount.getType() == SipAccount::SipAccountTypeBasic) {
		//loads the sip acccount
		_ui->login->setText(QString::fromStdString(sipAccount.getIdentity()));
		_ui->password->setText(QString::fromStdString(sipAccount.getPassword()));
		_ui->realm->setText(QString::fromStdString(sipAccount.getRealm()));
		_ui->displayname->setText(QString::fromStdString(sipAccount.getDisplayName()));
		_ui->registerServer->setText(QString::fromStdString(sipAccount.getRegisterServerHostname()));
		_ui->proxyServer->setText(QString::fromStdString(sipAccount.getSIPProxyServerHostname()));
		//_ui->registerPort->setText(QString::number(sipAccount.getRegisterServerPort()));
		_ui->proxyPort->setText(QString::number(sipAccount.getSIPProxyServerPort()));
		_ui->accountname->setText(QString::fromStdString(sipAccount.getVisibleName()));
		_ui->activateSIMPLE->setChecked(sipAccount.isPIMEnabled());
		setInfoMessage();
	} else {
		LOG_DEBUG("SIP getType() = SipAccount::SipAccountTypeWengo");
		//_loginDialog->changePage(QtLoginDialog::AddWengoAccount, sipAccount);
	}
}

void QtAddSIPAccount::load(std::string sipAccountName) {
	//load userprofile
	UserProfile * userProfile = _cUserProfileHandler.getUserProfileHandler().getUserProfile(sipAccountName);
	if (userProfile && userProfile->hasSipAccount()) {
		//tests account type
		if (!userProfile->hasWengoAccount()) {
			SipAccount * sipAccount = userProfile->getSipAccount();
			//loads the sip acccount
			_ui->login->setText(QString::fromStdString(sipAccount->getIdentity()));
			_ui->password->setText(QString::fromStdString(sipAccount->getPassword()));
			_ui->realm->setText(QString::fromStdString(sipAccount->getRealm()));
			_ui->displayname->setText(QString::fromStdString(sipAccount->getDisplayName()));
			_ui->registerServer->setText(QString::fromStdString(sipAccount->getRegisterServerHostname()));
			_ui->proxyServer->setText(QString::fromStdString(sipAccount->getSIPProxyServerHostname()));
			//_ui->registerPort->setText(QString::number(sipAccount->getRegisterServerPort()));
			_ui->proxyPort->setText(QString::number(sipAccount->getSIPProxyServerPort()));
			_ui->accountname->setText(QString::fromStdString(sipAccount->getVisibleName()));
			_ui->activateSIMPLE->setChecked(sipAccount->isPIMEnabled());
			setInfoMessage();
		} else {
			//should be loaded as a wengo account
			//_loginDialog->changePage(QtLoginDialog::AddWengoAccount, sipAccountName);//VOXOX CHANGE by Rolando 02-25-09
			_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount, sipAccountName);//VOXOX CHANGE by Rolando 02-25-09


		}
	} else {
		//_loginDialog->changePage(QtLoginDialog::AddAccount);//VOXOX CHANGE by Rolando 02-25-09
		_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount);//VOXOX CHANGE by Rolando 02-25-09
	}
}

void QtAddSIPAccount::initPage() {
	_ui->accountname->setFocus();
	_ui->loginButton->setDefault(true);
	setInfoMessage();

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_ui->realm->setText(QString::fromStdString(config.getSipRealm()));
	_ui->registerServer->setText(QString::fromStdString(config.getSipRegisterServer()));
	_ui->proxyServer->setText(QString::fromStdString(config.getSipProxyServer()));
	_ui->proxyPort->setText(QString::number(config.getSipProxyPort()));
	_ui->activateSIMPLE->setChecked(config.getSipSimpleSupport());
}

void QtAddSIPAccount::aMandatoryLineHasChanged() {
	
	std::string login = _ui->login->text().toStdString();
	std::string realm = _ui->realm->text().toStdString();
	
	if (	!login.empty() &&
			!realm.empty() 
	) {
		_ui->loginButton->setEnabled(true);
	} else {
		_ui->loginButton->setEnabled(false);
	}
}


void QtAddSIPAccount::showAdvancedSettings(bool show) {
	if (show && _reducedHeight == 0) {
		// First time we show the advanced settins. Store dialog height, will
		// be used if the user hide the advanced settings
		_reducedHeight = _loginDialog->height();
	}

	_ui->frame->setVisible(show);
	_ui->label_server->setVisible(show);
	_ui->label_proxy->setVisible(show);
	_ui->registerServer->setVisible(show);
	_ui->proxyServer->setVisible(show);
	_ui->label_proxyport->setVisible(show);
	_ui->proxyPort->setVisible(show);

	if (!show && _reducedHeight != 0) {
		// Make sure height is reduced to the size before expanding settings
		_loginDialog->setMinimumHeight(_reducedHeight);
		_loginDialog->resize(_loginDialog->width(), _reducedHeight);
	}

	if (show) {
		// If we try to show advanced settings a second time, the height will
		// remain _reducedHeight, this code makes sure we get back to the old
		// size
		this->layout()->invalidate();
		_loginDialog->layout()->invalidate();
		int height = _loginDialog->layout()->minimumSize().height();
		_loginDialog->resize(_loginDialog->width(), height);
	}
}
