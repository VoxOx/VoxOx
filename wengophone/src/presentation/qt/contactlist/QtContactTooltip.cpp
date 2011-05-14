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
* Custom ContactList tooltip 
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContactTooltip.h"
#include "QtContactListStyle.h"
#include "QtContactList.h"
#include "ui_ContactTooltip.h"
#include <qtutil/SafeConnect.h>

#include <model/contactlist/ContactProfile.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>
#include <presentation/qt/QtWengoPhone.h>
//VOXOX - CJC - 2009.06.16 
#include <presentation/qt/contactlist/QtContactActionManager.h>
#include <presentation/qt/contactlist/QtContactListManager.h>//VOXOX - CJC - 2009.06.12 
#include <QtGui/QtGui>
static const int TIMER_MILISECONDS = 500;

#include <util/SafeDelete.h>

QtContactTooltip::QtContactTooltip(QString contactId, QWidget * parent, CWengoPhone & cWengoPhone)
	:_contactId(contactId),
	QWidget(parent,Qt::ToolTip),
	_cWengoPhone(cWengoPhone){
	
	_ui = new Ui::ContactTooltip();
	_ui->setupUi(this);


	_timer = new QTimer(this);
	SAFE_CONNECT(_timer, SIGNAL(timeout()), SLOT(closeTooltip()));

	SAFE_CONNECT(_ui->lblChat, SIGNAL(linkActivated(const QString &)), SLOT(chatContact()));

	SAFE_CONNECT(_ui->lblCall, SIGNAL(linkActivated(const QString &)), SLOT(callContact()));

	SAFE_CONNECT(_ui->lblSMS, SIGNAL(linkActivated(const QString &)), SLOT(smsContact()));

	SAFE_CONNECT(_ui->lblSendFile, SIGNAL(linkActivated(const QString &)), SLOT(sendFileContact()));

	SAFE_CONNECT(_ui->lblSendFax, SIGNAL(linkActivated(const QString &)), SLOT(sendFaxContact()));

	SAFE_CONNECT(_ui->lblEmail, SIGNAL(linkActivated(const QString &)), SLOT(emailContact()));

	SAFE_CONNECT(_ui->lblProfile, SIGNAL(linkActivated(const QString &)), SLOT(profileContact()));

	//SAFE_CONNECT(_ui->lblSocial, SIGNAL(linkActivated(const QString &)), SLOT(socialContact()));//VOXOX CHANGE by Rolando - 2009.09.23 
	SAFE_CONNECT(_ui->lblStatusPixmap, SIGNAL(clicked()), SLOT(networkIconClicked()));//VOXOX CHANGE by Rolando - 2009.09.23

	//_networkIconMenu = NULL;//VOXOX CHANGE by Rolando - 2009.09.23

	_contactNetworkMenu = new QtContactNetworkMenu(_contactId.toStdString(),_cWengoPhone , NULL);//VOXOX CHANGE by Rolando - 2009.10.27 

	_imProtocol = QtEnumIMProtocol::IMProtocolUnknown;//VOXOX CHANGE by Rolando - 2009.09.23 
	
	_ui->lblLastLine->setVisible(false);//VOXOX CHANGE by Rolando - 2009.09.23 
	_ui->lblSocial->setVisible(false);//VOXOX CHANGE by Rolando - 2009.09.23

	_isASocialNetwork = false;//VOXOX CHANGE by Rolando - 2009.09.23

	_isMouseHover = false;//VOXOX CHANGE by Rolando - 2009.09.23

}

QtContactTooltip::~QtContactTooltip() {
	delete _ui;
	delete _timer;
	OWSAFE_DELETE(_contactNetworkMenu);//VOXOX CHANGE by Rolando - 2009.10.27 
	
}

void QtContactTooltip::chatContact() {

	closeThis();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->chatContact(_contactId);

	
}


void QtContactTooltip::socialContact() {

	if(_isASocialNetwork){//VOXOX CHANGE by Rolando - 2009.09.23 
		closeThis();

		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
		contactActionManager->openSocialContactPage(_contactId);
	}	
}

//void QtContactTooltip::buildMenuNetworkIcon()//VOXOX CHANGE by Rolando - 2009.09.24 
//{
//	QtEnumIMProtocol::IMProtocol protocol = _imProtocol;
//
//	_networkIconMenu = new QMenu(this);
//
//	QAction * chatAction	  = new QAction(tr("Chat"),   this);
//	QAction * socialWebAction = new QAction(tr("Social"), this);
//	
//	SAFE_CONNECT(chatAction,      SIGNAL(triggered()), SLOT(chatContact()));
//	SAFE_CONNECT(socialWebAction, SIGNAL(triggered()), SLOT(socialContact()));
//	
//	if(protocol == QtEnumIMProtocol::IMProtocolVoxOx)
//	{
//		QAction * callWithVideo =  new QAction(tr("Call with Video"), this);
//		SAFE_CONNECT(callWithVideo, SIGNAL(triggered()), SLOT(callContact()));
//
//		QAction * editContact =  new QAction(tr("Edit Contact"), this);
//		SAFE_CONNECT(editContact, SIGNAL(triggered()), SLOT(profileContact()));
//
//		_networkIconMenu->addAction(callWithVideo);
//		_networkIconMenu->addAction(editContact);
//
//		return;
//		
//	}
//	_networkIconMenu->addAction(chatAction);
//
//	if(protocol ==QtEnumIMProtocol::IMProtocolTwitter)
//	{
//		chatAction->setText(tr("Reply to"));
//		socialWebAction->setText(tr("Twitter Profile"));
//		_networkIconMenu->addAction(socialWebAction);
//	}
//	else if(protocol ==QtEnumIMProtocol::IMProtocolFacebook)
//	{
//		socialWebAction->setText(tr("Facebook Profile"));
//		_networkIconMenu->addAction(socialWebAction);
//	}
//	else if(protocol ==QtEnumIMProtocol::IMProtocolMYSPACE)
//	{
//		socialWebAction->setText(tr("MySpace Profile"));
//		_networkIconMenu->addAction(socialWebAction);
//	}
//}

