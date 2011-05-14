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

#ifndef PRESENCE_H
#define PRESENCE_H

#include <imwrapper/IMPresence.h>

#include <util/NonCopyable.h>
#include <util/Event.h>
#include <util/Trackable.h>

class ContactList;
class MyPresenceState;

/**
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class Presence : NonCopyable, public Trackable {
public:

	/**
	 * @see IMPresence::presenceStateChangedEvent
	 */
	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	Event<void (IMPresence & sender, EnumPresenceState::PresenceState state,
		std::string alias, std::string statusMessage, std::string from)> presenceStateChangedEvent;

	/**
	 * @see IMPresence::presenceStatusEvent
	 */
	Event<void (IMPresence & sender, EnumPresenceState::MyPresenceStatus status,
		std::string note)> myPresenceStatusEvent;

	/**
	 * @see IMPresence::subscribeStatusEvent
	 */
	Event<void (IMPresence & sender, std::string contactId,
		IMPresence::SubscribeStatus status)> subscribeStatusEvent;

	/**
	 * @see IMPresence::subscribeStatusEvent
	 */
	Event<void (IMPresence & sender, std::string contactId,
		std::string message)> authorizationRequestEvent;

	/**
	 * Ask for authorization to see MyPresenceState.
	 */
	Event<void (IMPresence & sender,  int sid, const std::string& from, const std::string& evtType)> incomingSubscribeEvent;

	/**
	 * @see IMPresence::contactIconChangedEvent
	 */
//	Event< void (IMPresence & sender, std::string contactId, OWPicture icon) > contactIconChangedEvent;
	Event< void (IMPresence & sender, std::string contactId, const OWPicture& icon) > contactIconChangedEvent;	//VOXOX - JRT - 2009.08.18 


	Presence(IMAccount imAccount);

	~Presence();

	/**
	 * @see IMPresence::changeMyPresence
	 */
	void changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note);

	/**
	 * @see IMPresence::changeMyAlias
	 */
	void changeMyAlias(const std::string & alias);

	/**
	 * @see IMPresence::changeMyIcon
	 */
	void changeMyIcon(const OWPicture & picture);

	/**
	 * @see IMPresence::subscribeToPresenceOf
	 */
	void subscribeToPresenceOf(const std::string & contactId);

	/**
	 * @see IMPresence::unsubscribeToPresenceOf
	 */
	void unsubscribeToPresenceOf(const std::string & contactId);

	/**
	 * @see IMPresence::blockContact
	 */
	void blockContact(const std::string & contactId);

	/**
	 * @see IMPresence::unblockContact
	 */
	void unblockContact(const std::string & contactId);

	/**
	 * @see IMPresence::authorizeContact
	 */
	void authorizeContact(const std::string & contactId, bool authorized, const std::string message);

	void reRequestAuthorization(const std::string & contactId);//VOXOX - CJC - 2009.07.31 

	void acceptSubscription(int sid);
	void rejectSubscription(int sid);

	std::string getIMAccountId() const {
//		return _imAccount.getUUID();
		return const_cast<IMAccount&>(_imAccount).getKey();	//VOXOX - JRT - 2009.04.24
	}

	/**
	 * Tells Presence to unregister to all registered Events.
	 */
	void unregisterAllEvents();

private:

	/**
	 * Sets the state of this Presence
	 *
	 * @param the desired state
	 */
	void setState(EnumPresenceState::PresenceState state);

	/**
	 * Catches presenceStateChangedEvent for debug reason.
	 *
	 * Shows a LOG_DEBUG() message.
	 */
	void presenceStateChangedEventHandler(IMPresence & sender, EnumPresenceState::PresenceState state,
		const std::string & alias,const std::string & statusMessage, const std::string & from);

	/**
	 * @see IMPresence::presenceStatusEvent
	 */
	void myPresenceStatusEventHandler(IMPresence & sender, EnumPresenceState::MyPresenceStatus status,
		std::string note);

	/**
	 * @see IMPresence::subscribeStatusEvent
	 */
	void subscribeStatusEventHandler(IMPresence & sender, std::string contactId,
		IMPresence::SubscribeStatus status);

	/**
	 * @see IMPresence::subscribeStatusEvent
	 */
	void authorizationRequestEventHandler(IMPresence & sender, std::string contactId,
		std::string message);

	/**
	 * Ask for authorization to see MyPresenceState.
	 */
	void incomingSubscribeEventHandler(IMPresence & sender,  int sid, const std::string& from, const std::string& evtType);

	/**
	 * @see IMPresence::contactIconChangedEvent
	 */
//	void contactIconChangedEventHandler(IMPresence & sender, std::string contactId, OWPicture icon);
	void contactIconChangedEventHandler(IMPresence & sender, std::string contactId, const OWPicture& icon);	//VOXOX - JRT - 2009.08.18 

	IMAccount _imAccount;

	IMPresence * _imPresence;

	MyPresenceState * _state;
};

#endif	//PRESENCE_H
