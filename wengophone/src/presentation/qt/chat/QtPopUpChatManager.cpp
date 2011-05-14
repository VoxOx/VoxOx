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
* CLASS QtPopUpChatManager 
* @author Rolando 
* @date 2009.06.10
*/



#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtPopUpChatManager.h"
#include "QtUMBase.h"

#include "QtUMChat.h"
//#include "QtCallWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
//#include "QtChatConferenceCallWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28  
#include "QtChatToEmailSMSUtils.h"
//#include "QtChatToEmailWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
//#include "QtChatToSMSWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
#include "QtChatToEmailMessageData.h"
#include "QtChatToSMSMessageData.h"
//#include "QtSendFileWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 


#include <qtutil/SafeConnect.h>
#include <QtGui/QtGui>


//VOXOX CHANGE by Rolando - 2009.06.15 
QtPopUpChatManager * QtPopUpChatManager::instance = NULL;

QtPopUpChatManager::QtPopUpChatManager()
	:QObject()
{
}

//VOXOX CHANGE by Rolando - 2009.06.15 
QtPopUpChatManager::~QtPopUpChatManager() {
	_chatExplodedMap.clear();
	
}

//VOXOX CHANGE by Rolando - 2009.06.15 
QtPopUpChatManager *QtPopUpChatManager::getInstance() {

	if (!instance) {
		instance = new QtPopUpChatManager();
	}

	return instance;
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::addExplodedChatToMap(QtPopUpChatDialog * qtPopUpChatDialog){

	QtUMBase * universalMessageWidget = qtPopUpChatDialog->getQtUMBase();
	if(universalMessageWidget){
		QString contactId = universalMessageWidget->getContactId();
		if(universalMessageWidget->getMode() == QtEnumUMMode::UniversalMessageChat){
			QtUMChat * widget = dynamic_cast<QtUMChat *>(universalMessageWidget);
			contactId = widget->getContactId();
		}
		/*else if(universalMessageWidget->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){
				QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(universalMessageWidget);
				contactId = widget->getEmail();
		}*/

		//VOXOX CHANGE by Rolando - 2009.10.28 - compilation problems fixed
		/*else if(universalMessageWidget->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
				QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(universalMessageWidget);
				contactId = widget->getNumber();
		}*/

		_chatExplodedMap[contactId] = qtPopUpChatDialog;
		SAFE_CONNECT(qtPopUpChatDialog, SIGNAL(closeWindowSignal(QString)), SLOT(closeWindowSlot(QString)));
		SAFE_CONNECT(qtPopUpChatDialog, SIGNAL(implodeButtonClicked(QString)), SLOT(implodeButtonClickedSlot(QString)));
	}
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::deleteExplodedChatByContactId(QString contactId){
	_chatExplodedMap.erase(contactId);
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::closeWindowSlot(QString contactId){
	QtPopUpChatDialog * qtPopUpChatDialog = getQtPopUpChatDialogByContactId(contactId);
	if(qtPopUpChatDialog){
		deleteExplodedChatByContactId(contactId);
		qtPopUpChatDialog->deleteLater();
	}
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::implodeButtonClickedSlot(QString contactId){
	QtPopUpChatDialog * qtPopUpChatDialog = getQtPopUpChatDialogByContactId(contactId);
	if(qtPopUpChatDialog){
		addPopUpChatToTabWidget(qtPopUpChatDialog->getQtUMBase());
		deleteExplodedChatByContactId(contactId);
		qtPopUpChatDialog->deleteLater();
	}
}

//VOXOX CHANGE by Rolando - 2009.06.15 
QtPopUpChatDialog * QtPopUpChatManager::getQtPopUpChatDialogByContactId(QString contactId){
	std::map<QString, QtPopUpChatDialog *>::iterator it;
	it=_chatExplodedMap.find (contactId);

	if(it!= _chatExplodedMap.end()){
		return it->second;
	}
	else{
		return NULL;
	}
}

//VOXOX CHANGE by Rolando - 2009.06.15 
int QtPopUpChatManager::getCountOfPopUpChatByMode(QtEnumUMMode::Mode mode){
	std::map<QString, QtPopUpChatDialog *>::iterator iter;
	QtUMBase * tmpQtUMBase = NULL;
	int counter = 0;

	for( iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
		tmpQtUMBase = iter->second->getQtUMBase();
		if(tmpQtUMBase){
			if(tmpQtUMBase->getMode() == mode){
				++counter;
			}
		}
	}

	return counter;
}

//VOXOX CHANGE by Rolando - 2009.06.15 
bool QtPopUpChatManager::existPopUpChatWidgetWithSessionId(int sessionId){
	std::map<QString, QtPopUpChatDialog *>::iterator iter;
	QtUMBase * tmpQtUMBase = NULL;

	for(iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
		tmpQtUMBase = iter->second->getQtUMBase();
		if(tmpQtUMBase){
			if(tmpQtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
				QtUMChat * widget = dynamic_cast<QtUMChat *>(tmpQtUMBase);
				if (widget->getSessionId() == sessionId) {
					iter->second->show();
					return true;
				}
			}			
		}
	}

	return false;
}

//VOXOX CHANGE by Rolando - 2009.06.15 
bool QtPopUpChatManager::existPopUpChatToEmail(int sessionId, QString email){
	/*std::map<QString, QtPopUpChatDialog *>::iterator iter;
	QtUMBase * tmpQtUMBase = NULL;

	for(iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
		tmpQtUMBase = iter->second->getQtUMBase();
		if(tmpQtUMBase){
			if(tmpQtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){
				QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(tmpQtUMBase);
				if (widget->getSessionId() == sessionId && widget->getEmail() == email) {
					iter->second->show();
					return true;
				}
			}			
		}
	}

	return false;*/
	return false;
}

//VOXOX CHANGE by Rolando - 2009.06.17
bool QtPopUpChatManager::existPopUpChatToSMS(int sessionId, QString contactId, QString number){
	
	//VOXOX CHANGE by Rolando - 2009.10.28 - problems with class QtChatToSMSWidget that not exists
	/*std::map<QString, QtPopUpChatDialog *>::iterator iter;
	QtUMBase * tmpQtUMBase = NULL;

	for(iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
		tmpQtUMBase = iter->second->getQtUMBase();
		if(tmpQtUMBase){
			if(tmpQtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
				QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(tmpQtUMBase);
				if (widget->getSessionId() == sessionId && widget->getContactId() == contactId) {
					iter->second->show();
					return true;
				}
				else if (widget->getSessionId() == sessionId && widget->getNumber() == number) {
					iter->second->show();
					return true;
				}
			}

		}
	}*/

	return false;
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::verifyChatWidgetMessageReceived(int sessionId, QString contactId, QString senderDisplayName, QString protocol, QString msg, bool & contactWasFound){
	//std::map<QString, QtPopUpChatDialog *>::iterator iter;
	//QtUMBase * tmpQtUMBase = NULL;
	//contactWasFound = false;

	//for( iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
	//	tmpQtUMBase = iter->second->getQtUMBase();
	//	if(tmpQtUMBase){
	//		if(tmpQtUMBase->getMode() == QtEnumUMMode::UniversalMessageChat){
	//			QtUMChat * widget = dynamic_cast<QtUMChat *>(tmpQtUMBase);
	//			if (widget->getSessionId() == sessionId) {
	//				QtUMChat * chatWidget = qobject_cast<QtUMChat *>(tmpQtUMBase);
	//				chatWidget->addToHistory(contactId, senderDisplayName,protocol, msg);
	//				iter->second->flashWindow();
	//				contactWasFound = true;
	//				break;
	//			}
	//		}			
	//	}
	//}	
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::verifyChatToEmailMessageReceived(int sessionId, QString contactId, QString email, QString senderDisplayName, QString protocol, QString msg, bool & contactWasFound){
	/*std::map<QString, QtPopUpChatDialog *>::iterator iter;
	QtUMBase * tmpQtUMBase = NULL;

	contactWasFound = false;

	for( iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
		tmpQtUMBase = iter->second->getQtUMBase();
		if(tmpQtUMBase){
			if(tmpQtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToEmail){
				QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(tmpQtUMBase);
				if (widget->getSessionId() == sessionId && widget->getEmail() == email) {
					QtChatToEmailWidget * chatToEmailWidget = qobject_cast<QtChatToEmailWidget *>(tmpQtUMBase);
					chatToEmailWidget->addToHistory(contactId, senderDisplayName,protocol, msg);
					iter->second->flashWindow();
					contactWasFound = true;
					break;
				}
			}			
		}
	}	*/
}

//VOXOX CHANGE by Rolando - 2009.06.15 
void QtPopUpChatManager::verifyChatToSMSMessageReceived(int sessionId, QString contactId, QString number, QString senderDisplayName, QString protocol, QString msg, QString contactKey, bool & contactWasFound){
	//std::map<QString, QtPopUpChatDialog *>::iterator iter;
	//QtUMBase * tmpQtUMBase = NULL;
	//contactWasFound = false;

	//for( iter = _chatExplodedMap.begin(); iter != _chatExplodedMap.end(); ++iter ) {
	//	tmpQtUMBase = iter->second->getQtUMBase();
	//	if(tmpQtUMBase){
	//		if(tmpQtUMBase->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
	//			QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(tmpQtUMBase);
	//			if (widget->getSessionId() == sessionId && widget->getContactId() == contactKey) {//VOXOX CHANGE by Rolando - 2009.06.17 
	//				QtChatToSMSWidget * chatToSMSWidget = qobject_cast<QtChatToSMSWidget *>(tmpQtUMBase);
	//				chatToSMSWidget->addToHistory(contactId, senderDisplayName,protocol, msg);
	//				iter->second->flashWindow();
	//				contactWasFound = true;
	//				break;
	//			}
	//			else if (widget->getSessionId() == sessionId && widget->getNumber() == number) {//VOXOX CHANGE by Rolando - 2009.06.17 
	//				QtChatToSMSWidget * chatToSMSWidget = qobject_cast<QtChatToSMSWidget *>(tmpQtUMBase);
	//				chatToSMSWidget->addToHistory(contactId, senderDisplayName,protocol, msg);
	//				iter->second->flashWindow();
	//				contactWasFound = true;
	//				break;
	//			}
	//		}
	//	}
	//}	
}
//VOXOX CHANGE by Rolando - 2009.06.11 
int QtPopUpChatManager::getNumberPopUpChatsOpen(){
	return _chatExplodedMap.size();
}