void QtContactTooltip::networkIconClicked(){
	if(_contactNetworkMenu!=NULL){
		QPoint p = _ui->lblStatusPixmap->pos();
		p.setY(p.y() + _ui->lblStatusPixmap->height());
		_contactNetworkMenu->move(this->mapToGlobal(p));
		_contactNetworkMenu->show();
	}
}

void QtContactTooltip::callContact() {

	closeThis();
	
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->callContact(_contactId);


}

void QtContactTooltip::smsContact() {

	
	closeThis();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->smsContact(_contactId);

	
}

void QtContactTooltip::sendFileContact() {


	closeThis();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->sendFileContact(_contactId);


}

void QtContactTooltip::sendFaxContact() {//VOXOX - CJC - 2009.06.23 


	closeThis();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->sendFaxContact(_contactId);


}

void QtContactTooltip::emailContact() {

	closeThis();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->emailContact(_contactId);

}


void QtContactTooltip::profileContact() {

	closeThis();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->profileContact(_contactId);

	
	
	
}

void QtContactTooltip::setProtocol(QtEnumIMProtocol::IMProtocol protocol){
	_imProtocol = protocol;//VOXOX CHANGE by Rolando - 2009.09.23
	//buildMenuNetworkIcon();//VOXOX CHANGE by Rolando - 2009.10.27 

	if(protocol== QtEnumIMProtocol::IMProtocolFacebook || 
	   protocol== QtEnumIMProtocol::IMProtocolTwitter || 
	   protocol== QtEnumIMProtocol::IMProtocolMYSPACE)//VOXOX CHANGE by Rolando - 2009.09.23 
	{
		_isASocialNetwork = true;//VOXOX CHANGE by Rolando - 2009.09.23		
	}
	else{
		_isASocialNetwork = false;//VOXOX CHANGE by Rolando - 2009.09.23 		
	}

	//VOXOX CHANGE by Rolando - 2009.09.23 - commented lines
	/*_ui->lblLastLine->setVisible(true);
	_ui->lblSocial->setVisible(true);
	if(protocol== QtEnumIMProtocol::IMProtocolFacebook){
		_ui->lblSocial->setText("Facebook");
	}else if (protocol== QtEnumIMProtocol::IMProtocolTwitter)
	{
		_ui->lblSocial->setText("Twitter");
	}
	else if (protocol== QtEnumIMProtocol::IMProtocolMYSPACE)
	{
		_ui->lblSocial->setText("MySpace");
	}else{
		_ui->lblLastLine->setVisible(false);
		_ui->lblSocial->setVisible(false);
	}*/
}
//VOXOX - CJC - 2009.06.13 
void QtContactTooltip::closeThis(){

	if(_timer){
		if(_timer->isActive()){
			_timer->stop();
		}
	}
	if(isVisible()){
		close();
	}
	
}

void QtContactTooltip::closeTooltip(){
	//Mouse is on top of the tooltip
	QtWengoPhone * qtWengoPhone = (QtWengoPhone *)_cWengoPhone.getPresentation();

	
	if(_isMouseHover || qtWengoPhone->getQtContactList()->getWidget()->underMouse()){
		_timer->start(TIMER_MILISECONDS);
	}else{
		_timer->stop();
		closeMe();
	}
}

void QtContactTooltip::setDisplayName(QString displayName){
	_displayName = checkLink(displayName);
	_ui->lblDisplayName->setText(_displayName);
}
void QtContactTooltip::setStatusMessage(QString statusMessage){
	QString message = checkLink(statusMessage);
	if(message!=""){
	_statusMessage = "<img src=\":/pics/status/status_update.png\" />";
	_statusMessage +="<span style=\"color:#ffffff;\"> status: </span>";
	_statusMessage +=QString("<span style=\"color:#adacac;\">%1</span>").arg(message);
	}
	else{
		_statusMessage = message;
	}
	_ui->lblStatusMessage->setText(_statusMessage);
}

//VOXOX CHANGE by Rolando - 2009.10.23 
//void QtContactTooltip::setStatusPixmap(QPixmap statusPixmap){
//	_statusPixmap = statusPixmap;
//	_ui->lblStatusPixmap->setPixmap(_statusPixmap);
//}

