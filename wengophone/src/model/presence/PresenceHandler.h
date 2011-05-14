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

#ifndef PRESENCEHANDLER_H
#define PRESENCEHANDLER_H

#include <model/chat/ChatHandler.h>

#include "../MyIMContactList.h"		//VOXOX - JRT - 2009.04.10 
#include <imwrapper/IMAccount.h>
#include <imwrapper/IMConnect.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/EnumPresenceState.h>
#include <imwrapper/IMPresence.h>

#include <util/Event.h>
#include <util/NonCopyable.h>
#include <util/Trackable.h>

#include <list>
#include <map>

class Connect;
class ConnectHandler;
class ContactList;
class IMAccountManager;
class IMContact;
class Presence;
class Thread;
class UserProfile;

/**
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class PresenceHandler : NonCopyable, public Trackable {
public:

	/**
	 * @see IMPresence::presenceStateChangedEvent
	 * @param imContact the IMContact for wich the state has changed
	 */
	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	Event<void (PresenceHandler & sender, EnumPresenceState::PresenceState state,
		const std::string & alias,const std::string & statusMessage, const IMContact & imContact)> presenceStateChangedEvent;

	/**
	 * @see IMPresence::presenceStatusEvent
	 */
	Event<void (PresenceHandler & sender, std::string imAccountId,
		EnumPresenceState::MyPresenceStatus status)> myPresenceStatusEvent;

	/**
	 * @see IMPresence::subscribeStatusEvent
	 */
	Event<void (PresenceHandler & sender, const IMContact & imContact,
		IMPresence::SubscribeStatus status)> subscribeStatusEvent;

	/**
	 * @see IMPresence::authorizationRequestEvent
	 */
	Event<void (PresenceHandler & sender, const IMContact & imContact,
		const std::string & message)> authorizationRequestEvent;

	/**
	 * @see IMPresence::incomingSubscribeEvent
	 */
	Event<void (PresenceHandler & sender,const std::string & imaccountId, int sid,const std::string & from,const std::string & evtType)> incomingSubscribeEvent;

	/**
	 * @see IMPresence::contactIconChangedEvent
	 */
//	Event< void (PresenceHandler & sender, const IMContact & imContact, OWPicture icon) > contactIconChangedEvent;
	Event< void (PresenceHandler & sender, const IMContact & imContact, const OWPicture& icon) > contactIconChangedEvent;	//VOXOX - JRT - 2009.08.18 


	PresenceHandler(UserProfile & userProfile);

	~PresenceHandler();

	/**
	 * Changes my presence on desired protocol.
	 *
	 * @param state my state
	 * @param note used by PresenceStateUserDefined
	 * @param imAccount the IMAccount that we want the presence changed. If NULL
	 * presence state of all accounts is changed.
	 */
//	void changeMyPresenceState(EnumPresenceState::PresenceState state, const std::string& note, IMAccount* imAccount = NULL);
	void changeMyPresenceState(EnumPresenceState::PresenceState state, const std::string& note, IMAccount* imAccount, bool save);

	/**
	 * Changes my alias.
	 *
	 * @param alias the desired alias
	 * @param imAccount the IMAccount that we want the alias changed. If NULL
	 * alias of all accounts is changed.
	 */
	void changeMyAlias(const std::string & alias, IMAccount * imAccount = NULL);
	//VOXOX CHANGE CJC CHANGE MY STATUS MESSAGE
	void changeMyStatusMessage(const std::string & statusMessage,IMAccount * imAccount = NULL);

	/**
	 * Changes my icon.
	 *
	 * @param picture the desired icon
	 * @param imAccount the IMAccount that we want the icon changed. If NULL
	 * icon of all accounts is changed.
	 */
	void changeMyIcon(const OWPicture & picture, IMAccount * imAccount = NULL);

	/**
	 * Subscribes to the presence of an IMContact.
	 *
	 * @param imContact the IMContact
	 */
	void subscribeToPresenceOf(const IMContact & imContact);

	/**
	 * Unsubscribes to the presence of an IMContact.
	 *
	 * @param imContact the IMContact
	 */
	void unsubscribeToPresenceOf(const IMContact & imContact);

	/**
	 * @see IMPresence::blockContact
	 */
	void blockContact(const IMContact & imContact);

	/**
	 * @see IMPresence::unblockContact
	 */
	void unblockContact(const IMContact & imContact);

	/**
	 * @see IMPresence::authorizeContact
	 */
	void authorizeContact(const IMContact & imContact, bool authorized,
		const std::string & message);

	void reRequestAuthorization(const IMContact & imContact);//VOXOX - CJC - 2009.07.31 

	void acceptSubscription(int sid,std::string imaccountId);
	void rejectSubscription(int sid,std::string imaccountId);

	/**
	 * Tells PresenceHandler to unregister to all registered Events.
	 */
	void unregisterAllEvents();

