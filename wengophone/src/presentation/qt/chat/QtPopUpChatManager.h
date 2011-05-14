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

#ifndef QTPOPUPCHATMANAGER_H
#define QTPOPUPCHATMANAGER_H

#include <QtCore/QString>
#include <QtCore/QObject>
#include <util/NonCopyable.h>

#include "QtPopUpChatDialog.h"


class QtPopUpChatManager:public QObject,NonCopyable  {
	Q_OBJECT
public:

	//VOXOX CHANGE by Rolando - 2009.06.15 
	static QtPopUpChatManager * getInstance();

	//VOXOX CHANGE by Rolando - 2009.06.15 
	QtPopUpChatManager();

	//VOXOX CHANGE by Rolando - 2009.06.15 
	~QtPopUpChatManager();

	//VOXOX CHANGE by Rolando - 2009.06.15 
	std::map<QString, QtPopUpChatDialog *> getChatExplodedMap(){ return _chatExplodedMap;}

	//VOXOX CHANGE by Rolando - 2009.06.15 
	void addExplodedChatToMap(QtPopUpChatDialog * qtPopUpChatDialog);

	//VOXOX CHANGE by Rolando - 2009.06.15 
	void deleteExplodedChatByContactId(QString contactId);

	//VOXOX CHANGE by Rolando - 2009.06.15 
	QtPopUpChatDialog * getQtPopUpChatDialogByContactId(QString contactId);

	//VOXOX CHANGE by Rolando - 2009.06.15 
	int getCountOfPopUpChatByMode(QtEnumUMMode::Mode mode);

	bool existPopUpChatWidgetWithSessionId(int sessionId);//VOXOX CHANGE by Rolando - 2009.06.12 
	bool existPopUpChatToEmail(int sessionId, QString email);//VOXOX CHANGE by Rolando - 2009.06.12 
	bool existPopUpChatToSMS(int sessionId, QString contactId, QString number);//VOXOX CHANGE by Rolando - 2009.06.17 

	void verifyChatWidgetMessageReceived(int sessionId, QString contactId, QString senderDisplayName, QString protocol, QString msg, bool & contactWasFound);//VOXOX CHANGE by Rolando - 2009.06.15 
	void verifyChatToEmailMessageReceived(int sessionId, QString contactId, QString email, QString senderDisplayName, QString protocol, QString msg, bool & contactWasFound);//VOXOX CHANGE by Rolando - 2009.06.15 
	void verifyChatToSMSMessageReceived(int sessionId, QString contactId, QString number, QString senderDisplayName, QString protocol, QString msg, QString contactKey, bool & contactWasFound);//VOXOX CHANGE by Rolando - 2009.06.15 



	int getNumberPopUpChatsOpen();//VOXOX CHANGE by Rolando - 2009.06.15 

public Q_SLOTS:

	void closeWindowSlot(QString contactId);//VOXOX CHANGE by Rolando - 2009.06.10 
	void implodeButtonClickedSlot(QString contactId);//VOXOX CHANGE by Rolando - 2009.06.10 

Q_SIGNALS:

	void addPopUpChatToTabWidget(QtUMBase*);//VOXOX CHANGE by Rolando - 2009.06.15 

private:	

	static QtPopUpChatManager * instance;//VOXOX CHANGE by Rolando - 2009.06.15 

	std::map<QString, QtPopUpChatDialog *> _chatExplodedMap;//VOXOX CHANGE by Rolando - 2009.06.10 
	
};

#endif	//QTPOPUPCHATMANAGER_H