//VOXOX CHANGE by Rolando - 2009.10.23 
void QtContactTooltip::setStatusPixmapPath(QString normalStatusPixmapPath, QString hoverStatusPixmapPath){
	_normalStatusPixmapPath = normalStatusPixmapPath;
	_hoverStatusPixmapPath = hoverStatusPixmapPath;
	_ui->lblStatusPixmap->setImages(_normalStatusPixmapPath, "", _hoverStatusPixmapPath);//VOXOX CHANGE by Rolando - 2009.10.23 

}
//VOXOX - CJC - 2009.06.10 
QString QtContactTooltip::checkLink(QString text){

	QString returnURL = text;
	QString textToReplace;

	//This expresion will only work if images come inside href tags (VoxOx, MSN only tested that does)
	QRegExp * regExp = new QRegExp("(http://|https://|ftp://|www.)[^ |\r|\n]+");
	
	if(regExp->indexIn(text) != -1){

		QString findUrl = regExp->cap();	

		returnURL = returnURL.replace(findUrl,QString("<a href=\"%1\"><span style=\" text-decoration: underline; color:#dce602;\">Link</span></a>").arg(findUrl));
	
	}

	return returnURL;

}

void QtContactTooltip::setAvatarPixmap(QPixmap avatarPixmap){
	
	int pixmapWidth = avatarPixmap.width();
	if(pixmapWidth > 150){
		_avatarPixmap = avatarPixmap.scaledToWidth(150,Qt::SmoothTransformation);
	}else{
		_avatarPixmap = avatarPixmap;
	}
	_ui->lblAvatarPixmap->setPixmap(_avatarPixmap);
	_ui->lblAvatarPixmap->setMinimumSize(_avatarPixmap.size());
	_ui->lblAvatarPixmap->setMaximumSize(_avatarPixmap.size());
	_ui->lblAvatarPixmap->resize(_avatarPixmap.size());
}

void QtContactTooltip::showTooltip() {
	this->setVisible(false);
	QtWengoPhone * qtWengoPhone = (QtWengoPhone *)_cWengoPhone.getPresentation();
	QPoint cursorPoint = QCursor::pos();

	
	int y = cursorPoint.y();
	int x = qtWengoPhone->getWidget()->x();

	/*int witdh;
	int height;
	int statusPixmapWidth = _statusPixmap.width();
	int statusPixmapHeight = _statusPixmap.height();
	int displayNameWithPixels =  QFontMetrics(_ui->lblStatusMessage->font()).width(_displayName);
	int statusMessageHeight = QFontMetrics(_ui->lblStatusMessage->font()).lineSpacing();
	int statusMessageWidth = QFontMetrics(_ui->lblStatusMessage->font()).width(_statusMessage);
	int calculatedHeight;*/
		
	//witdh =  _avatarPixmap.width() + displayNameWithPixels + statusPixmapWidth + _ui->gridLayout_5->horizontalSpacing()+11;
	/*_ui->lblStatusMessage->setMaximumWidth(displayNameWithPixels+ statusPixmapWidth );
	_ui->lblStatusMessage->setMinimumWidth(displayNameWithPixels+ statusPixmapWidth );*/
	//Calculate height
	//calculatedHeight = statusMessageWidth/displayNameWithPixels;
	//if(statusMessageWidth%displayNameWithPixels !=0) {
	//	calculatedHeight++;
	//}
	//calculatedHeight *= statusMessageHeight;
	//calculatedHeight += statusPixmapHeight;
	//
	//height = qMax(_avatarPixmap.height(),calculatedHeight);
	////height+=10;
	//

	//height += _ui->gridLayout_5->verticalSpacing()+11;
	////////Because UI spacing
	//height += 20;

	////
	//setMinimumSize(witdh,height);
	//setMaximumSize(witdh,height);
	//
	/*setMinimumHeight(height);
	setMaximumHeight(height);*/

	/*resize(witdh,height);*/
	y = y - (this->height()/2);
	//Tooltip wont fit on the left
	_timer->start(TIMER_MILISECONDS);
#if defined(OS_MACOSX)
	adjustSize();
	
	resize(size().width(), size().height()-300);
#endif
	int rightSize = QApplication::desktop()->screenGeometry(qtWengoPhone->getWidget()).width() - (x + qtWengoPhone->getWidget()->width());
	if(rightSize > this->width()){
		x = x + qtWengoPhone->getWidget()->frameSize().width();
		move(x,y);
		showNormal();	
	}else{
		x = x-this->width();
		move(x,y);
		showNormal();		
	}
}

//VOXOX CHANGE by Rolando - 2009.09.23 
void QtContactTooltip::enterEvent ( QEvent * event )
{
	_isMouseHover = true;
}

//VOXOX CHANGE by Rolando - 2009.09.23 
void QtContactTooltip::leaveEvent ( QEvent * event )
{
	QPoint pos = mapFromGlobal(cursor().pos());
	QRect rectTmp = rect();
	if(!rectTmp.contains(pos)){
		_isMouseHover = false;
	}
}

