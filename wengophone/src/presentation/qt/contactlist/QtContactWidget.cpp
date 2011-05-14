/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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
#include "QtContactWidget.h"

#include "ui_ContactWidget.h"

#include "QtContact.h"
#include "QtContactListManager.h"
//#include "QtContactManager.h"
#include "QtContactListStyle.h"

#include <presentation/qt/contactlist/QtContactListManager.h>
#include <presentation/qt/contactlist/QtContactActionManager.h>
#include <presentation/qt/profile/QtProfileDetails.h>


#include <presentation/qt/messagebox/QtVoxPhoneNumberMessageBox.h>
#include <presentation/qt/messagebox/QtVoxEmailMessageBox.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

#include <control/CWengoPhone.h>
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>

//#include <presentation/qt/QtEnumIMProtocol.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <presentation/qt/QtEnumPhoneType.h>
#include <presentation/qt/QtEnumEmailType.h>
#include <util/OWPicture.h>
#include <util/Logger.h>
#include <util/WebBrowser.h>
#include <util/SafeDelete.h>			//VOXOX - JRT - 2009.10.15 
#include <qtutil/PixmapMerging.h>
#include <qtutil/SafeConnect.h>

 

#include <QtGui/QtGui>

static const std::string AVATAR_BACKGROUND = ":/pics/avatar_background.png";

