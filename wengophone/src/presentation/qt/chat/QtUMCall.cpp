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
#include "QtUMCall.h"

#include "ui_UMFax.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <presentation/qt/phonecall/QtContactCallListWidget.h>


#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <qtutil/SafeConnect.h>


#include <string>

#include <QtGui/QtGui>



QtUMCall::QtUMCall(CChatHandler & cChatHandler,QWidget * parent,QString key,QtEnumUMMode::Mode mode) :
	QtUMBase(parent,key,mode),
	_cChatHandler(cChatHandler){

}

QtUMCall::~QtUMCall() {

	_qtContactCallListWidget->deleteLater();
	
}


void QtUMCall::setContactConnected(bool connected) {

	_isContactConnected = connected;
}


void QtUMCall::hangUp() {

	_qtContactCallListWidget->hangup();
}




void QtUMCall::init(const QString & phoneNumber,const QString & contactId,const QString & senderName,QtContactCallListWidget * qtContactCallListWidget){
	
	setPhoneNumber(phoneNumber);
	setContactId(contactId);
	setSenderName(senderName);
	_qtContactCallListWidget = qtContactCallListWidget;
}

QWidget* QtUMCall::getWidget(){

	return _qtContactCallListWidget->getWidget();

}
