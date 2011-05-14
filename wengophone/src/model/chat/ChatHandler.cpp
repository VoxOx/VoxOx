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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ChatHandler.h"

#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMWrapperFactory.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

ChatHandler::ChatHandler(UserProfile & userProfile)
	: _userProfile(userProfile) 
{
	_userProfile.getIMAccountManager().imAccountAddedEvent   += boost::bind(&ChatHandler::imAccountAddedEventHandler,   this, _1, _2);
	_userProfile.getIMAccountManager().imAccountRemovedEvent += boost::bind(&ChatHandler::imAccountRemovedEventHandler, this, _1, _2);
}

ChatHandler::~ChatHandler() 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	for (IMChatSessionList::const_iterator it = _imChatSessionList.begin(); it != _imChatSessionList.end(); it++) 
	{
		delete (*it);
	}

	_imChatSessionList.clear();	//VOXOX - JRT - 2009.07.12 

	for (IMChatMap::const_iterator it = _imChatMap.begin(); it != _imChatMap.end(); ++it) 
	{
		delete (*it).second;
	}
	_imChatMap.clear();		//VOXOX - JRT - 2009.07.12 
}

void ChatHandler::createSession(const std::string & imAccountId, const IMContactSet & imContactSet, 
								IMChat::IMChatType imChatType, const std::string userChatRoomName ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChat* imChat = findIMChatByAccount( imAccountId );

	if ( imChat )
	{
		LOG_DEBUG("creating new IMChatSession for: " + imAccountId);
		imChat->createSession(imContactSet, imChatType, userChatRoomName);
	}

	//VOXOX - JRT - 2009.07.10 
	//IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	//if (imAccount) 
	//{
	//	LOG_DEBUG("creating new IMChatSession for: " + imAccount->getLogin());
	//	IMChatMap::iterator it = _imChatMap.find(imAccount->getKey());
	//	if (it != _imChatMap.end()) 
	//	{
	//		(*it).second->createSession(imContactSet, imChatType, userChatRoomName);
	//	} 
	//	else 
	//	{
	//		LOG_ERROR("No IMChat created for this IMAccount");
	//	}
	//	OWSAFE_DELETE(imAccount);
	//} 
	//else 
	//{
	//	LOG_ERROR("IMAccount not found!");
	//}
}

//VOXOX - CJC - 2009.05.07 This will create a chat session to ask.im.voxox.com with the associated contact
//We will need to open a sesion to im.voxox.com but also mantain the contact that will use 
void ChatHandler::createChatToEmailSession(const std::string & imAccountId, const IMContactSet & imContactSet,const std::string & relatedContactId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChat* imChat = findIMChatByAccount( imAccountId );

	if ( imChat )
	{
		LOG_DEBUG("creating new IMChatSession for: " + imAccountId);
		imChat->createChatToEmailSession(imContactSet,relatedContactId);
	}

	//VOXOX - JRT - 2009.07.10 
	//IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	//if (imAccount) 
	//{
	//	LOG_DEBUG("creating new IMChatSession for: " + imAccount->getLogin());
	//	IMChatMap::iterator it = _imChatMap.find(imAccount->getKey());
	//	if (it != _imChatMap.end()) 
	//	{
	//		(*it).second->createChatToEmailSession(imContactSet,relatedContactId);
	//	} 
	//	else
	//	{
	//		LOG_ERROR("No IMChat created for this IMAccount");
	//	}
	//	OWSAFE_DELETE(imAccount);
	//} 
	//else
	//{
	//	LOG_ERROR("IMAccount not found!");
	//}
}