//QtContactWidget::QtContactWidget(const std::string & contactId, const std::string& qtContactKey, CWengoPhone & cWengoPhone,	QtContactManager * qtContactManager, QWidget * parent )
QtContactWidget::QtContactWidget(const std::string & contactId, const std::string& qtContactKey, CWengoPhone & cWengoPhone,	QWidget * parent )	//VOXOX - JRT - 2009.10.15 
	: QWidget(parent),
	_cWengoPhone(cWengoPhone) 
{
	_contactId	  = contactId;
	_qtContactKey = qtContactKey;

	_ui = new Ui::ContactWidget();
	_ui->setupUi(this);

	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
	QString tooltipStyle = QString("QToolTip{color: %1; background-color: %2; border-radius: 2px;}").arg(contactListStyle->getContactUserNameSelectedFontColor().name()).arg(contactListStyle->getContactNonSelectedBackgroundColor().name());
	_ui->lblUserName->setStyleSheet(QString("QLabel{color: "+contactListStyle->getContactUserNameSelectedFontColor().name()+";}")+tooltipStyle);
	_ui->lblNetworkStatus->setStyleSheet(QString("QLabel{color: "+contactListStyle->getContactSocialStatusSelectedFontColor().name()+";}")+tooltipStyle);
	//_ui->avatarButton->setStyleSheet(tooltipStyle);//VOXOX CHANGE by Rolando - 2009.10.20
	_ui->avatarLabel->setStyleSheet(tooltipStyle);//VOXOX CHANGE by Rolando - 2009.10.20 

	_ui->callButton->setImages	  (":/pics/contact/call_enabled.png",		"", ":/pics/contact/call_hover.png",	":/pics/contact/call_disabled.png"		);
	_ui->chatButton->setImages	  (":/pics/contact/chat_enabled.png",		"", ":/pics/contact/chat_hover.png",	":/pics/contact/chat_disabled.png"		);
	_ui->smsButton->setImages	  (":/pics/contact/sms_enabled.png",		"", ":/pics/contact/sms_hover.png",		":/pics/contact/sms_disabled.png"		);
	_ui->sendFileButton->setImages(":/pics/contact/sendfile_enabled.png",	"", ":/pics/contact/sendfile_hover.png",":/pics/contact/sendfile_disabled.png"	);
	_ui->emailButton->setImages	  (":/pics/contact/email_enabled.png",		"", ":/pics/contact/email_hover.png",	":/pics/contact/email_disabled.png"		);
	_ui->profileButton->setImages (":/pics/contact/profile_enabled.png",	"", ":/pics/contact/profile_hover.png",	":/pics/contact/profile_disabled.png"	);
	_ui->faxButton->setImages     (":/pics/contact/fax_enabled.png",	    "", ":/pics/contact/fax_hover.png",  	":/pics/contact/fax_disabled.png"	    );
	
	updateButtons();

//	updateToolTips();	//VOXOX - JRT - 2009.10.15 - Empty method.

	SAFE_CONNECT(_ui->callButton,		SIGNAL(clicked()),			SLOT(callButtonClicked()));
	SAFE_CONNECT(_ui->callButton,		SIGNAL(clickAndHold()),		SLOT(callButtonClickAndHolded()));	//VOXOX - CJC - 2009.06.11 
	SAFE_CONNECT(_ui->callButton,		SIGNAL(rightClick()),		SLOT(callButtonClickAndHolded()));//VOXOX - CJC - 2009.07.03 

	SAFE_CONNECT(_ui->chatButton,		SIGNAL(clicked()),			SLOT(chatButtonClicked()));

	SAFE_CONNECT(_ui->sendFileButton,	SIGNAL(clicked()),			SLOT(sendFileButtonClicked()));

	SAFE_CONNECT(_ui->emailButton,		SIGNAL(clicked()),			SLOT(emailButtonClicked()));
	SAFE_CONNECT(_ui->emailButton,		SIGNAL(clickAndHold()),		SLOT(emailButtonClickAndHolded()));	//VOXOX - CJC - 2009.06.11 
	SAFE_CONNECT(_ui->emailButton,		SIGNAL(rightClick()),		SLOT(emailButtonClickAndHolded()));	//VOXOX - CJC - 2009.07.03 

	SAFE_CONNECT(_ui->faxButton,		SIGNAL(clicked()),			SLOT(faxButtonClicked()));
	SAFE_CONNECT(_ui->faxButton,		SIGNAL(clickAndHold()),		SLOT(faxButtonClickAndHolded()));	//VOXOX - CJC - 2009.06.11 
	SAFE_CONNECT(_ui->faxButton,		SIGNAL(rightClick()),		SLOT(faxButtonClickAndHolded()));//VOXOX - CJC - 2009.07.03 

	SAFE_CONNECT(_ui->smsButton,		SIGNAL(clicked()),			SLOT(smsButtonClicked()));
	SAFE_CONNECT(_ui->smsButton,		SIGNAL(clickAndHold()),		SLOT(smsButtonClickAndHolded()));	//VOXOX - CJC - 2009.06.14 
	SAFE_CONNECT(_ui->smsButton,		SIGNAL(rightClick()),		SLOT(smsButtonClickAndHolded()));//VOXOX - CJC - 2009.07.03 

	SAFE_CONNECT(_ui->profileButton,	SIGNAL(clicked()), SLOT(profileButtonClicked()));

	//SAFE_CONNECT(_ui->avatarButton,		SIGNAL(clicked()), SLOT(avatarButtonClicked()));//VOXOX CHANGE by Rolando - 2009.10.20 - task completed "Update: clicking on contact avatar functionality"

	//VOXOX CHANGE ADD FUNCTIONALITY TO CLICK EVENT ON THE NETWORK LABEL TO OPEN THE CONTACS ONLINE PROFILE /FACEBOOK/TWITTER/MYSPACE 
	SAFE_CONNECT(_ui->lblNetworkIcon,	SIGNAL(leftMouseButtonPress()), SLOT(networkIconClicked()));
	
	_callMenu	= NULL;
	_emailMenu	= NULL;
	_smsMenu	= NULL;
	_faxMenu	= NULL;
	//_networkIconMenu = NULL;//VOXOX - CJC - 2009.07.08 //VOXOX CHANGE by Rolando - 2009.10.27  
	_contactNetworkMenu = new QtContactNetworkMenu(_contactId,_cWengoPhone , NULL);//VOXOX CHANGE by Rolando - 2009.10.27 

	//buildMenuNetworkIcon();//VOXOX CHANGE by Rolando - 2009.10.27  

//	SAFE_CONNECT_RECEIVER_TYPE(this, SIGNAL(editContact(QString)), qtContactManager, SLOT(editContact(QString)), Qt::QueuedConnection);
}

