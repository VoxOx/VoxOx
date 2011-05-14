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

#include "QtVoxOxLoadingLogin.h"

#include "ui_VoxOxLoadingLogin.h"

#include <presentation/qt/QtWengoPhone.h>

#include <control/profile/CUserProfileHandler.h>

#include <model/profile/UserProfile.h>

#include <model/profile/UserProfileHandler.h>


#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>

#include <qtutil/PixmapMerging.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>



//VOXOX CHANGE by Rolando 2009.05.05
QtVoxOxLoadingLogin::QtVoxOxLoadingLogin(QWidget* parent, QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler)
	: QWidget(NULL),
	_cUserProfileHandler(cUserProfileHandler),
	_qtWengoPhone(qtWengoPhone)
	{

	_ui = new Ui::VoxOxLoadingLogin();
	_ui->setupUi(this);

	//setStyleSheet(QString("QLabel#loadingMessage{ color: #ffffff; font-size: 13px; font-weight: bold; font-family: \"Tahoma\"; } "));

	_profileName = "";
		
	LANGUAGE_CHANGE(this);

	//VOXOX CHANGE by Rolando - 2009.05.21 - we use QMovie to be able to show a "gif" animation
	QMovie * movie =  new QMovie(":/pics/loading.gif");
	_ui->loadingMessage->setMovie(movie);
	movie->start();

}

QtVoxOxLoadingLogin::~QtVoxOxLoadingLogin() {
	OWSAFE_DELETE(_ui);
	/*if(_timer){
		_timer->stop();
		OWSAFE_DELETE(_timer);
	}*/
}
//
////TODO: VOXOX CHANGE by Rolando 05-04-09, change this kind of animation with a .mng file to show an animation when it is loading...
//void QtVoxOxLoadingLogin::updateLoadingMessage(){
//	++_seconds;
//
//	int counter = _seconds % 4;
//	QString suffix  = "";
//
//	switch(counter){
//		case 0:
//			suffix = "";
//		break;
//		case 1:
//			suffix = ".";
//		break;
//		case 2:
//			suffix = "..";
//		break;
//		case 3:
//			suffix = "...";
//		break;
//		default:;
//	}
//	_ui->loadingMessage->setText(_loadingMessage + suffix);
//
//	if(_seconds == 3600){//to avoid overflow we restart it every hour...
//		_seconds = 0;
//	}
//
//}
//
////VOXOX CHANGE by Rolando 2009.05.05
//void QtVoxOxLoadingLogin::setLoadingMessage(const QString & message) {
//	_loadingMessage = message;
//	_ui->loadingMessage->setText(_loadingMessage);
//}

//VOXOX CHANGE by Rolando 2009.05.05
void QtVoxOxLoadingLogin::setProfileName(const QString & profileName) {
	_profileName = profileName.toStdString();
	//updateAvatarLabel();//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
}

void QtVoxOxLoadingLogin::languageChanged() {
	_ui->retranslateUi(this);
}

//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
//VOXOX CHANGE by Rolando 2009.05.05
//void QtVoxOxLoadingLogin::updateAvatarLabel(){	
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
//}