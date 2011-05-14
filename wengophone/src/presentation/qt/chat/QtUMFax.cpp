/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2009.08.29
*/

#include "stdafx.h"		//VOXOX - JRT - 2009.04.05 - for PCH
#include "QtUMFax.h"

#include "ui_UMFax.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>




#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <qtutil/SafeConnect.h>


#include <string>

#include <QtGui/QtGui>



QtUMFax::QtUMFax(CChatHandler & cChatHandler,QWidget * parent,QString key,QtEnumUMMode::Mode mode) :
	QtUMBase(parent,key,mode),
	_cChatHandler(cChatHandler){

	_ui = new Ui::UMFax();

	_widget =  new QWidget(parent);

	//setup ui
	_ui->setupUi(_widget);

	
	//VOXOX CHANGE CJC MOVE LAYOUT DOWN A BIT
	#if defined(OS_MACOSX)
		//this->layout()->setContentsMargins(0, 11, 0, 0); 
	#endif

}

QtUMFax::~QtUMFax() {

	OWSAFE_DELETE(_widget);
	OWSAFE_DELETE(_ui);
	
}


void QtUMFax::setContactConnected(bool connected) {

	_isContactConnected = connected;
}



void QtUMFax::init(const QString & faxNumber,const QString & userName,const QString & md5,const QString & contactId,const QString & senderName) {


	setPhoneNumber(faxNumber);
	setContactId(contactId);
	setUsername(userName);
	setRecipient(senderName);
	setUserKey(md5);


	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString address = QString::fromStdString(config.getSendFaxUrl());
	if(_contactId!=_phoneNumber){
		address = address+QString("?username=%1&userkey=%2&recipient=%3&phonenumber=%4").arg(_userName).arg(_userKey).arg(_recepient).arg(_phoneNumber);
	}else{
		address = address+QString("?username=%1&userkey=%2&phonenumber=%3").arg(_userName).arg(_userKey).arg(_phoneNumber);
	}

	_ui->webView->setUrl(QUrl(address));
}