QtContactWidget::~QtContactWidget() 
{
	//VOXOX - JRT - 2009.10.15 
	OWSAFE_DELETE( _callMenu );
	OWSAFE_DELETE( _emailMenu );
	OWSAFE_DELETE( _smsMenu );
	OWSAFE_DELETE( _faxMenu );
	//OWSAFE_DELETE( _networkIconMenu );//VOXOX CHANGE by Rolando - 2009.10.27
	OWSAFE_DELETE( _contactNetworkMenu );//VOXOX CHANGE by Rolando - 2009.10.27 
	OWSAFE_DELETE( _ui );

	//	if(_callMenu){
//		delete _callMenu;
//	}
//	if(_emailMenu){
//		delete _emailMenu;
//	}
//	if(_smsMenu){
//		delete _smsMenu;
//	}
//	if(_faxMenu){
//		delete _faxMenu;
//	}
//	if(_networkIconMenu){
//		delete _networkIconMenu;
//	}

//	delete _ui;
}

void QtContactWidget::resizeEvent(QResizeEvent* event) 
{
	QWidget::resizeEvent(event);

	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
	QPalette pal = palette();
	
	//QRect r = rect();
	//QLinearGradient lg(QPointF(1, r.top()), QPointF(1, r.bottom()));
	//
	//lg.setColorAt(0, contactListStyle->getContactSelectedBackgroundColorTop());
	//lg.setColorAt(1, color.dark(150));

	//TODO ADD Gradient if needed

	pal.setBrush(backgroundRole(), QBrush(contactListStyle->getContactSelectedBackgroundColorTop()));
	setPalette(pal);
}

