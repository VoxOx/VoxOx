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
* CLASS QtPopUpChatDialog
* @author Rolando 
* @date 2009.06.10
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtPopUpChatDialog.h"
#include "QtUMWindow.h"
//#include "QtChatWidget.h"
//#include "QtCallWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
//#include "QtChatConferenceCallWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
#include "QtChatToEmailSMSUtils.h"
//#include "QtChatToEmailWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
//#include "QtChatToSMSWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
#include "QtChatToEmailMessageData.h"
//#include "QtChatToSMSMessageData.h"//VOXOX CHANGE by Rolando - 2009.10.28 
//#include "QtSendFileWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
//#include "QtSendFaxWidget.h"//VOXOX - CJC - 2009.06.23 //VOXOX CHANGE by Rolando - 2009.10.28 

//#include "QtUniversalMessageTabWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 

#include "ui_PopUpChatDialog.h"

#include <control/CWengoPhone.h>//VOXOX CHANGE by Rolando - 2009.06.16 
#include <control/profile/CUserProfileHandler.h>//VOXOX CHANGE by Rolando - 2009.06.16 
#include <control/profile/CUserProfile.h>//VOXOX CHANGE by Rolando - 2009.06.16 

#include <cutil/global.h>

#include <qtutil/Object.h>
#include <qtutil/SafeConnect.h>
//VOXOX - CJC - 2009.05.05 See if windows gets closed so it destroys the ui
#include <qtutil/CloseEventFilter.h>
#include <qtutil/WidgetUtils.h>
#include <qtutil/Widget.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <presentation/qt/contactlist/QtContactActionManager.h>
#if defined(OS_WINDOWS)
	#include <windows.h>
#endif

//VOXOX CHANGE by Alexander 02-20-09: added growl to display the text that the user just sent.
#if defined(OS_MACOSX)
	#include "qtutil/GrowlNotifier.h"
#endif

//VOXOX CHANGE by Rolando - 2009.06.16 
QtPopUpChatDialog::QtPopUpChatDialog(QWidget * parent, QtUMBase * QtUMBase,CWengoPhone & cWengoPhone)
	: QDialog(parent),
	_QtUMBase(QtUMBase),
    _cWengoPhone(cWengoPhone){//VOXOX CHANGE by Rolando - 2009.06.16 
//	
//	_ui = new Ui::PopUpChatDialog();
//	_ui->setupUi(this);
//
//#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.10 
//	_ui->leftButtonFrame->layout()->setSpacing(0);//VOXOX CHANGE by Rolando - 2009.07.10 
//	_ui->leftButtonFrame->layout()->setMargin(0);//VOXOX CHANGE by Rolando - 2009.07.10
//	_ui->smsButton->setMinimumSize(58,38);//VOXOX CHANGE by Rolando - 2009.07.10 
//	_ui->smsButton->setMaximumSize(58,38);//VOXOX CHANGE by Rolando - 2009.07.10 
//	_ui->sendFileButton->setMinimumSize(78,38);//VOXOX CHANGE by Rolando - 2009.07.10 
//	_ui->sendFileButton->setMaximumSize(78,38);//VOXOX CHANGE by Rolando - 2009.07.10 
//#endif
//
//	_ui->chatButton->setImages(":/pics/chat/btnChatButton.png",":/pics/chat/btnChatButtonPress.png","","",":/pics/chat/btnChatButtonCheck.png");
//	_ui->callButton->setImages(":/pics/chat/btnCallButton.png",":/pics/chat/btnCallButtonPress.png","","",":/pics/chat/btnCallButtonCheck.png");
//	_ui->smsButton->setImages(":/pics/chat/btnSMSButton.png",":/pics/chat/btnSMSButtonPress.png","","",":/pics/chat/btnSMSButtonCheck.png");
//	_ui->sendFileButton->setImages(":/pics/chat/btnSendFileButton.png",":/pics/chat/btnSendFileButtonPress.png","","",":/pics/chat/btnSendFileButtonCheck.png");
//	_ui->emailButton->setImages(":/pics/chat/btnEmailButton.png",":/pics/chat/btnEmailButtonPress.png","","",":/pics/chat/btnEmailButtonCheck.png");
//	_ui->faxButton->setImages(":/pics/chat/btnFaxButton.png",":/pics/chat/btnFaxButtonPress.png","","",":/pics/chat/btnFaxButtonCheck.png");
//	_ui->phoneSettingsButton->setImages(":/pics/chat/btnPhoneSettings.png",":/pics/chat/btnPhoneSettingsPress.png","","",":/pics/chat/btnPhoneSettingsCheck.png");
//	_ui->profileButton->setImages(":/pics/chat/btnProfileSettings.png",":/pics/chat/btnProfileSettingsPress.png","","",":/pics/chat/btnProfileSettingsCheck.png");
//	_ui->buttonFrame->setStyleSheet(QString("QFrame{background-image: url(:/pics/chat/btnBg.png); border:0;}"));
//
//	if (!_ui->mainFrame->layout()) {
//		Widget::createLayout(_ui->mainFrame);
//	}
//	_ui->mainFrame->layout()->addWidget(_QtUMBase);
//	_ui->mainFrame->setContentsMargins(0,0,0,0);
//
//	resize(490, 470);
//
//	if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
//		QtChatWidget * widget = dynamic_cast<QtChatWidget *>(_QtUMBase);
//		if(widget){
//			widget->getIMChatSession()->typingStateChangedEvent +=
//			boost::bind(&QtPopUpChatDialog::typingStateChangedEventHandler, this, _1, _2, _3);
//		}		
//	}
//
//	SAFE_CONNECT(_QtUMBase, SIGNAL(implodeChat()), SLOT(implodeButtonClickedSlot()));//VOXOX CHANGE by Rolando - 2009.06.08
//	
//	SAFE_CONNECT_TYPE(this, SIGNAL(typingStateChangedSignal(const IMChatSession *, const IMContact *, const  IMChat::TypingState *)),
//		SLOT(typingStateChangedThreadSafe(const IMChatSession *, const IMContact *, const IMChat::TypingState *)), Qt::QueuedConnection);
//
//	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeWindow()));
//	this->installEventFilter(closeEventFilter);
//
//	setupToolBarActions();

}

