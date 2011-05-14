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
#include "QtVoxOxLogin.h"

#include "ui_AddVoxOxAccount.h"

#include <presentation/qt/QtWengoPhone.h>

#include <presentation/qt/webbrowser/QtWebBrowser.h>

#include <control/profile/CUserProfileHandler.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/profile/UserProfileHandler.h>
#include <model/webservices/url/WsUrl.h>
#include <qtutil/PixmapMerging.h>
#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/ImageSelector.h>//VOXOX CHANGE by Rolando - 2009.05.25  delete this
#include <qtutil/DesktopService.h>//VOXOX CHANGE by Rolando - 2009.05.25 delete this
#include <util/WebBrowser.h>

const QSize LOST_PASSWORD_WINDOW_SIZE = QSize(400,330);
const QSize HELP_WINDOW_SIZE = QSize(350,400);
const QSize SIGN_UP_WINDOW_SIZE = QSize(350,530);

//VOXOX CHANGE by Rolando 05-05-09
QtVoxOxLogin::QtVoxOxLogin(QWidget* parent, QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler)
	: QWidget(NULL),
	_cUserProfileHandler(cUserProfileHandler),
	_qtWengoPhone(qtWengoPhone)
	{

	_qtWengoPhone = qtWengoPhone;

	_ui = new Ui::AddVoxOxAccount();
	_ui->setupUi(this);
	
	LANGUAGE_CHANGE(this);

	//WidgetBackgroundImage::setBackgroundImage(_ui->loginLabel, ":pics/headers/login.png", WidgetBackgroundImage::AdjustHeight);

	SAFE_CONNECT(_ui->loginButton, SIGNAL(clicked()), SLOT(loginClicked()));
	SAFE_CONNECT(_ui->linkToSignUpLabel, SIGNAL(linkActivated(const QString &)), SLOT(linkToSignUpLabelClicked()));
	//SAFE_CONNECT(_ui->helpLabel, SIGNAL(linkActivated(const QString &)), SLOT(helpButtonClicked()));
	SAFE_CONNECT(_ui->linkForgotPasswordLabel, SIGNAL(linkActivated(const QString &)), SLOT(linkForgotPasswordLabelClicked()));
	SAFE_CONNECT(_ui->publicComputerCheckBox, SIGNAL(stateChanged(int)), SLOT(publicComputerStateChanged(int)));
	SAFE_CONNECT(_ui->rememberPasswordCheckBox, SIGNAL(stateChanged(int)), SLOT(rememberPasswordStateChanged(int)));
	SAFE_CONNECT(_ui->usernameComboBox, SIGNAL(activated(int)), SLOT(itemActivatedComboBoxSlot(int)));

	initPage();	
}

QtVoxOxLogin::~QtVoxOxLogin() {
	OWSAFE_DELETE(_ui);
}
//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLogin::linkToSignUpLabelClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();//VOXOX - CJC - 2009.06.29 
	//openWengoUrlWithoutAuth(config.getAccountCreationUrl());//VOXOX - CJC - 2009.06.29 

	WebBrowser::openUrl(config.getAccountCreationUrl());

}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLogin::helpButtonClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();//VOXOX - CJC - 2009.06.29 
	//openWengoUrlWithoutAuth(config.getAccountCreationUrl());//VOXOX - CJC - 2009.06.29 

	WebBrowser::openUrl(config.getWikiUrl());
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLogin::linkForgotPasswordLabelClicked() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();//VOXOX - CJC - 2009.06.29 
	//openWengoUrlWithoutAuth(config.getAccountCreationUrl());//VOXOX - CJC - 2009.06.29 

	WebBrowser::openUrl(config.getLostPasswordUrl());

}
//VOXOX CHANGE by Rolando 04-06-09
void QtVoxOxLogin::publicComputerStateChanged(int){
	if(_ui->publicComputerCheckBox->isChecked()){		
		_ui->rememberPasswordFrame->hide();
		_ui->rememberPasswordCheckBox->setChecked(false);
		_ui->automaticallyLoginCheckBox->setChecked(false);
	}
	else{
		_ui->rememberPasswordFrame->show();
	}
}

void QtVoxOxLogin::rememberPasswordStateChanged(int){
	if(_ui->rememberPasswordCheckBox->isChecked()){		
		_ui->publicComputerFrame->hide();
		_ui->publicComputerCheckBox->setChecked(false);		
	}
	else{		
		_ui->publicComputerFrame->show();
	}
}

