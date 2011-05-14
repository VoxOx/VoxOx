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

#ifndef PHAPIIMPRESENCE_H
#define PHAPIIMPRESENCE_H

#include "PhApiFactory.h"

#include <imwrapper/IMPresence.h>
#include <set>

class IMAccount;

/**
 * PhApi Instant Messaging presence.
 *
 * @ingroup model
 * @author Mathieu Stute
 */
class PhApiIMPresence : public IMPresence {
	friend class PhApiFactory;
public:

	void changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note = String::null);

	void changeMyAlias(const std::string & nickname);

	void changeMyIcon(const OWPicture & picture);

	void subscribeToPresenceOf(const std::string & contactId);

	void unsubscribeToPresenceOf(const std::string & contactId);

	void blockContact(const std::string & contactId);

	void unblockContact(const std::string & contactId);

	void authorizeContact(const std::string & contactId, bool auhorized, const std::string message);

	void reRequestAuthorization(const std::string & contactId);//VOXOX - CJC - 2009.07.31 


	void acceptSubscription(int sid);
	void rejectSubscription(int sid);

private:

	PhApiIMPresence(IMAccount & account, PhApiWrapper & phApiWrapper);

	~PhApiIMPresence();

	void presenceStateChangedEventHandler(PhApiWrapper & sender, EnumPresenceState::PresenceState state, const std::string & note, const std::string & from);

	void myPresenceStatusEventHandler(PhApiWrapper & sender, EnumPresenceState::MyPresenceStatus status, const std::string & note);

	void subscribeStatusEventHandler(PhApiWrapper & sender, const std::string & contactId, IMPresence::SubscribeStatus status);

	void contactIconChangedEventHandler(PhApiWrapper & sender, const std::string & contactId, const std::string & filename);

	void incomingSubscribeEventHandler(SipWrapper & sender, int sid, const std::string& from, const std::string& evtType);

	const std::string getRessourcePath();

	/** Current icon filename. */
	std::string _iconFilename;

	PhApiWrapper & _phApiWrapper;
};

#endif	//PHAPIIMPRESENCE_H