private:

	/**
	 * @see ConnectHandler::connectedEvent
	 */
	void connectedEventHandler(ConnectHandler & sender, std::string imAccountId);

	/**
	 * @see ConnectHandler::disconnectedEvent
	 */
	void disconnectedEventHandler(ConnectHandler & sender, std::string imAccountId);

	/**
	 * @see IMPresence::presenceStateChangedEvent
	 */
	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	void presenceStateChangedEventHandler(IMPresence & sender,
		EnumPresenceState::PresenceState state, std::string note, std::string statusMessage, std::string from);

	/**
	 * @see presenceStateChangedEventHandler
	 */
	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	void presenceStateChangedEventHandlerThreadSafe(std::string imAccountId, 
		EnumPresenceState::PresenceState state, std::string note, std::string statusMessage, std::string from);

	/**
	 * @see IMPresence::myPresenceStatusEvent
	 */
	void myPresenceStatusEventHandler(IMPresence & sender,
		EnumPresenceState::MyPresenceStatus status);

	/**
	 * @see myPresenceStatusEventHandler
	 */
	void myPresenceStatusEventHandlerThreadSafe(std::string imAccountId,
		EnumPresenceState::MyPresenceStatus status);

	/**
	 * @see IMPresence::subscribeStatusEvent
	 */
	void subscribeStatusEventHandler(IMPresence & sender,
		std::string contactId, IMPresence::SubscribeStatus status);

	/**
	 * @see subscribeStatusEventHandler
	 */
	void subscribeStatusEventHandlerThreadSafe(std::string imAccountId,
		std::string contactId, IMPresence::SubscribeStatus status);

	/**
	 * @see IMPresence::authorizationRequestEvent
	 */
	void authorizationRequestEventHandler(IMPresence & sender,
		std::string contactId, std::string message);

	/**
	 * @see authorizationRequestEventHandler
	 */
	void authorizationRequestEventHandlerThreadSafe(std::string imAccountId,
		std::string contactId, std::string message);

	/**
	 * @see IMPresence::incomingSubscribeEventHandler
	 */
	void incomingSubscribeEventHandler(IMPresence & sender,
		 int sid, const std::string& from, const std::string& evtType);

	/**
	 * @see incomingSubscribeEventHandlerThreadSafe
	 */
	void incomingSubscribeEventHandlerThreadSafe(IMPresence & sender,int sid, std::string from, std::string evtType);

	/**
	 * @see IMAccountManager::imAccountAddedEvent
	 */
	void imAccountAddedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMAccountManager::imAccountRemovedEvent
	 */
	void imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMPresence::contactIconChangedEvent
	 */
//	void contactIconChangedEventHandler(IMPresence & sender, std::string contactId, OWPicture icon);
	void contactIconChangedEventHandler(IMPresence & sender, std::string contactId, const OWPicture& icon);	//VOXOX - JRT - 2009.08.18 

	/**
	 * Sets the presence state of associated Contacts to unknown.
	 */
	void setPresenceStateToUnknown(Presence * presence);

	typedef std::map<std::string, Presence *> PresenceMap;

	PresenceMap _presenceMap;

//	typedef std::list<IMContact> MyIMContactList;	//VOXOX - JRT - 2009.04.10 - Use new IMContactList class
	/**
	 * List of IMContact waiting for presence subscriptions. This
	 * can happend when someone tries to subscribe to a contact presence
	 * and if the associated IMAccount is not connected.
	 */
//	MyIMContactList _pendingSubscriptions;	//VOXOX - JRT - 2009.04.10 - Use new ImContactList class
	MyIMContactList _pendingSubscriptions;

	/** List of Contact for which Presence State has been subsribed. */
//	MyIMContactList _subscribedContacts;	//VOXOX - JRT - 2009.04.10 - Use new ImContact class
	MyIMContactList _subscribedContacts;

	UserProfile & _userProfile;
};

#endif	//PRESENCEHANDLER_H