void QtVoxOxLogin::itemActivatedComboBoxSlot(int index) {

	QString profileName = _ui->usernameComboBox->itemText(index);
	
	UserProfile userProfile;
	if (_cUserProfileHandler.getUserProfileHandler().LoadUserProfileFromName(&userProfile, _profileMap[profileName.toStdString()])) {
		SipAccount * sipAccount = userProfile.getSipAccount();
		if (sipAccount) {
			if (sipAccount->isPasswordRemembered()) {
				setPassword(sipAccount->getUserPassword());
				
				_ui->rememberPasswordCheckBox->setCheckState(Qt::Checked);
			} else {
				setPassword("");				
				_ui->rememberPasswordCheckBox->setCheckState(Qt::Unchecked);
			}

			_ui->publicComputerCheckBox->setChecked(!sipAccount->isUsernameRemembered());					

			_ui->automaticallyLoginCheckBox->setChecked(sipAccount->isAutoLoginVoxOx());			
		}
	}
	//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
	//updateAvatarLabel(profileName.toStdString());
}

void QtVoxOxLogin::loginClicked() 
{
	std::string login    = _ui->usernameComboBox->currentText().toStdString();
	std::string password = _ui->passwordLineEdit->text().toStdString();

	if (!login.empty() && !password.empty()) 
	{
		WengoAccount wengoAccount(login, password, _ui->rememberPasswordCheckBox->isChecked());

		wengoAccount.setVisibleName(login);
		wengoAccount.setRememberPassword(_ui->rememberPasswordCheckBox->isChecked());
		wengoAccount.setRememberUsername(_ui->rememberPasswordCheckBox->isChecked());
		wengoAccount.setAutoLoginVoxOx  (_ui->automaticallyLoginCheckBox->isChecked());

		// Looking for the selected profile
		if (!_cUserProfileHandler.userProfileExists(login)) 
		{
			// The selected profile does not exist. Creating a new one.
			UserProfile userProfile;
			userProfile.setSipAccount(wengoAccount, false /* needInitialization */);

			UserProfileFileStorage userProfileStorage(userProfile);
			userProfileStorage.save(login);
		}

		bool isInvisibleChecked = _ui->invisibleSignInCheckBox->isChecked();

		_cUserProfileHandler.setCurrentUserProfile(login, wengoAccount, isInvisibleChecked);	
		acceptLoginClicked(QString::fromStdString(login));
		_qtWengoPhone->showLoadingLoginMessage(QString::fromStdString(login));
	}	
}


void QtVoxOxLogin::setPassword(std::string password) {
	_ui->passwordLineEdit->setText(QString::fromStdString(password));
}

void QtVoxOxLogin::setAccountName(const QString & name) {
	_ui->usernameComboBox->setCurrentIndex(_ui->usernameComboBox->findText(name));
	//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
	//updateAvatarLabel(name.toStdString());
}

void QtVoxOxLogin::setInfoMessage() {
	
}

void QtVoxOxLogin::setErrorMessage(const QString & message) {
	
}

void QtVoxOxLogin::setLoginLabel(const QString & message) {	
}

void QtVoxOxLogin::languageChanged() {
	_ui->retranslateUi(this);
}

void QtVoxOxLogin::load(const SipAccount & sipAccount) {
	if (sipAccount.getType() == SipAccount::SipAccountTypeWengo) {
		const WengoAccount & wengoAccount = dynamic_cast<const WengoAccount &>(sipAccount);
		//setAccountName(QString::fromStdString(wengoAccount.getWengoLogin()));
		//setPassword(wengoAccount.getWengoPassword());
		//VOXOX CHANGE by Rolando 04-06-09	
		if (sipAccount.isPasswordRemembered()) {
			setPassword(sipAccount.getUserPassword());
			_ui->rememberPasswordCheckBox->setCheckState(Qt::Checked);
		} else {
			setPassword("");
			_ui->rememberPasswordCheckBox->setCheckState(Qt::Unchecked);
		}

		if (!sipAccount.isUsernameRemembered()) {		
			setAccountName("");
			setPassword("");
			_ui->rememberPasswordCheckBox->setCheckState(Qt::Unchecked);
			//_ui->publicComputerCheckBox->setCheckState(Qt::Checked);
		} else {
			setAccountName(QString::fromStdString(wengoAccount.getWengoLogin()));
			if (_ui->usernameComboBox->findText(QString::fromStdString(wengoAccount.getWengoLogin()))== -1){
					_ui->usernameComboBox->addItem(QString::fromStdString(wengoAccount.getWengoLogin()));
				}
			_ui->publicComputerCheckBox->setCheckState(Qt::Unchecked);
			
		}

		_ui->automaticallyLoginCheckBox->setChecked(sipAccount.isAutoLoginVoxOx());

		

	} else {
		LOG_DEBUG("SIP getType() = SipAccount::SipAccountTypeBasic");
	}
}