//VOXOX CHANGE CJC UPDATE CONTACT LIST WIDGET
void QtContactWidget::updateButtons() 
{
	bool							 hasAvailableSipNumber	= false;
	bool							 blocked				= false;
	EnumPresenceState::PresenceState eState					= EnumPresenceState::PresenceStateUnknown;
	QtEnumIMProtocol::IMProtocol	 qtImProtocol			= QtEnumIMProtocol::IMProtocolUnknown; 

	std::string						 foregroundPixmapData	= "";
	bool							 supportsIM				= false;

	Contact* contact = getContact();

	if ( contact )
	{
		hasAvailableSipNumber	= contact->hasPstnCall();
		blocked					= contact->isBlocked();
		eState					= contact->getPresenceState();
		qtImProtocol			= contact->getQtIMProtocol(); 
	
		foregroundPixmapData	= contact->getIcon().getData();
		supportsIM				= contact->hasIM();
	}

	//Set Avatar
	//_ui->avatarButton->setIcon(PixmapMerging::merge(foregroundPixmapData, AVATAR_BACKGROUND));//VOXOX CHANGE by Rolando - 2009.10.20 
	_ui->avatarLabel->setPixmap(PixmapMerging::merge(foregroundPixmapData, AVATAR_BACKGROUND));//VOXOX CHANGE by Rolando - 2009.10.20 

	//Set Status Pixmap
	QtContactPixmap::ContactPixmap status = QtContactPixmap::determinePixmap( qtImProtocol, eState, hasAvailableSipNumber,blocked, _ui->lblNetworkIcon->underMouse());

	QtContactPixmap * spx = QtContactPixmap::getInstance();
	QString normalNetworkPixmapPath = spx->getPixmapPath(status);//VOXOX CHANGE by Rolando - 2009.10.23 

	//VOXOX CHANGE by Rolando - 2009.10.22 - updates hover image
	QtContactPixmap::ContactPixmap hoverStatus = QtContactPixmap::determineHoverPixmap( qtImProtocol, eState, hasAvailableSipNumber,blocked);
	QString hoverNetworkPixmapPath = spx->getPixmapPath(hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.22 
	
	updateNetworkPixmaps(normalNetworkPixmapPath, hoverNetworkPixmapPath);//VOXOX CHANGE by Rolando - 2009.10.22 

	//VOXOX CHANGE by Rolando - 2009.10.23 _ui->lblNetworkIcon->setIcon(QIcon(px));


	//Set contact text
	updateText();

	
	//TODO GET STATUS MESSAGE

	//VOXOX - CJC - 2009.06.10 
	_ui->callButton->setEnabled(true);

	_ui->chatButton->setEnabled    ( supportsIM );
	_ui->sendFileButton->setEnabled(true);
	_ui->smsButton->setEnabled     (true);
	_ui->emailButton->setEnabled   (true);	//VOXOX - CJC - 2009.05.07 TODO Verify that user has email to enable this button
	_ui->profileButton->setEnabled (true);

	//_ui->sendFileButton->setEnabled(contactProfile.hasFileTransfer());
}


QString QtContactWidget::verifyText(QRect painterRect, QFont font, QString text)
{
	int xText = painterRect.left();
	int textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text	
	
	bool fixedText = false;

	while(xText + textWidth > this->geometry().width()-_ui->lblNetworkIcon->width()-5)//we need to check that we can paint on the QtContactWidget but not on the vertical line
	{
		text = text.remove(text.length()- 1,text.length());
		textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text
		fixedText = true;
	}
	
	if(fixedText)
	{
		if(text.length() >= 3)
		{			
			text = text.remove(text.length()- 3,text.length() );
			text += "...";
		}		
	}
	return text;
}

//VOXOX CHANGE by Rolando - 2009.10.22 
void  QtContactWidget::updateNetworkPixmaps(QString normalPixmapPath, QString hoverPixmapPath){
	_ui->lblNetworkIcon->setImages(normalPixmapPath,"",hoverPixmapPath);//VOXOX CHANGE by Rolando - 2009.10.23 
}

void QtContactWidget::updateText()
{
	std::string displayName = "unknown";
	std::string statusMsg   = "";
	Contact* contact = getContact();

	if ( contact )
	{
		displayName = contact->getDisplayName();
		statusMsg   = contact->getStatusMessage();
	}

	QString text = verifyText(_ui->lblUserName->geometry(), _ui->lblUserName->font(), QString::fromUtf8( displayName.c_str()));
	_ui->lblUserName->setText(text);
	
	QString statusMessage = verifyText(_ui->lblNetworkStatus->geometry(), _ui->lblNetworkStatus->font(), QString::fromUtf8( statusMsg.c_str() ));
	_ui->lblNetworkStatus->setText(statusMessage);
}

//VOXOX CHANGE by Rolando - 2009.10.27 - this code is now in QtContactNetworkMenu class
//void QtContactWidget::buildMenuNetworkIcon()//VOXOX - CJC - 2009.07.08 
//{
//	QtEnumIMProtocol::IMProtocol protocol = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactQtProtocol(_contactId);
//
//	_networkIconMenu = new QMenu(this);
//
//	QAction * chatAction	  = new QAction(tr("Chat"),   this);
//	QAction * socialWebAction = new QAction(tr("Social"), this);
//	
//	SAFE_CONNECT(chatAction,      SIGNAL(triggered()), SLOT(chatButtonClicked()));
//	SAFE_CONNECT(socialWebAction, SIGNAL(triggered()), SLOT(webProfileContactClicked()));
//	
//	if(protocol == QtEnumIMProtocol::IMProtocolVoxOx)
//	{
//		QAction * callWithVideo =  new QAction(tr("Call with Video"), this);
//		SAFE_CONNECT(callWithVideo, SIGNAL(triggered()), SLOT(callButtonClicked()));
//
//		QAction * editContact =  new QAction(tr("Edit Contact"), this);
//		SAFE_CONNECT(editContact, SIGNAL(triggered()), SLOT(profileContactClicked()));
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



void QtContactWidget::callButtonClicked() 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->callContact(QString::fromStdString(_contactId));
	
}

void QtContactWidget::profileContactClicked() //VOXOX - CJC - 2009.07.08 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->profileContact(QString::fromStdString(_contactId));
	
}


void QtContactWidget::webProfileContactClicked() 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->openSocialContactPage(QString::fromStdString(_contactId));
	
}
void QtContactWidget::faxButtonClicked() //VOXOX - CJC - 2009.06.23 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->sendFaxContact(QString::fromStdString(_contactId));
	
}

