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
#include "CChatHandler.h"

#include <presentation/PChatHandler.h>
#include <presentation/PFactory.h>

#include <control/profile/CUserProfile.h>
#include <control/CWengoPhone.h>

#include <model/chat/ChatHandler.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <thread/ThreadEvent.h>
#include <thread/Thread.h>			//VOXOX - JRT - 2009.10.06 

CChatHandler::CChatHandler(ChatHandler & chatHandler, CUserProfile & cUserProfile)
	: _chatHandler(chatHandler),
	_cUserProfile(cUserProfile) {

	_pChatHandler = NULL;
	_waitMutexLocked = false;		//VOXOX - JRT - 2009.10.06 

	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::initPresentationThreadSafe, this));
	PFactory::postEvent(event);
}

CChatHandler::~CChatHandler() {
	//VOXOX - JRT - 2009.04.13 - TODO: do _pChatHander get deleted?
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&PChatHandler::deletePresentation, _pChatHandler));
	PFactory::postEvent(event);
}

void CChatHandler::initPresentationThreadSafe() 
{
	_pChatHandler = PFactory::getFactory().createPresentationChatHandler(*this);
	_chatHandler.newIMChatSessionCreatedEvent		 += boost::bind(&CChatHandler::newIMChatSessionCreatedEventHandler, this, _1, _2);

	_chatHandler.newSendMessageEvent		 += boost::bind(&CChatHandler::newSendMessageEventHandler, this, _1, _2,_3);//VOXOX - CJC - 2009.10.14 
	//VOXOX - CJC - 2009.05.07 Voxox change newIMChat to email session event
	_chatHandler.newIMChatToEmailSessionCreatedEvent += boost::bind(&CChatHandler::newIMChatToEmailSessionCreatedEventHandler, this, _1, _2, _3);

	//VOXOX - CJC - 2009.05.19 Chat to SMS
	_chatHandler.newIMChatToSMSSessionCreatedEvent += boost::bind(&CChatHandler::newIMChatToSMSSessionCreatedEventHandler, this, _1, _2, _3);
	//VOXOX CHANGE CJC SUPPORT OTHER OBJECTS ONCHAT
	_chatHandler.newSendFileCreatedEvent		+= boost::bind(&CChatHandler::newSendFileCreatedEventHandler, this, _1, _2);
	_chatHandler.initChatCreatedEvent			+= boost::bind(&CChatHandler::newInitChatCreatedEventHandler, this, _1);			//VOXOX - CJC - 2009.06.03 
	_chatHandler.newSendFaxCreatedEvent			+= boost::bind(&CChatHandler::newSendFaxCreatedEventHandler,  this, _1, _2);		//VOXOX - CJC - 2009.06.23 
//	_chatHandler.newContactProfileCreatedEvent	+= boost::bind(&CChatHandler::newContactProfileCreatedEventHandler, this, _1, _2);	//VOXOX - JRT - 2009.06.30 
	_chatHandler.incomingGroupChatInviteEvent	+= boost::bind(&CChatHandler::incomingGroupChatInviteEventHandler,  this, _1, _2);	//VOXOX - JRT - 2009.07.10 
}

Presentation * CChatHandler::getPresentation() const {
	return _pChatHandler;
}

CWengoPhone & CChatHandler::getCWengoPhone() const {
	return _cUserProfile.getCWengoPhone();
}

CUserProfile & CChatHandler::getCUserProfile() const {
	return _cUserProfile;
}

std::string CChatHandler::getTranslationLanguageXML(){

	return getCUserProfile().getTranslationLanguageXML();
}
//-----------------------------------------------------------------------------

void CChatHandler::incomingGroupChatInviteEventHandler(ChatHandler& sender, GroupChatInfo& gcInfo) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	gcInfo.respondToInviteEvent += boost::bind(&CChatHandler::respondToInviteEventHandler, this, _1 );

	//Transfers to UI thread.
	typedef ThreadEvent1<void (GroupChatInfo&), GroupChatInfo&> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::incomingGroupChatInviteEventHandlerThreadSafe, this, _1), gcInfo);
	PFactory::postEvent(event);

	waitForInviteResponse();
}

