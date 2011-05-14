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

#ifndef OWQTCHATHANDLER_H
#define OWQTCHATHANDLER_H

#include <presentation/PChatHandler.h>
#include <imwrapper/IMChat.h>

#include <QtCore/QObject>

class CChatHandler;
class IMAccount;
class IMContactSet;
class QtUMWindow;
class QtWengoPhone;

/**
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class QtChatHandler : public QObject, public PChatHandler {
	Q_OBJECT
public:

	QtChatHandler(CChatHandler & cChatHandler, QtWengoPhone & qtWengoPhone);

	~QtChatHandler();

	void createSession(const QString & imAccountId, IMContactSet & imContactSet, IMChat::IMChatType imChatType, const std::string& userChatRoomName );	//VOXOX - JRT - 2009.06.15 

	

	void newIMChatSessionCreatedEvent(IMChatSession & imChatSession);

	void newSendMessageCreatedEvent(IMChatSession & imChatSession, std::string & message);//VOXOX - CJC - 2009.10.14 

	//VOXOX - CJC - 2009.05.07 Add support for chat to email messages
	void newIMChatToEmailSessionCreatedEvent(IMChatSession & imChatSession, std::string & email);
	//VOXOX - CJC - 2009.05.19 Chat to sms
	void newIMChatToSMSSessionCreatedEvent(IMChatSession & imChatSession, std::string & number);

	
	void newSendFileCreatedEvent(IMContact & imContact);		//VOXOX CHANGE CJC TO SUPPORT SEND FILE IN CHAT
//	void newContactProfileCreatedEvent(IMContact & imContact);	//VOXOX CHANGE CJC SUPPORT CONTACT PROFILE IN CHAT	//VOXOX - JRT - 2009.06.30 
	void newSendFaxCreatedEvent(std::string & faxNumber);		//VOXOX - CJC - 2009.06.23 
	void newInitChatCreatedEvent();								//VOXOX - CJC - 2009.06.03 

	void incomingGroupChatInviteEvent( GroupChatInfo& gcInfo );	//VOXOX - JRT - 2009.07.10 

	void updatePresentation();
	void deletePresentation();

private:
	void initChatWindow();

	CChatHandler & _cChatHandler;

	QtUMWindow * _qtUMWindow;

	QtWengoPhone & _qtWengoPhone;
};

#endif	//OWQTCHATHANDLER_H