//VOXOX - CJC - 2009.06.14 
QString QtContactWidget::addNumber() {


	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addNumberToContact(QString::fromStdString(_contactId));
		
}

QString QtContactWidget::addFaxNumber() //VOXOX - CJC - 2009.06.23 
{
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addFaxNumberToContact(QString::fromStdString(_contactId));
		
}

//VOXOX - CJC - 2009.06.11 
void QtContactWidget::callButtonClickAndHolded() 
{
	Contact* contact = getContact();
	
	_callMenu = new QMenu(this);

	SAFE_CONNECT(_callMenu, SIGNAL(triggered(QAction *)), SLOT(callNumberFromMenu(QAction *)));

	Telephones& rPhones = contact->getTelephones();

	if(rPhones.hasValid()){
	
		for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
		{
			if ( (*it).isValid() )
			{
				QString number = QString::fromStdString((*it).getNumber());
				QtEnumPhoneType::Type type = QtEnumPhoneType::toPhoneType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumPhoneType::getTypeIconPath(type));
				//VOXOX - CJC - 2009.06.24 Show no icon
				_callMenu->addAction(number);
			}
		}
		_callMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction("Add Number +", this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addNumber()));

	_callMenu->addAction(addNumber);
	
	QPoint p = _ui->callButton->pos();
	p.setY(p.y() + _ui->callButton->height());
	_callMenu->move(this->mapToGlobal(p));
	_callMenu->show();
	
}

void QtContactWidget::faxButtonClickAndHolded() //VOXOX - CJC - 2009.06.23 
{
	Contact* contact = getContact();
	
	_faxMenu = new QMenu(this);

	SAFE_CONNECT(_faxMenu, SIGNAL(triggered(QAction *)), SLOT(faxNumberFromMenu(QAction *)));

	Telephones& rPhones = contact->getTelephones();

	if(rPhones.hasValid()){
	
		for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
		{
				if ( (*it).isValid() && (*it).getType() == QtEnumPhoneType::toString(QtEnumPhoneType::Fax).toStdString())
				{

					QString number = QString::fromStdString((*it).getNumber());
					QtEnumPhoneType::Type type = QtEnumPhoneType::toPhoneType(QString::fromStdString((*it).getType()));
					//QIcon icon(QtEnumPhoneType::getTypeIconPath(type));
					_faxMenu->addAction(number);

				}
			
		}
		_faxMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction("Add Fax Number +", this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addFaxNumber()));

	_faxMenu->addAction(addNumber);
	
	QPoint p = _ui->callButton->pos();
	p.setY(p.y() + _ui->callButton->height());
	_faxMenu->move(this->mapToGlobal(p));
	_faxMenu->show();
	
}

//VOXOX - CJC - 2009.06.11 
void QtContactWidget::callNumberFromMenu(QAction * action) {

	QString number = action->text();
	if(number!="Add Number +"){
		//VOXOX - CJC - 2009.06.11 
		getCUserProfile()->makeCall(number.toStdString());
	}
}


void QtContactWidget::faxNumberFromMenu(QAction * action) {//VOXOX - CJC - 2009.06.23 

	QString number = action->text();
	if(number!="Add Fax Number +"){
		//VOXOX - CJC - 2009.06.11 
		getCUserProfile()->sendFax(number.toStdString());
	}
}


void QtContactWidget::smsButtonClicked() 
{
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->smsContact(QString::fromStdString(_contactId));
}

//VOXOX - CJC - 2009.06.14 
QString QtContactWidget::addMobileNumber() {

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addMobileNumberToContact(QString::fromStdString(_contactId));
		
}

