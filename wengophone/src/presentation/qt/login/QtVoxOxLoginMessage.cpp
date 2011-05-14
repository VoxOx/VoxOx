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
#include "QtVoxOxLoginMessage.h"

#include "ui_LoginMessage.h"

#include <presentation/qt/QtWengoPhone.h>

#include <control/profile/CUserProfileHandler.h>

#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/profile/UserProfileHandler.h>


#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>

#include <qtutil/PixmapMerging.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

//VOXOX CHANGE by Rolando 2009.05.05
QtVoxOxLoginMessage::QtVoxOxLoginMessage(QWidget* parent, QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler)
	: QWidget(NULL),
	_cUserProfileHandler(cUserProfileHandler),
	_qtWengoPhone(qtWengoPhone)
	{

	_qtWengoPhone = qtWengoPhone;

	_ui = new Ui::LoginMessage();
	_ui->setupUi(this);

	_profileName = "";

	_ui->tryAgainButton->setDefault(true);
	
	LANGUAGE_CHANGE(this);
		
	SAFE_CONNECT(_ui->tryAgainButton, SIGNAL(clicked()), SLOT(tryAgainButtonClicked()));

}

QtVoxOxLoginMessage::~QtVoxOxLoginMessage() {
	OWSAFE_DELETE(_ui);
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLoginMessage::tryAgainButtonClicked() {
	tryAgainButton(_profileName);
	deleteLater();	
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLoginMessage::setErrorMessage(const QString & message) {
	_ui->loginMessageError->setText(message);
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLoginMessage::setAdviceMessage(const QString & message) {
	_ui->loginMessageAdvice->setText(message);
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLoginMessage::setProfileName(const QString & profileName) {
	_profileName = profileName.toStdString();
	//updateAvatarLabel();//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
}

void QtVoxOxLoginMessage::languageChanged() {
	_ui->retranslateUi(this);
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLoginMessage::keyPressEvent(QKeyEvent *event) {
	if (event->key()==Qt::Key_Return) {
		tryAgainButtonClicked();
	}
}

//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
//VOXOX CHANGE by Rolando 2009.05.05
//void QtVoxOxLoginMessage::updateAvatarLabel(){	
//		
//	UserProfile * userProfile = new UserProfile();
//
//	//gets userprofile
//	_cUserProfileHandler.getUserProfileHandler().LoadUserProfileFromName(userProfile, _profileName);
//	std::string backgroundPixmapFilename = ":/pics/avatar_background.png";
//
//	if(userProfile){		
//		std::string foregroundPixmapData = userProfile->getIcon().getData();
//		_ui->avatarLabel->setPixmap(PixmapMerging::merge(foregroundPixmapData, backgroundPixmapFilename));//merge two images
//	}
//	else{
//		QPixmap backgroundPixmap;
//		backgroundPixmap.loadFromData((uchar *) backgroundPixmapFilename.c_str(), backgroundPixmapFilename.size());
//		_ui->avatarLabel->setPixmap(backgroundPixmap);
//	}
//		
//}

