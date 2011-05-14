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
#include "QtLogAccount.h"
#include "QtLoginDialog.h"

#include "ui_LogAccount.h"

#include <presentation/qt/QtWengoPhone.h>

#include <control/profile/CUserProfileHandler.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>
#include <model/webservices/url/WsUrl.h>

#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtLogAccount::QtLogAccount(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler)
	:ILogin(qtLoginDialog, cUserProfileHandler){

	_ui = new Ui::LogAccount();
	_ui->setupUi(this);

	LANGUAGE_CHANGE(this);

	WidgetBackgroundImage::setBackgroundImage(_ui->loginLabel, ":pics/headers/login.png", WidgetBackgroundImage::AdjustHeight);

	SAFE_CONNECT(_ui->createNewAccount, SIGNAL(clicked()), SLOT(createAccountButtonClicked()));
	SAFE_CONNECT(_ui->loginButton, SIGNAL(clicked()), SLOT(loginClicked()));
	SAFE_CONNECT_RECEIVER(_ui->cancelButton, SIGNAL(clicked()), _loginDialog, SLOT(reject()));
	SAFE_CONNECT(_ui->modifyButton, SIGNAL(clicked()), SLOT(modifyClicked()));
	SAFE_CONNECT(_ui->helpLabel, SIGNAL(linkActivated(const QString &)), SLOT(helpButtonClicked()));
	SAFE_CONNECT(_ui->forgotPasswordLabel, SIGNAL(linkActivated(const QString &)), SLOT(forgotPasswordButtonClicked()));

	//VOXOX CHANGE for VoxOx by Rolando 01-09-09, we will use it in any case
//#ifndef CUSTOM_ACCOUNT
//	_ui->forgotPasswordLabel->hide();
//#endif

	_ui->forgotPasswordLabel->hide();
}

QtLogAccount::~QtLogAccount() {
	OWSAFE_DELETE(_ui);
}

void QtLogAccount::setValidAccount(SipAccount sipAccount) {
	ILogin::load(sipAccount);
	setInfoMessage();
}

void QtLogAccount::createAccountButtonClicked() {
	//_loginDialog->changePage(QtLoginDialog::AddAccount);//VOXOX CHANGE by Rolando 
	_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount);//VOXOX CHANGE by Rolando 
}

void QtLogAccount::helpButtonClicked() {
	WsUrl::showWikiPage();
}

void QtLogAccount::forgotPasswordButtonClicked() {
	WsUrl::showLostPasswordPage();
}

void QtLogAccount::setAccountName(const QString & name) {
	_ui->accountComboBox->setEditText(name);
}

void QtLogAccount::setPassword(const QString & password) {
	_ui->passwordLineEdit->setText(password);
}

void QtLogAccount::setRememberPassword(bool /*remember*/) {
}

void QtLogAccount::initPage() {
	_ui->accountComboBox->clear();
	std::vector<std::string> profileNames = _cUserProfileHandler.getUserProfileNames(true);
	
	if (profileNames.size() > 0) {

		std::string lastUsed = ConfigManager::getInstance().getCurrentConfig().getProfileLastUsedName();
		std::string lastUsedIndex = "";

		// finds all userprofiles
		for (std::vector<std::string>::const_iterator it = profileNames.begin();
			it != profileNames.end();
			++it) {

			UserProfile userProfile;
			std::string tmpname;
			SipAccount * sipaccount = NULL;

			//gets userprofile
			if (_cUserProfileHandler.getUserProfileHandler().LoadUserProfileFromName(&userProfile, *it)) {
				
				//gets sipaccount
				sipaccount = userProfile.getSipAccount();
				if (sipaccount) {
					
					//finds and displays visible name, saves real userprofile name
					tmpname = sipaccount->getVisibleName();
					profileMap[tmpname] = (*it);
					_ui->accountComboBox->addItem(QString::fromStdString(tmpname));
					
					//finds last used index
					if ((*it)==lastUsed) {
						lastUsedIndex = tmpname;
					}
				}
			}
		}
		////
		
		//default value
		_ui->accountComboBox->setCurrentIndex(_ui->accountComboBox->findText(QString::fromStdString(lastUsedIndex)));
		currentIndexChanged(QString::fromStdString(lastUsedIndex));
		
		SAFE_CONNECT(_ui->accountComboBox, SIGNAL(currentIndexChanged(const QString &)), 
			SLOT(currentIndexChanged(const QString &)));
	} else {
		//_loginDialog->changePage(QtLoginDialog::AddAccount);//VOXOX CHANGE by Rolando
		_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount);//VOXOX CHANGE by Rolando 
		return;
	}

	_ui->accountComboBox->setFocus();
	_ui->loginButton->setDefault(true);
	setInfoMessage();
	_ui->passwordLineEdit->setEnabled(false);
}