//VOXOX - CJC - 2009.06.11 
void QtContactWidget::smsButtonClickAndHolded() 
{
	Contact* contact = getContact();
	
	_smsMenu = new QMenu(this);

	SAFE_CONNECT(_smsMenu, SIGNAL(triggered(QAction *)), SLOT(sendSMSFromMenu(QAction *)));

	Telephones& rPhones = contact->getTelephones();

	if(rPhones.hasValid()){
	
		for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
		{
			if ( (*it).isValid() && (*it).getType() == QtEnumPhoneType::toString(QtEnumPhoneType::Mobile).toStdString())
			{
				QString number = QString::fromStdString((*it).getNumber());
				QtEnumPhoneType::Type type = QtEnumPhoneType::toPhoneType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumPhoneType::getTypeIconPath(type));//VOXOX - CJC - 2009.06.24 
				_smsMenu->addAction(number);
			}
		}
		_smsMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction(tr("Add Mobile Number +"), this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addMobileNumber()));

	_smsMenu->addAction(addNumber);
	
	QPoint p = _ui->callButton->pos();
	p.setY(p.y() + _ui->callButton->height());
	_smsMenu->move(this->mapToGlobal(p));
	_smsMenu->show();
	
}

//VOXOX - CJC - 2009.06.11 
void QtContactWidget::sendSMSFromMenu(QAction * action) {


	if(getCUserProfile()->getUserProfile().getVoxOxAccount()->isConnected()){
			
			QString sms = action->text();
			if(sms!=tr("Add Mobile Number +")){
				//Send email to contact//Chat window will get the prefered email and use that one
				getCUserProfile()->startIMToSMS(sms.toStdString());
			}

	}else{
			QtVoxMessageBox box(this);
			box.setWindowTitle(tr("Your VoxOx IM account is offline"));
			box.setText(tr("Your VoxOx IM account is offline. To send text messages, please reconnect and try again."));
			box.setStandardButtons(QMessageBox::Ok);
			box.exec();
	}

	
}


void QtContactWidget::chatButtonClicked() 
{
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->chatContact(QString::fromStdString(_contactId));
}

void QtContactWidget::emailButtonClicked() 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->emailContact(QString::fromStdString(_contactId));

}

//VOXOX - CJC - 2009.06.14 
QString QtContactWidget::addEmail() 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addEmailToContact(QString::fromStdString(_contactId));
}

//VOXOX - CJC - 2009.06.11 
void QtContactWidget::emailButtonClickAndHolded() 
{
	Contact* contact = getContact();
	
	_emailMenu = new QMenu(this);

	SAFE_CONNECT(_emailMenu, SIGNAL(triggered(QAction *)), SLOT(sendEmailFromMenu(QAction *)));

	EmailAddresses& rEmail = contact->getEmailAddresses();

	if(rEmail.hasValid()){
		
		for ( EmailAddresses::iterator it = rEmail.begin(); it != rEmail.end(); it++ )
		{
			if ( (*it).isValid() )
			{
				QString email = QString::fromStdString((*it).getAddress());
				QtEnumEmailType::Type type = QtEnumEmailType::toEmailType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumEmailType::getTypeIconPath(type));
				//VOXOX - CJC - 2009.06.24 Show no icon
				_emailMenu->addAction(email);
			}
		}
		_emailMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction("Add Email Address +", this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addEmail()));

	_emailMenu->addAction(addNumber);
	
	QPoint p = _ui->emailButton->pos();
	p.setY(p.y() + _ui->emailButton->height());
	_emailMenu->move(this->mapToGlobal(p));
	_emailMenu->show();
	
}

//VOXOX - CJC - 2009.06.11 
void QtContactWidget::sendEmailFromMenu(QAction * action) {

	if(getCUserProfile()->getUserProfile().getVoxOxAccount()->isConnected()){
			
			QString email = action->text();
			if(email!="Add Email Address +"){
				//Send email to contact//Chat window will get the prefered email and use that one
				getCUserProfile()->startIMToEmail(email.toStdString());
			}

	}else{
			QtVoxMessageBox box(this);
			box.setWindowTitle(tr("Your VoxOx IM account is offline"));
			box.setText(tr("Your VoxOx IM account is offline. To send emails, please reconnect and try again."));
			box.setStandardButtons(QMessageBox::Ok);
			box.exec();
	}
		
}


void QtContactWidget::profileButtonClicked() 
{
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->profileContact(QString::fromStdString(_contactId));
}

void QtContactWidget::sendFileButtonClicked() 
{
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->sendFileContact(QString::fromStdString(_contactId));
}