void QtVoxOxLogin::load(std::string sipAccountName, bool isARetry) {
	//load userprofile
	UserProfile * userProfile = _cUserProfileHandler.getUserProfileHandler().getUserProfile(sipAccountName);
	if (userProfile) {
		SipAccount * sipAccount = userProfile->getSipAccount();
		//VOXOX CHANGE by Rolando 04-06-09
		if (sipAccount) {
			
			if (sipAccount->isPasswordRemembered()) {
				setPassword(sipAccount->getUserPassword());
				_ui->rememberPasswordCheckBox->setCheckState(Qt::Checked);
			} else {
				setPassword("");
				_ui->rememberPasswordCheckBox->setCheckState(Qt::Unchecked);
			}

			if (!sipAccount->isUsernameRemembered()) {		
				setAccountName("");
				setPassword("");
				_ui->rememberPasswordCheckBox->setCheckState(Qt::Unchecked);
				/*_ui->publicComputerCheckBox->setCheckState(Qt::Checked);*/
			} else {
				// Add and select the given SipAccount
				//SERGIO MARIN IF EXISTS ACCOUNT IN COMBO BOX
				if (_ui->usernameComboBox->findText(QString::fromStdString(sipAccountName))== -1){
					_ui->usernameComboBox->addItem(QString::fromStdString(sipAccountName));
				}

				setAccountName(QString::fromStdString(sipAccountName));

				_ui->publicComputerCheckBox->setCheckState(Qt::Unchecked);
				
			}
			
			if(isARetry){
				setLastEnteredName(sipAccountName);
				_ui->passwordLineEdit->setFocus();
			}

			_ui->automaticallyLoginCheckBox->setChecked(sipAccount->isAutoLoginVoxOx());

		}
	}
	else {
		setLastEnteredName(sipAccountName);
	}
}

void QtVoxOxLogin::setLastEnteredName(std::string profileName){
	if(profileName != ""){
		_ui->usernameComboBox->setEditText(QString::fromStdString(profileName));
	}
	//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
	//updateAvatarLabel(profileName);
}

void QtVoxOxLogin::initPage() {

	_ui->usernameComboBox->clear();
	_ui->usernameComboBox->setFocus();
	_ui->loginButton->setDefault(true);

	std::vector<std::string> profileNames = _cUserProfileHandler.getUserProfileNames(true);
	//VOXOX CHANGE by Rolando 04-06-09
	if (profileNames.size() > 0) {

		// finds all userprofiles
		for (std::vector<std::string>::const_iterator it = profileNames.begin();
			it != profileNames.end();
			++it) {

			UserProfile userProfile;
			std::string tmpname;
			SipAccount * sipAccount = NULL;

			//gets userprofile
			if (_cUserProfileHandler.getUserProfileHandler().LoadUserProfileFromName(&userProfile, *it)) {
				
				//gets sipaccount
				sipAccount = userProfile.getSipAccount();
				if (sipAccount) {
					if(sipAccount->isUsernameRemembered()){
						//finds and displays visible name, saves real userprofile name
						tmpname = sipAccount->getVisibleName();
						_profileMap[tmpname] = (*it);
						if (_ui->usernameComboBox->findText(QString::fromStdString(tmpname))== -1){
							_ui->usernameComboBox->addItem(QString::fromStdString(tmpname));
						}											
					}									
				}
			}
		}
		////
		
		//default value
		if(_ui->usernameComboBox->count() > 0){
			_ui->usernameComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
			/*_ui->usernameComboBox->setCurrentIndex(0);
			currentIndexChanged(_ui->usernameComboBox->currentText());*/
			Config & config = ConfigManager::getInstance().getCurrentConfig();
			QString lastLoggedAccount = QString::fromStdString(config.getProfileLastLoggedAccount());
			//setAccountName(lastLoggedAccount);
			if(lastLoggedAccount != ""){
				load(lastLoggedAccount.toStdString());//VOXOX CHANGE by Rolando 04-06-09
			}
			else{//VOXOX CHANGE by Rolando 04-06-09
				load(_ui->usernameComboBox->currentText().toStdString());
			}
		}
		else{
			//linkToSignUpLabelClicked();//TODO: VOXOX CHANGE by Rolando 05-04-09, uncomment this sentence when openssl is compiled in VoxOx
			//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
			//updateAvatarLabel("");
		}
		
	} 
	else {
		//VOXOX CHANGE by Rolando 04-06-09
		//linkToSignUpLabelClicked();//TODO: VOXOX CHANGE by Rolando 05-04-09, uncomment this sentence when openssl is compiled in VoxOx
		//updateAvatarLabel("");//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
	}	

	_ui->usernameComboBox->setFocus();
	_ui->loginButton->setDefault(true);

}
//VOXOX CHANGE by Rolando 04-06-09
void QtVoxOxLogin::keyPressEvent(QKeyEvent *event) {
	if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
		loginClicked();
	}
}
