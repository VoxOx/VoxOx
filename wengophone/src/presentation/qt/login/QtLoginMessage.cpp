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
#include "QtLoginMessage.h"

#include "QtAddAccountUtils.h"
#include "QtLoginDialog.h"

#include "ui_LoginMessage.h"

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

#include <qtutil/PixmapMerging.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtLoginMessage::QtLoginMessage(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler)
	:ILogin(qtLoginDialog, cUserProfileHandler){

	_ui = new Ui::LoginMessage();
	_ui->setupUi(this);

	_profileName = "";
	
	LANGUAGE_CHANGE(this);
		
	SAFE_CONNECT(_ui->tryAgainButton, SIGNAL(clicked()), SLOT(tryAgainButtonClicked()));
	resize(QSize(279,380));
}

QtLoginMessage::~QtLoginMessage() {
	OWSAFE_DELETE(_ui);
}


void QtLoginMessage::helpButtonClicked() {
	WsUrl::showWikiPage();
}



void QtLoginMessage::tryAgainButtonClicked() {
	_loginDialog->changePage(QtLoginDialog::AddVoxOxAccount,_profileName);
	//_loginDialog->accept();
	
}

void QtLoginMessage::setInfoMessage() {
	setLoginLabel(tr("Please enter your email address<br/>and your password"));
}

void QtLoginMessage::setErrorMessage(const QString & message) {
	/*setLoginLabel(QString("<font color=\"red\">%1</font>").arg(message));*/
	setLoginLabel(message);	
}

void QtLoginMessage::setAdviceMessage(const QString & message) {
	/*setLoginLabel(QString("<font color=\"red\">%1</font>").arg(message));*/
	_ui->loginMessageAdvice->setText(message);
}

void QtLoginMessage::setLoginLabel(const QString & message) {
	/*QString loginMessageError = QString("<span style=\"font-size:16pt;\">" +
                                  tr("Login") + "</span><br/>%1").arg(message);*/
	_ui->loginMessageError->setText(message);
}


void QtLoginMessage::setProfileName(const QString & profileName) {
	_profileName = profileName.toStdString();
	//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
	//updateAvatarLabel();
}

void QtLoginMessage::languageChanged() {
	_ui->retranslateUi(this);
}

void QtLoginMessage::load(const SipAccount & sipAccount) {
	setInfoMessage();
}

void QtLoginMessage::load(std::string sipAccountName) {
	setInfoMessage();
}

void QtLoginMessage::initPage() {
	_ui->tryAgainButton->setDefault(true);
	setInfoMessage();
}

void QtLoginMessage::keyPressEvent(QKeyEvent *event) {
	if (event->key()==Qt::Key_Return) {
		tryAgainButtonClicked();
	}
}

//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
//void QtLoginMessage::updateAvatarLabel(){	
//		
//	UserProfile * userProfile = new UserProfile();
//
//	//gets userprofile
//	_cUserProfileHandler.getUserProfileHandler().LoadUserProfileFromName(userProfile, _profileName);
//	std::string backgroundPixmapFilename = ":/pics/avatar_background.png";
//
//	if(userProfile){		
//		std::string foregroundPixmapData = userProfile->getIcon().getData();
//		_ui->avatarLabel->setPixmap(PixmapMerging::mergeFromPixmap(foregroundPixmapData, backgroundPixmapFilename));//merge two images
//	}
//	else{
//		QPixmap backgroundPixmap;
//		backgroundPixmap.loadFromData((uchar *) backgroundPixmapFilename.c_str(), backgroundPixmapFilename.size());
//		_ui->avatarLabel->setPixmap(backgroundPixmap);
//	}
//		
//}

