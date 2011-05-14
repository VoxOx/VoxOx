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

#ifndef OWCCHATHANDLER_H
#define OWCCHATHANDLER_H

#include <control/Control.h>
#include <imwrapper/IMChat.h>		//VOXOX - JRT - 2009.06.15 

#include <QtCore/QWaitCondition>	//VOXOX - JRT - 2009.07.11 
#include <QtCore/QMutex>			//VOXOX - JRT - 2009.07.11 

class Presentation;
class ChatHandler;
class CUserProfile;
class CContactList;		//VOXOX - JRT - OK - 2010.01.06 
class CWengoPhone;
class IMAccount;
class IMContactSet;
class IMContact;
class IMChatSession;
class PChatHandler;
class MessageTranslation;//VOXOX - CJC - 2010.01.22 

/**
 *
 * @ingroup control
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class CChatHandler : public Control {
public:

	CChatHandler(ChatHandler & chatHandler, CUserProfile & cUserProfile);

	~CChatHandler();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	CUserProfile & getCUserProfile() const;

	/**
	 * @see ChatHandler::createSession()
	 */
	void createSession(const std::string & imAccountId, const IMContactSet & imContactSet, 
						IMChat::IMChatType imChatType, const std::string& userChatRoomName);

	std::string getTranslationLanguageXML();
	MessageTranslation getMessageTranslationSettings(const std::string & key);	//VOXOX - CJC - 2010.01.22 

	void setMessageTranslationSettings(const std::string & key, const MessageTranslation & translation );
private:
	void initPresentationThreadSafe();

	void newIMChatSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession);

	void newSendMessageEventHandler(ChatHandler & sender, IMChatSession & imChatSession,std::string & message);//VOXOX - CJC - 2009.10.14 

	void newIMChatToEmailSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession, std::string & relatedContactId);	//VOXOX - CJC - 2009.05.07 
	void newIMChatToSMSSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession, std::string & relatedContactId);		//VOXOX - CJC - 2009.05.19 Chat to sms

	void newSendFileCreatedEventHandler(ChatHandler & sender, IMContact & imContact);	//VOXOX CHANGE CJC SUPPORT SEND FILE
	

	//VOXOX CHANGE CJC SUPPORT CONTACT PROFILE ONCHAT
//	void newContactProfileCreatedEventHandler(ChatHandler & sender, IMContact & imContact);	//VOXOX - JRT - 2009.06.30 

	//VOXOX - CJC - 2009.06.23 
	void newSendFaxCreatedEventHandler(ChatHandler & sender, std::string & faxNumber);
	

	void newInitChatCreatedEventHandler(ChatHandler & sender);
	void newIMChatSessionCreatedEventHandlerThreadSafe(IMChatSession & imChatSession);
	void newSendMessageEventHandlerThreadSafe(IMChatSession & imChatSession,std::string & message);//VOXOX - CJC - 2009.10.14 
	void newIMChatToEmailSessionCreatedEventHandlerThreadSafe(IMChatSession & imChatSession, std::string & relatedContactId);	//VOXOX - CJC - 2009.05.07 Create chat to email UI and interface
	void newIMChatToSMSSessionCreatedEventHandlerThreadSafe(IMChatSession & imChatSession, std::string & relatedContactId);		//VOXOX - CJC - 2009.05.19 Chat to sms

	void newSendFileCreatedEventHandlerThreadSafe(IMContact & imContact);
//	void newContactProfileCreatedEventHandlerThreadSafe(IMContact & imContact);	//VOXOX - JRT - 2009.06.30 
	void newInitChatCreatedEventHandlerThreadSafe();							//VOXOX - CJC - 2009.06.03 
	void newSendFaxCreatedEventHandlerThreadSafe(std::string & faxNumber);		//VOXOX - CJC - 2009.06.23 

	void incomingGroupChatInviteEventHandler		  ( ChatHandler& sender, GroupChatInfo& gcInfo);	//VOXOX - JRT - 2009.07.10 
	void incomingGroupChatInviteEventHandlerThreadSafe( const GroupChatInfo& gcInfo );					//VOXOX - JRT - 2009.10.06 

	CContactList& getCContactList();	//VOXOX - JRT - OK - 2010.01.06 
	ChatHandler&  _chatHandler;
	PChatHandler* _pChatHandler;
	CUserProfile& _cUserProfile;

	//VOXOX - JRT - 2009.07.11 
	void waitForInviteResponse();
	void respondToInviteEventHandler( const GroupChatInfo& gcInfo );

	mutable RecursiveMutex	_mutex;
	QMutex					_waitMutex;
	QWaitCondition			_waitGroupChatInvite;
	bool					_waitMutexLocked;
};

#endif	//OWCCHATHANDLER_H
