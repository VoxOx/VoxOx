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
#include "stdafx.h"		//VOXOX - JRT - 2009.04.05 - for PCH
#include "QtUMSendFile.h"

#include "ui_UMSendFile.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>




#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <qtutil/SafeConnect.h>


#include <string>

#include <QtGui/QtGui>



QtUMSendFile::QtUMSendFile(CChatHandler & cChatHandler,QWidget * parent,QString key,QtEnumUMMode::Mode mode) :
	QtUMBase(parent,key,mode),
	_cChatHandler(cChatHandler){

	_ui = new Ui::UMSendFile();

	_widget =  new QWidget(parent);

	//setup ui
	_ui->setupUi(_widget);
	

	SAFE_CONNECT(_ui->webView, SIGNAL(urlChanged(const QUrl &)), SLOT(beforeNavigateSlot(const QUrl &)));
	
	//VOXOX CHANGE CJC MOVE LAYOUT DOWN A BIT
	#if defined(OS_MACOSX)
		//this->layout()->setContentsMargins(0, 11, 0, 0); 
	#endif
	
	_uuid = QUuid::createUuid().toString();

	//
	


	////

}

QtUMSendFile::~QtUMSendFile() {

	OWSAFE_DELETE(_widget);
	OWSAFE_DELETE(_ui);
	
}


void QtUMSendFile::setContactConnected(bool connected) {

	_isContactConnected = connected;
}



void QtUMSendFile::init(const QString & contactId,const QString & userName,const QString & md5,const QString & contactIdUploadFile,const QString & senderName) {

	setContactId(contactId);
	setUsername(userName);
	setMd5(md5);
	setContactIdUploadFile(contactIdUploadFile);
	setSenderName(senderName);

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString address = QString::fromStdString(config.getSendFileUrl());


	address = address+QString("?username=%1&userkey=%2&recipient=%3&uid=%4").arg(_userName).arg(_md5).arg(_contactIdUploadFile).arg(_uuid);

	_ui->webView->setUrl(QUrl(address));
	
}


void QtUMSendFile::beforeNavigateSlot(const QUrl & link){
	QString url = link.toString();
	if(url.contains("sendFileStep2.php")){
		LOG_DEBUG("File is uploading...");	
	}
	if(url.contains("sendFileStep4.php")){
		LOG_DEBUG("File is uploaded...");	
		//Get download link from tittle
		QString expectedTitle("Voxox | File Transfer | Instant Message | ");
		QString downloadLink = _ui->webView->title();
		if(downloadLink.contains(expectedTitle)){
			
			//Get link
			downloadLink.replace(expectedTitle, QString(""));
			//Setup message
			QString message  = 	QString("%1 has send you a file from VoxOx. You can download the file now at: %2").arg(_userName).arg(downloadLink);
			//Send message to contact
			_cChatHandler.getCUserProfile().startIMAndSendMessage(_contactId.toStdString(),message.toStdString());

		}
	}
}