void QtLogAccount::currentIndexChanged(const QString & profileName) {
	UserProfile userProfile;
	if (_cUserProfileHandler.getUserProfileHandler().LoadUserProfileFromName(&userProfile, profileMap[profileName.toStdString()])) {
		SipAccount * sipaccount = userProfile.getSipAccount();
		if (sipaccount) {
			if (sipaccount->isPasswordRemembered()) {
				setPassword(QString::fromStdString(sipaccount->getUserPassword()));
				setRememberPassword(true);
			} else {
				setPassword(QString::fromStdString(""));
				setRememberPassword(false);
			}
		}
	}
}

void QtLogAccount::loginClicked() {

	std::string login = profileMap[ _ui->accountComboBox->currentText().toStdString() ];

	/*** et le password !! ***/
	if (!login.empty()) {

		// Looking for the selected profile
		if (!_cUserProfileHandler.userProfileExists(login)) {
			// The selected profile does not exist. normally should NOT happen !!
			setErrorMessage(tr("Invalid account"));
		} else {
			_cUserProfileHandler.setCurrentUserProfile(login, SipAccount::empty);
		}

		_loginDialog->accept();
	}
}

void QtLogAccount::modifyClicked() {

	std::string login = profileMap[ _ui->accountComboBox->currentText().toStdString() ];

	if (!login.empty()) {

		// Looking for the selected profile
		if (!_cUserProfileHandler.userProfileExists(login)) {
			// The selected profile does not exist. normally should NOT happen !!
			//_loginDialog->changePage(QtLoginDialog::AddAccount);//VOXOX CHANGE by Rolando
			_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount);//VOXOX CHANGE by Rolando
		} else {

			UserProfile * userProfile = _cUserProfileHandler.getUserProfileHandler().getUserProfile(login);

			if (userProfile) {
				if (userProfile->hasWengoAccount())
				{
					//wengo account
					//_loginDialog->changePage(QtLoginDialog::AddWengoAccount, login);//VOXOX CHANGE by Rolando
					_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount, login);//VOXOX CHANGE by Rolando
				} else if (userProfile->hasSipAccount()) {
					//"generic" sip account
					//_loginDialog->changePage(QtLoginDialog::AddSIPAccount, login);//VOXOX CHANGE by Rolando
					_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount, login);//VOXOX CHANGE by Rolando
				}
				//_loginDialog->currentPage()->setBackPage(QtLoginDialog::LogAccount);//VOXOX CHANGE by Rolando
				_loginDialog->currentPage()->setBackPage(QtLoginDialog::AddVoxOxAccount);//VOXOX CHANGE by Rolando
			}
		}
	}
}

void QtLogAccount::setInfoMessage() {
	setLoginLabel(tr("Please choose a previously used profile<br/> or fill in the informations to connect to a<br/> @company@ account"));
}

void QtLogAccount::setErrorMessage(const QString & message) {
	setLoginLabel(QString("<font color=\"red\">%1</font>").arg(message));
}

void QtLogAccount::setLoginLabel(const QString & message) {
	QString loginLabel = QString("<span style=\"font-size:16pt;\">Login</span><br/>%1").arg(message);
	_ui->loginLabel->setText(loginLabel);
}

void QtLogAccount::languageChanged() {
	_ui->retranslateUi(this);
}

void QtLogAccount::load(std::string sipAccountName) {

	//load userprofile
	UserProfile * userProfile = _cUserProfileHandler.getUserProfileHandler().getUserProfile(sipAccountName);
	if (userProfile) {
		// Add and select the given SipAccount
		_ui->accountComboBox->addItem(QString::fromStdString(sipAccountName));
		_ui->accountComboBox->setCurrentIndex(_ui->accountComboBox->findText(QString::fromStdString(sipAccountName)));
		setInfoMessage();

		SipAccount * sipAccount = userProfile->getSipAccount();
		setPassword(QString::fromStdString(sipAccount->getUserPassword()));
	} else {
		//_loginDialog->changePage(QtLoginDialog::AddAccount);//VOXOX CHANGE by Rolando
		_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount);//VOXOX CHANGE by Rolando
	}
	
	OWSAFE_DELETE(userProfile);
}