//VOXOX - CJC - 2009.05.19  This will create a chat session to ask.im.voxox.com with the associated contact
//We will need to open a sesion to im.voxox.com but also mantain the contact that will use 
void ChatHandler::createChatToSMSSession(const std::string & imAccountId, const IMContactSet & imContactSet,const std::string & relatedContactId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChat* imChat = findIMChatByAccount( imAccountId );

	if ( imChat )
	{
		LOG_DEBUG("creating new Im Chat to SMS for: " + imAccountId);
		imChat->createChatToSMSSession(imContactSet,relatedContactId);
	}

	//VOXOX - JRT - 2009.07.10 
	//IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	//if (imAccount) 
	//{
	//	LOG_DEBUG("creating new Im Chat to SMS for: " + imAccount->getLogin());
	//	IMChatMap::iterator it = _imChatMap.find(imAccount->getKey());
	//	if (it != _imChatMap.end()) 
	//	{
	//		(*it).second->createChatToSMSSession(imContactSet,relatedContactId);
	//	} 
	//	else 
	//	{
	//		LOG_ERROR("No IMChat created for this IMAccount");
	//	}
	//	OWSAFE_DELETE(imAccount);
	//}
	//else 
	//{
	//	LOG_ERROR("IMAccount not found!");
	//}
}


//VOXOX CHANGE CJC CALL SKYPE CONTACT
void ChatHandler::callSkypeContact(const std::string & imAccountId, const IMContactSet & imContactSet) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChat* imChat = findIMChatByAccount( imAccountId );

	if ( imChat )
	{
		LOG_DEBUG("calling Skype Contact for: " + imAccountId);
		imChat->callSkypeContact(imContactSet);
	}

	//VOXOX - JRT - 2009.07.10 
	//IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	//if (imAccount) 
	//{
	//	LOG_DEBUG("calling Skype Contact for: " + imAccount->getLogin());
	//	IMChatMap::iterator it = _imChatMap.find(imAccount->getKey() );
	//	if (it != _imChatMap.end()) 
	//	{
	//		(*it).second->callSkypeContact(imContactSet);
	//	} 
	//	else 
	//	{
	//		LOG_ERROR("No IMChat created for this IMAccount");
	//	}
	//	OWSAFE_DELETE(imAccount);
	//} 
	//else 
	//{
	//	LOG_ERROR("IMAccount not found!");
	//}
}

//VOXOX CHANGE CJC CREATE SESSION AND SEND MESSAGE AT THE SAME TIME
void ChatHandler::createSessionAndSendMessage(const std::string & imAccountId, const IMContactSet & imContactSet,const std::string & message) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChat* imChat = findIMChatByAccount( imAccountId );

	if ( imChat )
	{
		LOG_DEBUG("creating new IMChatSession for: " + imAccountId);
		LOG_DEBUG("Sending Message: " + message);
		imChat->createSessionAndSendMessage(imContactSet,message);
	}

	//VOXOX - JRT - 2009.07.10 
	//IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	//if (imAccount) 
	//{
	//	LOG_DEBUG("creating new IMChatSession for: " + imAccount->getLogin());
	//	LOG_DEBUG("Sending Message: " + message);
	//	IMChatMap::iterator it = _imChatMap.find( imAccount->getKey());
	//	if (it != _imChatMap.end()) 
	//	{
	//		(*it).second->createSessionAndSendMessage(imContactSet,message);
	//	} 
	//	else 
	//	{
	//		LOG_ERROR("No IMChat created for this IMAccount");
	//	}
	//	OWSAFE_DELETE(imAccount);
	//} 
	//else 
	//{
	//	LOG_ERROR("IMAccount not found!");
	//}
}

//VOXOX CHANGE CJC SEND FILE ON CHAT
void ChatHandler::createSendFile(const std::string & imAccountId, IMContact & imContact) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	if (imAccount) 
	{
		newSendFileCreatedEvent(*this, imContact);
		OWSAFE_DELETE(imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not found!");
	}
}

//VOXOX - CJC - 2009.06.23 
void ChatHandler::createSendFax(std::string & faxNumber) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	newSendFaxCreatedEvent(*this, faxNumber);
}


//VOXOX CHANGE CJC SEND FILE ON CHAT
void ChatHandler::createContactProfile(const std::string & imAccountId, IMContact & imContact) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	if (imAccount) 
	{
		newContactProfileCreatedEvent(*this, imContact);
		OWSAFE_DELETE(imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not found!");
	}
}

//VOXOX - CJC - 2009.06.03 
void ChatHandler::initChat()
{
	RecursiveMutex::ScopedLock lock(_mutex);

	initChatCreatedEvent(*this);
}

