/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWCUSERPROFILEHANDLER_H
#define OWCUSERPROFILEHANDLER_H

#include <model/account/wengo/WengoAccount.h>
#include <model/profile/SoftUpdatePrefs.h>	//VOXOX - JRT - 2009.12.10 

#include <thread/Mutex.h>

#include <util/Trackable.h>

#include <string>

class CUserProfile;
class CWengoPhone;
class PUserProfileHandler;
class UserProfile;
class UserProfileHandler;
class UserProfileStorage;

class SoftUpdateInfo;	//VOXOX - JRT - 2009.10.30 

/**
 * Control of UserProfileHandler.
 *
 * @author Philippe Bernery
 */
class CUserProfileHandler : public Trackable {
public:

	CUserProfileHandler(UserProfileHandler & userProfileHandler, CWengoPhone & cWengoPhone);

	~CUserProfileHandler();

	/**
	 * @see UserProfileHandler::getUserProfileNames
	 */
	std::vector<std::string> getUserProfileNames(bool onlyRealDir = false);

	/**
	 * @return true if the UserProfile named 'name' exists
	 */
	bool userProfileExists(const std::string & name) const;

	/**
	 * @see UserProfileHandler::setUserProfile
	 */
	void setCurrentUserProfile(const std::string & profileName, const SipAccount & sipAccount, bool loginInvisible = false);

	/**
	 * Convenience method to log the user off
	 */
	void logOff( bool quitting );	//VOXOX - JRT - 2009.07.13 - So we can use same code path for logoff and exit.

	/**
	 * @see UserProfileHandler::currentUserProfileReleased
	 */
	void currentUserProfileReleased();

	/**
	 * Return a copy of the SipAccount of the UserProfile named 'name'.
	 *
	 * @param name the UserProfile name
	 * @return the SipAccount of the UserProfile named 'name'
	 */
	SipAccount getSipAccountOfUserProfile(const std::string & name);

	/**
	 * @return the current CUserProfile.
	 * Can be NULL.
	 */
	CUserProfile * getCUserProfile() const 
	{ 
		Mutex::ScopedLock scopedLock(_mutex);
		return _cUserProfile;
	}

	/**
	 * @return the Presentation layer.
	 */
	PUserProfileHandler * getPresentation() const { return _pUserProfileHandler; }

	/**
	 * @return the Model layer
	 */
	UserProfileHandler & getUserProfileHandler() const { return _userProfileHandler; }

	void updateUserProfile();		//VOXOX - JRT - 2009.08.03 

	void ignoreVersion( const SoftUpdateInfo& suInfo );	//VOXOX - JRT - 2009.10.30 
	SoftUpdatePrefs& getSoftUpdatePrefs();			//VOXOX - JRT - 2009.12.10 
	void			 saveSoftUpdatePrefs();			//VOXOX - JRT - 2009.12.10 

private:
	/**
	 * @see UserProfileHandler::noCurrentUserProfileSetEvent
	 */
	void noCurrentUserProfileSetEventHandler(UserProfileHandler & sender);

	/**
	 * @see UserProfileHandler::currentUserProfileWillDieEvent
	 */
	void currentUserProfileWillDieEventHandlerThreadSafe();
	void currentUserProfileWillDieEventHandler(UserProfileHandler & sender);

	/**
	 * @see UserProfileHandler::userProfileInitializedEvent
	 */
	void userProfileInitializedEventHandler(UserProfileHandler & sender, UserProfile & userProfile);

	/**
	 * @see UserProfileHandler::sipAccountConnectionFailedEvent
	 */
	void sipAccountConnectionFailedEventHandler(SipAccount & sipAccount, EnumSipLoginState::SipLoginState, std::string loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 

	/**
	 * @see UserProfileStorage::profileLoadedFromBackupsEvent
	 */
	void profileLoadedFromBackupsEventHandler(UserProfileStorage &sender, std::string profileName);

	/**
	 * @see UserProfileStorage::profileCannotBeLoadedEvent
	 */
	void profileCannotBeLoadedEventHandler(UserProfileStorage &sender, std::string profileName);

	/**
	 * @see setCurrentUserProfile
	 */
	void setUserProfileWithSipAccountThreadSafe(std::string profileName, SipAccount sipAccount);

	/**
	 * @see setCurrentUserProfile
	 */
	void setUserProfileWithWengoAccountThreadSafe(std::string profileName, WengoAccount wengoAccount, bool loginInvisible);

	/**
	 * @see currentUserProfileReleased
	 */
	void currentUserProfileReleasedThreadSafe();
	
	/**
	 * @see UserProfileHandler::networkDiscoveryStateChangedEvent
	 */
	void networkDiscoveryStateChangedEventHandler(SipAccount & sender, SipAccount::NetworkDiscoveryState state);

private:
	CUserProfile*			_cUserProfile;
	PUserProfileHandler*	_pUserProfileHandler;
	UserProfileHandler&		_userProfileHandler;
	CWengoPhone&			_cWengoPhone;

	SoftUpdatePrefs			_defSoftUpdatePrefs;	//VOXOX - JRT - 2009.12.10 	


	mutable Mutex _mutex;
};

#endif //OWCUSERPROFILEHANDLER_H