void CChatHandler::incomingGroupChatInviteEventHandlerThreadSafe( const GroupChatInfo& gcInfo )		//VOXOX - JRT - 2009.10.06 
{
	//Wait for the main thread to lock waitMutex
	while ( !_waitMutexLocked )	//VOXOX - JRT - 2009.10.06 
	{
		Thread::msleep( 1 );
	}

	//Should now be in UI thread.
	LOG_DEBUG("new incomingGroupChatInviteEvent");
	if (_pChatHandler) 
	{
		_pChatHandler->incomingGroupChatInviteEvent( const_cast<GroupChatInfo&>(gcInfo) );
	}
}

void CChatHandler::waitForInviteResponse()
{
	_waitMutex.lock();
	_waitMutexLocked = true;	//VOXOX - JRT - 2009.10.06 
	_waitGroupChatInvite.wait(&_waitMutex);
	_waitMutex.unlock();			//VOXOX - JRT - 2009.10.06 
}

void CChatHandler::respondToInviteEventHandler( const GroupChatInfo& gcInfo )
{
	_waitGroupChatInvite.wakeAll();
	_waitMutexLocked = false;		//VOXOX - JRT - 2009.10.06 
}

//-----------------------------------------------------------------------------

void CChatHandler::newIMChatSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession) {
	//VOXOX - JRT - 2009.07.10 - Using a new threads causes 'joined' messages to be lost for group chat.
	//							 Not using new thread won't allow new widget to be created.
	typedef ThreadEvent1<void (IMChatSession &), IMChatSession &> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newIMChatSessionCreatedEventHandlerThreadSafe, this, _1), imChatSession);
	PFactory::postEvent(event);
}

void CChatHandler::newIMChatSessionCreatedEventHandlerThreadSafe(IMChatSession & imChatSession) {
	LOG_DEBUG("new IMChatSessionCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newIMChatSessionCreatedEvent(imChatSession);
	}
}

void CChatHandler::newSendMessageEventHandler(ChatHandler & sender, IMChatSession & imChatSession,std::string & message){//VOXOX - CJC - 2009.10.14 

	typedef ThreadEvent2<void (IMChatSession &, std::string message), IMChatSession &, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newSendMessageEventHandlerThreadSafe, this, _1, _2), imChatSession, message);
	PFactory::postEvent(event);
}

void CChatHandler::newSendMessageEventHandlerThreadSafe(IMChatSession & imChatSession,std::string & message) {//VOXOX - CJC - 2009.10.14 
	LOG_DEBUG("new IMChatSessionCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newSendMessageCreatedEvent(imChatSession,message);
	}
}
//VOXOX - CJC - 2009.05.07 Add support for chat to email events
void CChatHandler::newIMChatToEmailSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession, std::string & relatedContactId) {
	typedef ThreadEvent2<void (IMChatSession &, std::string relatedContactId), IMChatSession &, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newIMChatToEmailSessionCreatedEventHandlerThreadSafe, this, _1, _2), imChatSession, relatedContactId);
	PFactory::postEvent(event);
}

//VOXOX - CJC - 2009.05.19 Chat to sms
void CChatHandler::newIMChatToSMSSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession, std::string & relatedContactId) {
	typedef ThreadEvent2<void (IMChatSession &, std::string relatedContactId), IMChatSession &, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newIMChatToSMSSessionCreatedEventHandlerThreadSafe, this, _1, _2), imChatSession, relatedContactId);
	PFactory::postEvent(event);
}