void ChatHandler::newIMChatSessionCreatedEventHandler(IMChat & sender, IMChatSession & imChatSession) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	LOG_DEBUG("a new IMChatSession has been created");
	_imChatSessionList.push_back(&imChatSession);
	imChatSession.imChatSessionWillDieEvent += boost::bind(&ChatHandler::imChatSessionWillDieEventHandler, this, _1);
	newIMChatSessionCreatedEvent(*this, imChatSession);
}

//VOXOX - CJC - 2009.05.07 Suport chat to email created sessions
void ChatHandler::newIMChatToEmailSessionCreatedEventHandler(IMChat & sender, IMChatSession & imChatSession) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	std::string relatedContactId = sender.getRelatedContactId();
	//VOXOX - CJC - 2009.05.07 Set variables to default
	sender.setIMChatType(IMChat::Chat);	//VOXOX - JRT - 2009.06.14 TODO: Chris, is this right?
	sender.setRelatedContactId("");

	LOG_DEBUG("a new IMChatToEmailSession has been created");
	_imChatSessionList.push_back(&imChatSession);
	imChatSession.imChatSessionWillDieEvent += boost::bind(&ChatHandler::imChatSessionWillDieEventHandler, this, _1);
	newIMChatToEmailSessionCreatedEvent(*this, imChatSession,relatedContactId);
}

//VOXOX - CJC - 2009.05.19  Suport chat to email created sessions
void ChatHandler::newIMChatToSMSSessionCreatedEventHandler(IMChat & sender, IMChatSession & imChatSession) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	std::string relatedContactId = sender.getRelatedContactId();
	//VOXOX - CJC - 2009.05.07 Set variables to default
	sender.setIMChatType(IMChat::Chat);	//VOXOX - JRT - 2009.06.14 TODO: Chris, is this right?
	sender.setRelatedContactId("");

	LOG_DEBUG("a new IMChatToSMSSession has been created");
	_imChatSessionList.push_back(&imChatSession);
	imChatSession.imChatSessionWillDieEvent += 	boost::bind(&ChatHandler::imChatSessionWillDieEventHandler, this, _1);
	newIMChatToSMSSessionCreatedEvent(*this, imChatSession,relatedContactId);
}

void ChatHandler::newIMChatAndSendMessageSessionCreatedEventHandler(IMChat & sender, IMChatSession & imChatSession, const std::string & message, bool killChatSession) 
{
	RecursiveMutex::ScopedLock lock(_mutex);
	//Clear Variables
	//VOXOX CHANGE CJC CLEAR MESSAGE
	sender.setAutoMessage("");
	//VOXOX CHANGE CJC SET AUTOMESSAGE FALSE
	
	std::string realMessage = message;
	
	if(killChatSession){//VOXOX - CJC - 2009.10.14 Session already exist, dont created it
		_imChatSessionList.push_back(&imChatSession);
		imChatSession.imChatSessionWillDieEvent += boost::bind(&ChatHandler::imChatSessionWillDieEventHandler, this, _1);
		newIMChatSessionCreatedEvent(*this, imChatSession);
	}

	LOG_DEBUG("a new IMChatSessionAndSendMessage has been created");
	//Send Message
	//imChatSession.sendMessage(message);

	newSendMessageEvent(*this, imChatSession,realMessage);

	////Kill Session
	////VOXOX CHANGE CJC IF SESSION IS OPEN WE DONT WANT TO KILL IT, JUST SEND THE MESSAGE
	//if(killChatSession)
	//{
	//	imChatSession.close();
	//}
}

void ChatHandler::imChatSessionWillDieEventHandler(IMChatSession & sender) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChatSessionList::iterator it = std::find(_imChatSessionList.begin(), _imChatSessionList.end(), &sender);

	if (it != _imChatSessionList.end()) 
	{
		_imChatSessionList.erase(it);
	} 
	else 
	{
		LOG_ERROR("IMChatSession not in ChatHandler");
	}
}

//VOXOX - JRT - 2009.07.10 
void ChatHandler::incomingGroupChatInviteEventHandler( IMChat& sender, GroupChatInfo& gcInfo) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	incomingGroupChatInviteEvent(*this, gcInfo );
}


