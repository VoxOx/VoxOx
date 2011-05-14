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

#ifndef PUSERPROFILEHANDLER_H
#define PUSERPROFILEHANDLER_H

#include "Presentation.h"

#include <model/account/SipAccount.h>

#include <string>

class WengoAccount;

/**
 * Presentation layer of UserProfileHandler
 * @ingroup presentation
 * @author Philippe Bernery
 */
class PUserProfileHandler : public Presentation {
public:

	virtual void setCurrentUserProfileEventHandler() = 0;
	/**
	 * @see UserProfileHandler::noCurrentUserProfileSetEvent
	 */
	virtual void noCurrentUserProfileSetEventHandler() = 0;

	/**
	 * @see UserProfileHandler::currentUserProfileWillDieEvent
	 */
	virtual void currentUserProfileWillDieEventHandler() = 0;

	/**
	 * @see UserProfileHandler::userProfileInitializedEvent
	 */
	virtual void userProfileInitializedEventHandler() = 0;

	/**
	 * @see UserProfileHandler::sipAccountConnectionFailedEvent
	 */
	virtual void sipAccountConnectionFailedEventHandler(const SipAccount & sipAccount, EnumSipLoginState::SipLoginState, std::string loginErrorMessage) = 0;//VOXOX CHANGE by Rolando - 2009.09.10 

	/**
	 * @see UserProfileStorage::profileLoadedFromBackupsEvent
	 */
	virtual void profileLoadedFromBackupsEventHandler(std::string profileName) = 0;

	/**
	 * @see UserProfileStorage::profileCannotBeLoadedEvent
	 */
	virtual void profileCannotBeLoadedEventHandler(std::string profileName) = 0;

	/**
	 * @see CUserProfileHandler::networkDiscoveryStateChangedEventHandler
	 */
	virtual void networkErrorEventHandler(const SipAccount & sipAccount) = 0;
};

#endif	//PUSERPROFILEHANDLER_H
