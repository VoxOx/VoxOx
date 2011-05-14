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

#ifndef OWCHATHANDLER_H
#define OWCHATHANDLER_H

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMChat.h>		//VOXOX - JRT - 2009.06.15 

#include <thread/RecursiveMutex.h>

#include <util/Event.h>
#include <util/NonCopyable.h>
#include <util/Trackable.h>

#include <map>

class IMAccountManager;
class IMChat;
class IMChatSession;
class IMContactSet;
class IMContact;
class UserProfile;

/**
 * Allows to create an instant messaging chat session.
 *
 * @see IMChatSession
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class ChatHandler : NonCopyable, public Trackable {
public:

	ChatHandler(UserProfile & userProfile);

	~ChatHandler();

	/**
	 * Emitted when a new IMChatSession has been created.
	 *
	 * @param sender this class
	 * @param imChatSession the new IMChatSession
	 */
	Event<void (ChatHandler & sender, IMChatSession & imChatSession)> newIMChatSessionCreatedEvent;

	Event<void (ChatHandler & sender, IMChatSession & imChatSession,std::string & message)> newSendMessageEvent;//VOXOX - CJC - 2009.10.14 
	//VOXOX - CJC - 2009.05.19 Chat to email
	Event<void (ChatHandler & sender, IMChatSession & imChatSession,std::string & relatedContactId)> newIMChatToEmailSessionCreatedEvent;
	//VOXOX - CJC - 2009.05.19 Chat to sms
	Event<void (ChatHandler & sender, IMChatSession & imChatSession,std::string & relatedContactId)> newIMChatToSMSSessionCreatedEvent;

	//VOXOX CHANGE CJC CREATE SEND FILE
	Event<void (ChatHandler & sender, IMContact & imContact)> newSendFileCreatedEvent;
	//VOXOX CHANGE CJC CREATE CONTACT PROFILE
	Event<void (ChatHandler & sender, IMContact & imContact)> newContactProfileCreatedEvent;
	//VOXOX - CJC - 2009.05.27 
	Event<void (ChatHandler & sender)> initChatCreatedEvent;

	//VOXOX - CJC - 2009.06.23 
	Event<void (ChatHandler & sender, std::string & faxNumber)> newSendFaxCreatedEvent;

	Event<void (ChatHandler & sender, GroupChatInfo& gcInfo  )> incomingGroupChatInviteEvent;	//VOXOX - JRT - 2009.07.10 

	/**
	 * Creates a new IMChatSession.
	 * The newChatSessionCreatedEvent is emitted when an IMChatSession is created
	 *
	 * @param imAccount the Id of the IMAccount for which we want to create the new IMChatSession
	 */
	void createSession				(const std::string & imAccountId, const IMContactSet & imContactSet, IMChat::IMChatType imChatType, const std::string userChatRoomName );	//VOXOX - JRT - 2009.06.15 
	void createChatToEmailSession	(const std::string & imAccountId, const IMContactSet & imContactSet,const std::string & relatedContactId);
	void createChatToSMSSession		(const std::string & imAccountId, const IMContactSet & imContactSet,const std::string & relatedContactId);
	void createSessionAndSendMessage(const std::string & imAccountId, const IMContactSet & imContactSet,const std::string & message);

	//TODO: MOVE THIS FROM HERE, RIGHT NOW THE ONLY WAY OF GETTING TO LIBPURRPLE WITH CHAT HANDLER
	void callSkypeContact(const std::string & imAccountId, const IMContactSet & imContactSet);

	void createSendFile(const std::string & imAccountId, IMContact & imContact);
	void createSendFax (std::string & faxNumber);

	//VOXOX CHANGE CJC CONTACT PROFILE ON CHAT
	void createContactProfile(const std::string & imAccountId, IMContact & imContact);
	
	void initChat();	//VOXOX - CJC - 2009.06.03 

	/**
	 * Tells ChatHandler to unregister to all registered Events.
	 */
	void unregisterAllEvents();

private:

	/**
	 * @see IMChat::newIMChatSessionCreatedEvent
	 */
	void newIMChatSessionCreatedEventHandler			  (IMChat & sender, IMChatSession & imChatSession);
	void newIMChatToEmailSessionCreatedEventHandler		  (IMChat & sender, IMChatSession & imChatSession);
	void newIMChatToSMSSessionCreatedEventHandler		  (IMChat & sender, IMChatSession & imChatSession);
	void newIMChatAndSendMessageSessionCreatedEventHandler(IMChat & sender, IMChatSession & imChatSession, const std::string & message,bool killChatSession);

	/**
	 * @see IMAccountManager::imAccountAddedEvent
	 */
	void imAccountAddedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMAccountManager::imAccountRemovedEvent
	 */
	void imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMChatSession::imChatSessionWillDieEvent
	 */
	void imChatSessionWillDieEventHandler(IMChatSession & sender);

	void incomingGroupChatInviteEventHandler( IMChat& sender, GroupChatInfo& gcInfo) ;	//VOXOX - JRT - 2009.07.10 

	IMChat*	findIMChatByAccount( const std::string& accountId );	//VOXOX - JRT - 2009.07.10 

	typedef std::map<std::string, IMChat *> IMChatMap;
	IMChatMap _imChatMap;

	typedef std::list<IMChatSession *> IMChatSessionList;
	IMChatSessionList _imChatSessionList;

	UserProfile & _userProfile;

	mutable RecursiveMutex _mutex;
};

#endif	//OWCHATHANDLER_H