void ChatHandler::imAccountAddedEventHandler(IMAccountManager & sender, std::string imAccountId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	if (imAccount) 
	{
		LOG_DEBUG("new account added: login: " + imAccount->getLogin() + " protocol: " + EnumIMProtocol::toString(imAccount->getProtocol()));

		IMChatMap::iterator it = _imChatMap.find(imAccountId);

		if (it == _imChatMap.end()) 
		{
			IMChat * imChat = IMWrapperFactory::getFactory().createIMChat(*imAccount);

			imChat->newIMChatSessionCreatedEvent				+= boost::bind(&ChatHandler::newIMChatSessionCreatedEventHandler,				this, _1, _2);
			imChat->newIMChatAndSendMessageSessionCreatedEvent	+= boost::bind(&ChatHandler::newIMChatAndSendMessageSessionCreatedEventHandler, this, _1, _2,_3,_4);//VOXOX CHANGE CJC ADD BIND TO CREATE SESSION AND SEND MESSAGE METHOD
			imChat->newIMChatToEmailSessionCreatedEvent			+= boost::bind(&ChatHandler::newIMChatToEmailSessionCreatedEventHandler,		this, _1, _2);	//VOXOX - CJC - 2009.05.07 VoxOx change get created chat to email session
			imChat->newIMChatToSMSSessionCreatedEvent			+= boost::bind(&ChatHandler::newIMChatToSMSSessionCreatedEventHandler,			this, _1, _2);	//VOXOX - CJC - 2009.05.19 Chat to sms
			imChat->incomingGroupChatInviteEvent				+= boost::bind(&ChatHandler::incomingGroupChatInviteEventHandler,				this, _1, _2);	//VOXOX - JRT - 2009.07.10 
	
			_imChatMap.insert(std::pair<std::string, IMChat *>(imAccountId, imChat));
		} 
		else 
		{
			LOG_ERROR("this IMAccount has already been added " + imAccount->getLogin());
		}

		OWSAFE_DELETE(imAccount);
	}
}

void ChatHandler::imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMChatMap::iterator it = _imChatMap.find(imAccountId);

	if (it != _imChatMap.end()) 
	{
		//TODO close all IMChatSession opened with this IMAccount
		delete (*it).second;
		_imChatMap.erase(it);
	} 
	else 
	{
		LOG_ERROR("this IMAccount has not been added: " + imAccountId);
	}
}

void ChatHandler::unregisterAllEvents() 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	_userProfile.getIMAccountManager().imAccountAddedEvent   -= boost::bind(&ChatHandler::imAccountAddedEventHandler,   this, _1, _2);
	_userProfile.getIMAccountManager().imAccountRemovedEvent -= boost::bind(&ChatHandler::imAccountRemovedEventHandler, this, _1, _2);

	for (IMChatMap::const_iterator it = _imChatMap.begin(); it != _imChatMap.end(); ++it) 
	{
		(*it).second->newIMChatSessionCreatedEvent				 -= boost::bind(&ChatHandler::newIMChatSessionCreatedEventHandler,				 this, _1, _2);
		(*it).second->newIMChatToEmailSessionCreatedEvent		 -= boost::bind(&ChatHandler::newIMChatToEmailSessionCreatedEventHandler,		 this, _1, _2);
		(*it).second->newIMChatToSMSSessionCreatedEvent			 -= boost::bind(&ChatHandler::newIMChatToSMSSessionCreatedEventHandler,			 this, _1, _2);
		(*it).second->newIMChatAndSendMessageSessionCreatedEvent -= boost::bind(&ChatHandler::newIMChatAndSendMessageSessionCreatedEventHandler, this, _1, _2, _3, _4);

		(*it).second->incomingGroupChatInviteEvent				 -= boost::bind(&ChatHandler::incomingGroupChatInviteEventHandler,				 this, _1, _2);	//VOXOX - JRT - 2009.07.10 

	}
}

IMChat*ChatHandler::findIMChatByAccount( const std::string& imAccountId )
{
	IMChat* result = false;

	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	if (imAccount) 
	{
		RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.07.12 

		IMChatMap::iterator it = _imChatMap.find(imAccount->getKey() );
		if (it != _imChatMap.end()) 
		{
			result = (*it).second;
		} 
		else 
		{
			LOG_ERROR("No IMChat created for this IMAccount");
		}

		OWSAFE_DELETE(imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not found!");
	}

	return result;
}
