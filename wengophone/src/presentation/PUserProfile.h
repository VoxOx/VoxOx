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

#ifndef OWPUSERPROFILE_H
#define OWPUSERPROFILE_H

#include "Presentation.h"

#include <model/account/SipAccount.h>

#include <string>

class IMContact;
class PresenceHandler;

/**
 * Presentation layer of UserProfile.
 *
 * @ingroup presentation
 * @author Philippe Bernery
 */
class PUserProfile : public Presentation {
public:

	/**
	 * @see UserProfile::loginStateChangedEvent
	 */
	virtual void loginStateChangedEventHandler(SipAccount & sender,
		EnumSipLoginState::SipLoginState state) = 0;

	/**
	 * @see UserProfile::networkDiscoveryStateChangedEvent
	 */
	virtual void networkDiscoveryStateChangedEventHandler(SipAccount & sender,
		SipAccount::NetworkDiscoveryState state) = 0;

	/**
	 * @see PresenceHandler::authorizationRequestEvent
	 */
	virtual void authorizationRequestEventHandler(PresenceHandler & sender,
		const IMContact & imContact, const std::string & message) = 0;

	/**
	 * @see PresenceHandler::incomingSubscribeEvent
	 */
	virtual void incomingSubscribeEventHandler(PresenceHandler & sender,
		const std::string & imaccountId,int sid,const std::string & from,const std::string & evtType) = 0;
};

#endif	//OWPUSERPROFILE_H