QtPopUpChatDialog::~QtPopUpChatDialog() {
	//VOXOX CHANGE by Rolando - 2009.06.10 
	//OWSAFE_DELETE(_ui);
}

//VOXOX CHANGE by Rolando - 2009.06.15
QtUMBase * QtPopUpChatDialog::getQtUMBase(){
	/*if(_QtUMBase){
		return _QtUMBase;
	}
	else{
		return NULL;
	}*/
	return NULL;
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::show() {
	/*_qtUMBase->show();
	QDialog::show();

	if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
		QtChatWidget * widget = dynamic_cast<QtChatWidget *>(_QtUMBase);
		if (widget) {
			widget->getChatEdit()->setFocus();
			return;
		}
	}
	else if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){
			QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_QtUMBase);
			if (widget) {				
				widget->getChatEdit()->setFocus();
				return;				
			}
	}
	else if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
		QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(_QtUMBase);
		if (widget) {
			widget->getChatEdit()->setFocus();
			return;
		}
	}*/
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatDialog::implodeButtonClickedSlot(){
	
	//if(_QtUMBase){
	//	_ui->mainFrame = new QFrame();//VOXOX CHANGE by Rolando - 2009.06.10 - hack to avoid to delete _QtUMBase value when close window
	//	
	//	QString contactId = _QtUMBase->getContactId();//VOXOX CHANGE by Rolando - 2009.06.15

	//	//VOXOX CHANGE by Rolando - 2009.06.15 
	//	if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
	//		QtChatWidget * widget = dynamic_cast<QtChatWidget *>(_QtUMBase);
	//		if (widget) {
	//			contactId = widget->getContactId();//VOXOX CHANGE by Rolando - 2009.06.15 
	//			
	//		}
	//	}
	//	else if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){//VOXOX CHANGE by Rolando - 2009.06.15 
	//			QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_QtUMBase);
	//			if (widget) {
	//				contactId = widget->getEmail();//VOXOX CHANGE by Rolando - 2009.06.15 
	//			
	//			}
	//	}
	//	else if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){//VOXOX CHANGE by Rolando - 2009.06.15 
	//		QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(_QtUMBase);
	//		if (widget) {
	//			contactId = widget->getNumber();//VOXOX CHANGE by Rolando - 2009.06.15 
	//			
	//		}
	//	}
	//	
	//	implodeButtonClicked(contactId);
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::flashWindow(){
	//WidgetUtils::flashWindow(this);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::closeWindow(){

	//QString contactId = _QtUMBase->getContactId();//VOXOX CHANGE by Rolando - 2009.06.15 
	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
	//	QtChatWidget * widget = dynamic_cast<QtChatWidget *>(_QtUMBase);
	//	if (widget) {
	//		contactId = widget->getContactId();
	//		
	//	}
	//}
	//else if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){
	//		QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_QtUMBase);
	//		if (widget) {
	//			contactId = widget->getEmail();
	//		
	//		}
	//}
	//else if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
	//	QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(_QtUMBase);
	//	if (widget) {
	//		contactId = widget->getNumber();
	//		
	//	}
	//}



	//closeWindowSignal(contactId);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setupToolBarActions() {

	//SAFE_CONNECT(_ui->chatButton, SIGNAL(clicked()), SLOT(startChat()));
	//SAFE_CONNECT(_ui->callButton, SIGNAL(clicked()), SLOT(startCall()));
	//SAFE_CONNECT(_ui->smsButton, SIGNAL(clicked()), SLOT(startSMS()));
	//SAFE_CONNECT(_ui->sendFileButton, SIGNAL(clicked()), SLOT(startSendFile()));

	//SAFE_CONNECT(_ui->profileButton, SIGNAL(clicked()), SLOT(startProfile()));
	//SAFE_CONNECT(_ui->emailButton, SIGNAL(clicked()), SLOT(startEmailChat()));
	//SAFE_CONNECT(_ui->faxButton, SIGNAL(clicked()), SLOT(startFax()));
	//SAFE_CONNECT(_ui->phoneSettingsButton, SIGNAL(clicked()), SLOT(phoneSettings()));

	//setBtnChatChecked(false);
	//setBtnCallButtonChecked(false);
	//setSMSButtonChecked(false);
	//setSendFileButtonChecked(false);
	//setProfileButtonChecked(false);
	//setEmailButtonChecked(false);
	//setFaxButtonChecked(false);
	//setPhoneSettingsButtonChecked(false);

	//QtEnumUMMode::Mode mode= _QtUMBase->getMode();

	//if(mode==QtEnumUMMode::UniversalMessageChat){
	//	 //buildChatStatusMenus();//VOXOX CHANGE by Rolando - 2009.06.11 - necessary only if we use a qtstatusbar
	//	 setBtnChatChecked(true);
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageCall){
	//	_ui->callButton->setChecked(true);
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageConferenceCall){
	//	_ui->callButton->setChecked(true);
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageChatToSMS){
	//	_ui->smsButton->setChecked(true);
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageFileTransfer){
	//	  _ui->sendFileButton->setChecked(true);
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageContactProfile){
	//	  _ui->profileButton->setChecked(true);
	//}else if(mode==QtEnumUMMode::UniversalMessageChatToEmail){
	//	_ui->emailButton->setChecked(true);
	//}

}

//VOXOX CHANGE by Rolando - 2009.06.11 
void QtPopUpChatDialog::typingStateChangedEventHandler(IMChatSession & sender, const IMContact & imContact, IMChat::TypingState state) {
	/*IMChat::TypingState * tmpState = new IMChat::TypingState;
	*tmpState = state;
	typingStateChangedSignal(&sender,&imContact,tmpState);
}*/

}

//VOXOX CHANGE by Rolando - 2009.06.11 
void QtPopUpChatDialog::typingStateChangedThreadSafe(const IMChatSession * sender, const IMContact * from,const IMChat::TypingState * state) {

//	//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGETS
//	if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
//		QtChatWidget * widget = (QtChatWidget *) _qtUMBase;
//		if (widget->getSessionId() == sender->getId()) {
//		
//			//only display mssage if IMChatSession is active
//			if (!widget->isVisible()) {
//				return;
//			}
//		
//			QString remoteName = QString::fromUtf8(from->getDisplayContactId().c_str());
//			switch (*state) {
//			case IMChat::TypingStateNotTyping:
//				setChatWindowTitle(_QtUMBase->getTabText());
//				break;
//			case IMChat::TypingStateTyping:
//				setChatWindowTitle(tr("%1 is typing").arg(_QtUMBase->getTabText()));
//				break;
//			case IMChat::TypingStateStopTyping:
//				setChatWindowTitle(_QtUMBase->getTabText());
//				break;
//			default:
//				break;
//			}
//		}
//	}
//
//	OWSAFE_DELETE(state);
//
}

//VOXOX CHANGE by Rolando - 2009.06.11 
void QtPopUpChatDialog::setChatWindowTitle(const QString & text) {

	//QString typeText="";
	//QtEnumUMMode::Mode mode= _QtUMBase->getMode();
	//
	//if(mode==QtEnumUMMode::UniversalMessageChat){
	//	 typeText = "Chat";
	//}
	//if(mode==QtEnumUMMode::UniversalMessageCall){
	//	 typeText = "Call";
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageFileTransfer){
	//	 typeText = "Send File";
	//}
	//else if(mode==QtEnumUMMode::UniversalMessageContactProfile){
	//	typeText = "Profile";
	//}else if(mode==QtEnumUMMode::UniversalMessageChatToEmail){
	//	typeText = "Email";
	//}else if(mode==QtEnumUMMode::UniversalMessageChatToSMS){
	//	typeText = "SMS";
	//}

	//setWindowTitle(typeText +" "+ text);



}

//VOXOX CHANGE by Rolando - 2009.06.15
QtEnumUMMode::Mode QtPopUpChatDialog::getWidgetMode(){
	//if(_QtUMBase){
	//	return _QtUMBase->getMode();
	//}else{
	return QtEnumUMMode::UniversalMessageUnknown;
	//}
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startChat(){
	////VOXOX CHANGE by Rolando - 2009.07.09 
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	//contactActionManager->chatContact(_QtUMBase->getContactId());
	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
	//	_ui->chatButton->setChecked(true);
	//}
	//else{
	//	_ui->chatButton->setChecked(false);
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startCall(){
	////VOXOX CHANGE by Rolando - 2009.07.09	
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	//contactActionManager->callContact(_QtUMBase->getContactId());
	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageCall){
	//	_ui->callButton->setChecked(true);
	//}
	//else{
	//	_ui->callButton->setChecked(false);
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startSMS(){
	////VOXOX CHANGE by Rolando - 2009.07.09	
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	//contactActionManager->smsContact(_QtUMBase->getContactId());
	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
	//	_ui->smsButton->setChecked(true);
	//}
	//else{
	//	_ui->smsButton->setChecked(false);
	//}
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startSendFile(){
	////VOXOX CHANGE by Rolando - 2009.07.09	
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	//contactActionManager->sendFileContact(_QtUMBase->getContactId());
	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageFileTransfer){
	//	_ui->sendFileButton->setChecked(true);
	//}
	//else{
	//	_ui->sendFileButton->setChecked(false);
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startProfile(){
	////VOXOX CHANGE by Rolando - 2009.07.09 
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	//contactActionManager->profileContact(_QtUMBase->getContactId());
	///*if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageContactProfile){
	//	_ui->profileButton->setChecked(true);
	//}
	//else{
	//	_ui->profileButton->setChecked(false);
	//}*/		
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startEmailChat(){
	////VOXOX CHANGE by Rolando - 2009.07.09	
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	//contactActionManager->emailContact(_QtUMBase->getContactId());
	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){
	//	_ui->emailButton->setChecked(true);
	//}
	//else{
	//	_ui->emailButton->setChecked(false);
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::startFax(){
	//VOXOX CHANGE by Rolando - 2009.07.09 
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);//VOXOX - CJC - 2009.06.23 
	//contactActionManager->sendFaxContact(_QtUMBase->getContactId());

	//if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessageFax){
	//	_ui->faxButton->setChecked(true);
	//}
	//else{
	//	_ui->faxButton->setChecked(false);
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::phoneSettings(){
	////VOXOX CHANGE by Rolando - 2009.07.09
	//QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	////TODO: FIX this when exists a way to open directly to edit phone settings.
	//contactActionManager->profileContact(_QtUMBase->getContactId());
	///*if(_QtUMBase->getMode() == QtEnumUMMode::UniversalMessagePhoneSettings){
	//	_ui->phoneSettingsButton->setChecked(true);
	//}
	//else{
	//	_ui->phoneSettingsButton->setChecked(false);
	//}*/	
}

//Utilitary methods
//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setBtnChatEnabled(bool enable){
	//_ui->chatButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setBtnCallButtonEnabled(bool enable){
	//_ui->callButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setSMSButtonEnabled(bool enable){
	//_ui->smsButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setSendFileButtonEnabled(bool enable){
	//_ui->sendFileButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setProfileButtonEnabled(bool enable){
	//_ui->profileButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setEmailButtonEnabled(bool enable){
	//_ui->emailButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setFaxButtonEnabled(bool enable){
	//_ui->faxButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setPhoneSettingsButtonEnabled(bool enable){
	//_ui->phoneSettingsButton->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setButtonFrameEnabled(bool enable){
	//_ui->buttonFrame->setEnabled(enable);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setBtnChatChecked(bool checked){
	//_ui->chatButton->setChecked(checked);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setBtnCallButtonChecked(bool checked){
	//_ui->callButton->setChecked(checked);
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setSMSButtonChecked(bool checked){
	//_ui->smsButton->setChecked(checked);	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setSendFileButtonChecked(bool checked){
	//_ui->sendFileButton->setChecked(checked);	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setProfileButtonChecked(bool checked){
	//_ui->profileButton->setChecked(checked);	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setEmailButtonChecked(bool checked){
	//_ui->emailButton->setChecked(checked);	
}

//VOXOX CHANGE by Rolando - 2009.06.15
void QtPopUpChatDialog::setFaxButtonChecked(bool checked){
	//_ui->faxButton->setChecked(checked);	
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatDialog::setPhoneSettingsButtonChecked(bool checked){
	//_ui->phoneSettingsButton->setChecked(checked);	
}