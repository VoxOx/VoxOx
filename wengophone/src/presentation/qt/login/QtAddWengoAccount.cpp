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
#include "QtAddWengoAccount.h"

#include "QtAddAccountUtils.h"
#include "QtLoginDialog.h"

#include "ui_AddWengoAccount.h"

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

QtAddWengoAccount::QtAddWengoAccount(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler)
	:ILogin(qtLoginDialog, cUserProfileHandler){

	_ui = new Ui::AddWengoAccount();
	_ui->setupUi(this);

	LANGUAGE_CHANGE(this);

	WidgetBackgroundImage::setBackgroundImage(_ui->loginLabel, ":pics/headers/login.png", WidgetBackgroundImage::AdjustHeight);

	SAFE_CONNECT(_ui->loginButton, SIGNAL(clicked()), SLOT(loginClicked()));
	SAFE_CONNECT(_ui->backButton, SIGNAL(clicked()), SLOT(goBack()));
	SAFE_CONNECT_RECEIVER(_ui->cancelButton, SIGNAL(clicked()), _loginDialog, SLOT(reject()));
	SAFE_CONNECT(_ui->createWengoAccountLabel, SIGNAL(linkActivated(const QString &)), SLOT(createAccountButtonClicked()));
	SAFE_CONNECT(_ui->helpLabel, SIGNAL(linkActivated(const QString &)), SLOT(helpButtonClicked()));
	SAFE_CONNECT(_ui->forgotPasswordLabel, SIGNAL(linkActivated(const QString &)), SLOT(forgotPasswordButtonClicked()));
}

QtAddWengoAccount::~QtAddWengoAccount() {
	OWSAFE_DELETE(_ui);
}

void QtAddWengoAccount::createAccountButtonClicked() {
	WsUrl::showWengoAccountCreation();
}

void QtAddWengoAccount::helpButtonClicked() {
	WsUrl::showWikiPage();
}

void QtAddWengoAccount::forgotPasswordButtonClicked() {
	WsUrl::showLostPasswordPage();
}

void QtAddWengoAccount::loginClicked() {
	std::string login = _ui->email->text().toStdString();
	
	// check if login is an email address
	//VOXOX CHANGE
	//CJC
	//We dont need any @
	/*if (login.find("@", 0) == login.npos) {
	
		QMessageBox::warning(this, tr("@product@ - Connexion"),
			tr("Please enter an email address."),
			QMessageBox::Ok, QMessageBox::NoButton);
	
		return;
	}*/
	////
	
	std::string password = _ui->password->text().toStdString();
	if (!login.empty() && !password.empty()) {
		WengoAccount wengoAccount(login, password, true/*_ui->rememberPassword->isChecked()*/);
		wengoAccount.setVisibleName(login);

		// Looking for the selected profile
		if (!_cUserProfileHandler.userProfileExists(login)) {
			// The selected profile does not exist. Create it.
			UserProfile userProfile;

			//VOXOX CHANGE by Rolando 01-20-09 eliminated add test calls
			/*Config & config = ConfigManager::getInstance().getCurrentConfig();
			QtAddAccountUtils::addTestContacts(&userProfile,
				config.getWengoAudioTestCall(),
				config.getWengoVideoTestCall()
				);*/

			userProfile.setSipAccount(wengoAccount, false /* needInitialization */);
			UserProfileFileStorage userProfileStorage(userProfile);
			userProfileStorage.save(login);
		}
		_cUserProfileHandler.setCurrentUserProfile(login, wengoAccount);
		_loginDialog->accept();
	}
}

void QtAddWengoAccount::setInfoMessage() {
	setLoginLabel(tr("Please enter your email address<br/>and your password"));
}

void QtAddWengoAccount::setErrorMessage(const QString & message) {
	setLoginLabel(QString("<font color=\"red\">%1</font>").arg(message));
}

void QtAddWengoAccount::setLoginLabel(const QString & message) {
	QString loginLabel = QString("<span style=\"font-size:16pt;\">" +
                                  tr("Login") + "</span><br/>%1").arg(message);
	_ui->loginLabel->setText(loginLabel);
}

void QtAddWengoAccount::languageChanged() {
	_ui->retranslateUi(this);
}

void QtAddWengoAccount::load(const SipAccount & sipAccount) {
	if (sipAccount.getType() == SipAccount::SipAccountTypeWengo) {
		const WengoAccount & wengoAccount = dynamic_cast<const WengoAccount &>(sipAccount);
		_ui->email->setText(QString::fromStdString(wengoAccount.getWengoLogin()));
		_ui->password->setText(QString::fromStdString(wengoAccount.getWengoPassword()));
		setInfoMessage();
	} else {
		LOG_DEBUG("SIP getType() = SipAccount::SipAccountTypeBasic");
		//_loginDialog->changePage(QtLoginDialog::AddSIPAccount, sipAccount);
	}
}

void QtAddWengoAccount::load(std::string sipAccountName) {
	//load userprofile
	UserProfile * userProfile = _cUserProfileHandler.getUserProfileHandler().getUserProfile(sipAccountName);
	if (userProfile && userProfile->hasSipAccount()) {
		//tests account type
		if (userProfile->hasWengoAccount()) {
			//loads the wengo acccount
			WengoAccount * wengoAccount = userProfile->getWengoAccount();
			_ui->email->setText(QString::fromStdString(wengoAccount->getWengoLogin()));
			_ui->password->setText(QString::fromStdString(wengoAccount->getWengoPassword()));
			setInfoMessage();
		} else {
			//should be loaded as a sip account
			//_loginDialog->changePage(QtLoginDialog::AddSIPAccount, sipAccountName);//VOXOX CHANGE by Rolando 02-25-09
			_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount, sipAccountName);//VOXOX CHANGE by Rolando 02-25-09

		}
	} else {
		//_loginDialog->changePage(QtLoginDialog::AddAccount);//VOXOX CHANGE by Rolando 02-25-09
		_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount, sipAccountName);//VOXOX CHANGE by Rolando 02-25-09

	}
	OWSAFE_DELETE(userProfile);
}

void QtAddWengoAccount::initPage() {
	_ui->email->setFocus();
	_ui->loginButton->setDefault(true);
	setInfoMessage();
}

void QtAddWengoAccount::keyPressEvent(QKeyEvent *event) {
	if (event->key()==Qt::Key_Return) {
		loginClicked();
	}
}
