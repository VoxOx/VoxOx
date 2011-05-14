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

#ifndef OWIMPRESENCE_H
#define OWIMPRESENCE_H

#include "IMAccount.h"

#include <util/Event.h>
#include <util/Interface.h>
#include <util/OWPicture.h>
#include <util/String.h>
#include <util/Trackable.h>

/**
 * Wrapper for Instant Messaging presence.
 *
 * @author Philippe Bernery
 * @author Tanguy Krotoff
 */
class IMPresence : Interface, public Trackable {
public:

	/**
	 * Presence status of a buddy has changed.
	 *
	 * @param sender this class
	 * @param state new buddy presence status
	 * @param alias buddy alias (Joe @ the beach)
	 * @param from buddy who changed its presence status
	 */
	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	Event<void (IMPresence & sender, EnumPresenceState::PresenceState state,
		std::string alias,std::string statusMessage, std::string from)> presenceStateChangedEvent;

	/*Event<void (IMPresence & sender, EnumPresenceState::PresenceState state,
		std::string alias, std::string from)> presenceStateChangedEvent;*/

	/**
	 * Status changed.
	 *
	 * @param sender this class
	 * @param status new status
	 * @param note used when PresenceStateUserDefined is used
	 */
	Event<void (IMPresence & sender, EnumPresenceState::MyPresenceStatus status,
		std::string note)> myPresenceStatusEvent;

	/**
	 * Emitted when my alias changed.
	 *
	 * @param sender this class
	 * @param nickname the new nickname
	 */
	//Event< void (IMPresence & sender, const std::string & nickname) > myNicknameChangedEvent;

	enum SubscribeStatus {
		/** Subscription to contact id has been successful */
		SubscribeStatusOk,

		/** An error occured while subscribing to contact presence */
		SubscribeStatusError
	};

	/**
	 * Subscribe status event.
	 *
	 * @param sender this class
	 * @param contactId the contact id
	 * @param status the SubscribeStatus
	 */
	Event<void (IMPresence & sender, std::string contactId, 
		SubscribeStatus status)> subscribeStatusEvent;

	/**
	 * Ask for authorization to see MyPresenceState.
	 *
	 * @param sender this class
	 * @param contactId the contact who asked for seeing my presence
	 * @param message a message sent by the contact to see my presence
	 */
	Event<void (IMPresence & sender, std::string contactId,
			std::string message)> authorizationRequestEvent;

	/**
	 * Ask for authorization to see MyPresenceState.
	 */
	Event<void (IMPresence & sender,  int sid, const std::string& from, const std::string& evtType)> incomingSubscribeEvent;

	/**
	 * Emitted when a contact has been blocked or unblocked.
	 *
	 * @param sender this class
	 * @param contactId the contact that has been changed
	 * @param blocked blocking state
	 */
	Event< void (IMPresence & sender, std::string contactId,
		bool blocked) > contactBlockingStateChangedEvent;

	/**
	 * Emitted when a contact changes his icon.
	 *
	 * @param sender this class
	 * @param contactId the contact that changes his icon
	 * @param icon the new icon
	 */
//	Event< void (IMPresence & sender, std::string contactId, OWPicture icon) > contactIconChangedEvent;
	Event< void (IMPresence & sender, std::string contactId, const OWPicture& icon) > contactIconChangedEvent;	//VOXOX - JRT - 2009.08.18 

	virtual ~IMPresence() { }

	/**
	 * Changes my presence status: online, offline, under the shower...
	 *
	 * @param state new presence state
	 * @param note personnalized status string ("I'm under the shower"),
	 *        used only with PresenceUserDefined
	 */
	virtual void changeMyPresence(EnumPresenceState::PresenceState state,
		const std::string & note = String::null) = 0;

	/**
	 * Changes my alias (e.g: 'Joe... at the beach!').
	 *
	 * @param alias the desired alias.
	 */
	virtual void changeMyAlias(const std::string & alias) = 0;

	/**
	 * Changes my icon.
	 *
	 * @param picture the desired icon
	 */
	virtual void changeMyIcon(const OWPicture & picture) = 0;

	/**
	 * Subscribes to the presence of a contact.
	 *
	 * @param contactId id of the contact
	 */
	virtual void subscribeToPresenceOf(const std::string & contactId) = 0;

	/**
	 * Unsubscribes to the presence of a contact.
	 *
	 * @param contactId id of the contact
	 */
	virtual void unsubscribeToPresenceOf(const std::string & contactId) = 0;

	/**
	 * Avoid a contact to see my Presence.
	 *
	 * @param contactId contact id
	 */
	virtual void blockContact(const std::string & contactId) = 0;

	/**
	 * Authorizes a contact to see my Presence.
	 *
	 * @param contactId contact id
	 */
	virtual void unblockContact(const std::string & contactId) = 0;

	/**
	 * Authorizes a contact to add me in his contact list.
	 *
	 * @param contactId contact id
 	 * @param authorized TRUE if we agree
	 */
	virtual void authorizeContact(const std::string & contactId, bool auhorized,
		const std::string message) = 0;


	virtual void reRequestAuthorization(const std::string & contactId) = 0;//VOXOX - CJC - 2009.07.31 

	virtual void acceptSubscription(int sid) { }
	virtual void rejectSubscription(int sid) { }

	void setIMAccount(const IMAccount & imAccount) {
		_imAccount = imAccount;
	}

	std::string getIMAccountId() const {
//		return _imAccount.getUUID();
		return const_cast<IMAccount&>(_imAccount).getKey();	//VOXOX - JRT - 2009.04.24
	}

protected:

	IMPresence(IMAccount account)
		: _imAccount(account) {}

	IMAccount _imAccount;
};

#endif	//OWIMPRESENCE_H