//VOXOX - CJC - 2009.05.07 Chat to email session 
void CChatHandler::newIMChatToEmailSessionCreatedEventHandlerThreadSafe(IMChatSession & imChatSession , std::string & relatedContactId) {
	LOG_DEBUG("new IMChatToEmailSessionCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newIMChatToEmailSessionCreatedEvent(imChatSession,relatedContactId);
	}
}
//VOXOX - CJC - 2009.05.19 Chat to sms
void CChatHandler::newIMChatToSMSSessionCreatedEventHandlerThreadSafe(IMChatSession & imChatSession , std::string & relatedContactId) {
	LOG_DEBUG("new IMChatToSMSSessionCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newIMChatToSMSSessionCreatedEvent(imChatSession,relatedContactId);
	}
}



//CJC VOXOX CHANGE SUPPORT SEND FILE
void CChatHandler::newSendFileCreatedEventHandler(ChatHandler & sender, IMContact & imContact){
	typedef ThreadEvent1<void (IMContact & imContact), IMContact &> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newSendFileCreatedEventHandlerThreadSafe, this, _1), imContact);
	PFactory::postEvent(event);
}

void CChatHandler::newSendFileCreatedEventHandlerThreadSafe(IMContact & imContact) {
	LOG_DEBUG("new SendFileCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newSendFileCreatedEvent(imContact);
	}
}


//VOXOX - CJC - 2009.06.23 
void CChatHandler::newSendFaxCreatedEventHandler(ChatHandler & sender, std::string & faxNumber){
	typedef ThreadEvent1<void (std::string relatedContactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newSendFaxCreatedEventHandlerThreadSafe, this, _1), faxNumber);
	PFactory::postEvent(event);
}

void CChatHandler::newSendFaxCreatedEventHandlerThreadSafe(std::string & faxNumber) {
	LOG_DEBUG("new SendFaxCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newSendFaxCreatedEvent(faxNumber);
	}
}


//CJC VOXOX CHANGE SUPPORT SEND FILE
void CChatHandler::newInitChatCreatedEventHandler(ChatHandler & sender){
	typedef ThreadEvent0 <void()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newInitChatCreatedEventHandlerThreadSafe, this));
	PFactory::postEvent(event);
}

void CChatHandler::newInitChatCreatedEventHandlerThreadSafe() {
	LOG_DEBUG("new InitChatCreatedEvent");
	if (_pChatHandler) {
		_pChatHandler->newInitChatCreatedEvent();
	}
}


//CJC VOXOX CHANGE SUPPORT SEND FILE
//VOXOX - JRT - 2009.06.30 - Handled in separate window.
//void CChatHandler::newContactProfileCreatedEventHandler(ChatHandler & sender, IMContact & imContact){
//	typedef ThreadEvent1<void (IMContact & imContact), IMContact &> MyThreadEvent;
//	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CChatHandler::newContactProfileCreatedEventHandlerThreadSafe, this, _1), imContact);
//	PFactory::postEvent(event);
//}

//void CChatHandler::newContactProfileCreatedEventHandlerThreadSafe(IMContact & imContact) {
//	LOG_DEBUG("new ContactProfileCreatedEvent");
//	if (_pChatHandler) {
//		_pChatHandler->newContactProfileCreatedEvent(imContact);
//	}
//}


void CChatHandler::createSession(const std::string & imAccountId, const IMContactSet & imContactSet, 
								 IMChat::IMChatType imChatType, const std::string& userChatRoomName) 
{
	_chatHandler.createSession(imAccountId, imContactSet, imChatType, userChatRoomName );
}
//-----------------------------------------------------------------------------
//VOXOX - JRT - OK - 2010.01.06 
CContactList& CChatHandler::getCContactList()
{
	return getCUserProfile().getCContactList();
}
MessageTranslation CChatHandler::getMessageTranslationSettings(const std::string & key){//VOXOX - CJC - 2010.01.22 

	return getCContactList().getMessageTranslationSettings(key);
}

void CChatHandler::setMessageTranslationSettings(const std::string & key, const MessageTranslation & translation ){//VOXOX - CJC - 2010.01.22 

	getCContactList().addMessageTranslationSettings(key,translation);

}