//void QtContactWidget::updateToolTips() 
//{
//}

//VOXOX CHANGE CJC - Open contact profile for Facebook and MySpace
//TODO: Move this functionality to a better location, WebPage.cpp EJ?
void QtContactWidget::avatarButtonClicked() 
{
	profileButtonClicked();
}

void QtContactWidget::networkIconClicked() 
{
	//VOXOX CHANGE by Rolando - 2009.10.27 
	if(_contactNetworkMenu!=NULL){
		QPoint p = _ui->lblNetworkIcon->pos();
		p.setY(p.y() + _ui->lblNetworkIcon->height());
		int widthMenu = _contactNetworkMenu->width();//VOXOX CHANGE by Rolando - 2009.10.20
		int heightMenu = _contactNetworkMenu->height();//VOXOX CHANGE by Rolando - 2009.10.27 

		if(widthMenu > 140){//VOXOX CHANGE by Rolando - 2009.10.27 - TODO: FIX this awful hack because sometimes QT returns a value that not corresponds to the real size of menu
			widthMenu = 140;//VOXOX CHANGE by Rolando - 2009.10.27 - TODO: FIX this awful hack because sometimes QT returns a value that not corresponds to the real size of menu
		}

		if(heightMenu > 84){//VOXOX CHANGE by Rolando - 2009.10.27 - TODO: FIX this awful hack because sometimes QT returns a value that not corresponds to the real size of menu
			heightMenu = _contactNetworkMenu->actions().count() * 21;//VOXOX CHANGE by Rolando - 2009.10.27 - TODO: FIX this awful hack because sometimes QT returns a value that not corresponds to the real size of menu
		}
		 
		QPoint point = getPositionToShowMenu(this->mapToGlobal(p), widthMenu, heightMenu);//VOXOX CHANGE by Rolando - 2009.10.20 
		_contactNetworkMenu->move(point);//VOXOX CHANGE by Rolando - 2009.10.20 
		_contactNetworkMenu->show();
	}	

}

//VOXOX CHANGE by Rolando - 2009.10.20 
QPoint QtContactWidget::getPositionToShowMenu(QPoint currentPoint, int widthMenu, int heightMenu)
{
	QRect desktopRect = QApplication::desktop()->screenGeometry();
	QPoint pointToReturn = currentPoint;//VOXOX CHANGE by Rolando - 2009.10.20 

	if(desktopRect.right() <= currentPoint.x() + widthMenu){//VOXOX CHANGE by Rolando - 2009.10.20  - checks position "x"
		currentPoint.setX(desktopRect.right() - widthMenu);//VOXOX CHANGE by Rolando - 2009.10.27 
		pointToReturn = currentPoint;
	}

	if(desktopRect.bottom() <= currentPoint.y() + heightMenu){//VOXOX CHANGE by Rolando - 2009.10.20 - checks position "y"
		currentPoint.setY(currentPoint.y() - heightMenu);
		pointToReturn = currentPoint;
	}

	return pointToReturn;
}

void QtContactWidget::paintEvent(QPaintEvent *) 
{
	QPainter painter(this);
	paintContact(&painter, rect());
	painter.end();
}

void QtContactWidget::paintContact(QPainter * painter, const QRect & rect) 
{
	QtContactListManager * ul = QtContactListManager::getInstance();
	QtContact * qtContact = ul->getContact(QString::fromStdString(_qtContactKey));

	if ( qtContact) 
	{
		QStyleOptionViewItem option;
		option.initFrom(ul->getTreeWidget());
		option.state = QStyle::State_Selected;

		//VOXOX CHANGE CJC WE DONT NEED TO WIDGET REPAINTED WHEN IT GETS SELECTED
		//qtContact->paintForeground(painter, option);
		//updateToolTips();
	}
}

CUserProfile* QtContactWidget::getCUserProfile()
{
	return _cWengoPhone.getCUserProfileHandler().getCUserProfile();
}

Contact* QtContactWidget::getContact()
{
	Contact* contact = getCUserProfile()->getCContactList().getContactByKey( _contactId );
	return contact;
}
