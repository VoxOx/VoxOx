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
#include "QtChatHandler.h"

#include "QtUMWindow.h"
#include "emoticons/QtEmoticonsManager.h"

#include <control/chat/CChatHandler.h>

#include <presentation/qt/QtWengoPhone.h>

#include <util/Logger.h>

QtChatHandler::QtChatHandler(CChatHandler & cChatHandler, QtWengoPhone & qtWengoPhone)
	: QObject(NULL),
	_cChatHandler(cChatHandler),
	_qtWengoPhone(qtWengoPhone) 
{
	_qtUMWindow = NULL;
}

QtChatHandler::~QtChatHandler() 
{
	if (_qtUMWindow) 
	{
		_qtUMWindow->hide();
		delete _qtUMWindow;
	}
}

void QtChatHandler::newIMChatSessionCreatedEvent(IMChatSession & imChatSession) 
{
	initChatWindow();
	_qtUMWindow->imChatSessionCreatedHandler(&imChatSession);
}


void QtChatHandler::newSendMessageCreatedEvent(IMChatSession & imChatSession, std::string & message) //VOXOX - CJC - 2009.10.14 
{
	initChatWindow();
	_qtUMWindow->sendMessageCreatedHandler(&imChatSession,message);
}

//VOXOX - CJC - 2009.05.07 Add chat to email support
void QtChatHandler::newIMChatToEmailSessionCreatedEvent(IMChatSession & imChatSession, std::string & email ) 
{
	initChatWindow();
	_qtUMWindow->imChatToEmailSessionCreatedHandler(&imChatSession,email);
	
}
//VOXOX - CJC - 2009.05.19 Chat to sms
void QtChatHandler::newIMChatToSMSSessionCreatedEvent(IMChatSession & imChatSession, std::string & number) 
{
	initChatWindow();
	_qtUMWindow->imChatToSMSSessionCreatedHandler(&imChatSession,number);
}


//VOXOX CHANGE CJC ADD SUPPORT FOR SEND FILE
void QtChatHandler::newSendFileCreatedEvent(IMContact & imContact) 
{
	initChatWindow();
	_qtUMWindow->addSendFileWidgetTab(imContact);
	
}

//VOXOX CHANGE CJC ADD SUPPORT FOR SEND FILE
void QtChatHandler::newSendFaxCreatedEvent(std::string & faxNumber) 
{
	initChatWindow();
	_qtUMWindow->addSendFaxWidgetTab(faxNumber);
	
}

//VOXOX - JRT - 2009.07.10 
void QtChatHandler::incomingGroupChatInviteEvent( GroupChatInfo& gcInfo )
{
	_qtUMWindow->incomingGroupChatInvite( gcInfo );
} 


//VOXOX CHANGE CJC ADD SUPPORT FOR SEND FILE
//VOXOX - JRT - 2009.06.30 - Now done in separate window.
//void QtChatHandler::newContactProfileCreatedEvent(IMContact & imContact) 
//{
//	initChatWindow();
//	_qtUMWindow->addContactProfileWidgetTab(imContact);
//}

//VOXOX CHANGE CJC ADD SUPPORT FOR SEND FILE
void QtChatHandler::newInitChatCreatedEvent() 
{
	initChatWindow();
}

void QtChatHandler::initChatWindow()
{
	if (!_qtUMWindow) 
	{
// VOXOX CHANGE by ASV 05-22-2009: This fixes a bug on the windows version that was not showing the chat window when in the task bar		
#if defined(OS_MACOSX)
		_qtUMWindow =  new QtUMWindow(_qtWengoPhone.getWidget(), _cChatHandler, _qtWengoPhone);
#else
		_qtUMWindow =  new QtUMWindow(NULL, _cChatHandler, _qtWengoPhone);
#endif
	} 
}


void QtChatHandler::createSession(const QString & imAccountId, IMContactSet & imContactSet, 
									IMChat::IMChatType imChatType, const std::string& userChatRoomName ) 
{
	_cChatHandler.createSession(imAccountId.toStdString(), imContactSet, imChatType, userChatRoomName);
}

void QtChatHandler::updatePresentation() 
{
}

void QtChatHandler::deletePresentation() 
{
	// we should delete the QtUMWindow
	// but it crashes in QtEmoticonsManager::getQtEmoticonList
	/* delete(this); */
	if (_qtUMWindow) 
	{
		//_qtUMWindow->closeWindow();
		_qtUMWindow->close();
		_qtWengoPhone.setChatWindow(NULL);
	}
}

