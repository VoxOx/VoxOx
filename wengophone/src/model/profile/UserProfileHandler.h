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

#ifndef OWUSERPROFILEHANDLER_H
#define OWUSERPROFILEHANDLER_H

#include <model/account/SipAccount.h>

#include <thread/RecursiveMutex.h>
#include <thread/Timer.h>
#include <util/Event.h>
#include <util/StringList.h>
#include <util/Trackable.h>

class Profile;
class UserProfile;
class UserProfileStorage;
class WengoAccount;
class SipAccount;
class WengoPhone;

/**
 * Manages the UserProfiles.
 *
 * @author Philippe Bernery
 */
class UserProfileHandler : public Trackable 
{
public:

	/**
	 * No current UserProfile has been set. This can happen at the first launch
	 * of the softphone when no UserProfile has been set.
	 *
	 * @param sender this class
	 */
	Event<void (UserProfileHandler & sender)> noCurrentUserProfileSetEvent;

	/**
	 * Emitted when a UserProfile will be destroyed.
	 *
	 * After the event, the old UserProfile reference will be invalidated.
	 *
	 * @param sender this class
	 */
	Event<void (UserProfileHandler & sender)> currentUserProfileWillDieEvent;

	/**
	 * A UserProfile has been set.
	 *
	 * @param sender this class
	 * @param userProfile the set UserProfile
	 */
	Event<void (UserProfileHandler & sender, UserProfile & userProfile)> userProfileInitializedEvent;

	/**
	 * Emitted when the requested SipAccount connection failed.
	 *
	 * @param account SipAccount
	 * @param state Error type
	 */
	Event<void (SipAccount & sipAccount, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage)> sipAccountConnectionFailedEvent;//VOXOX CHANGE by Rolando - 2009.09.10 

	/**
	 * @see UserProfile::networkDiscoveryStateChangedEvent
	 */
	Event< void (SipAccount & sender, SipAccount::NetworkDiscoveryState state) > networkDiscoveryStateChangedEvent;

	/**
	 * @see UserProfileStorage::profileLoadedFromBackupsEvent
	 */
	Event < void (UserProfileStorage &sender, std::string profileName) > profileLoadedFromBackupsEvent;

	/**
	 * @see UserProfileStorage::profileCannotBeLoadedEvent
	 */
	Event < void (UserProfileStorage &sender, std::string profileName) > profileCannotBeLoadedEvent;

	/**
	 * Gets the names of existing user profiles.
	 *
	 * These UserProfiles can then be loaded with getUserProfile.
	 *
	 * @param onlyRealDir if <code>false</code> can return directoru=y ending with ".new" or ".old"
	 *
	 * @return vector of UserProfiles
	 */
	static StringList getUserProfileNames(bool onlyRealDir = false);

	UserProfileHandler();

	~UserProfileHandler();

	/**
	 * Initializes the UserPofileHandler.
	 *
	 * During initialization, the UserProfileHandler will check for the last
	 * used UserProfile. If no UserProfile has been used, the
	 * noCurrentUserProfileSetEvent will be emitted.
	 */
	void init();

	/**
	 * Sets the current UserProfile.
	 *
	 * A non empty SipAccount can be given in parameter. It will be used
	 * to update password and remember Password attributes of the UserProfile.
	 *
	 * @param name the name of the current UserProfile to set
	 * @param wengoAccount the WengoAccount that will update the UserProfile
	 */
	//VOXOX CHANGE by Rolando - 2009.05.22 - added parameter "loginInvisible" to be able to login with a invisible presence
	void setCurrentUserProfile(const std::string & name, const SipAccount & sipAccount, bool loginInvisible = false);

	/**
	 * @return the current userProfile
	 */
	UserProfile * getCurrentUserProfile() const {
		return _currentUserProfile;
	}

	/**
	 * Check if a UserProfile exists.
	 *
	 * @param name name of the UserProfile to check
	 * @return true if a UserProfile with the given name exists
	 */
	bool userProfileExists(const std::string & name);

	/**
	 * Must be called when the current UserProfile is released and can be
	 * destroyed.
	 */
	void currentUserProfileReleased();

	/**
	 * Gets a UserProfile.
	 *
	 * Used by model and control. Must not be used by presentation
	 *
	 * @param name the name of the desired UserProfile
	 * @return the UserProfile or NULL if no UserProfile with the given found
	 */
	UserProfile * getUserProfile(const std::string & name);
	
	/**
	 * Loads an UserProfile
	 *
	 * @param userprofile the UserProfile that is to be loaded
	 * @param name the name of the desired UserProfile
	 * @return <code>true</code> if the UserProfile have been correctly loadded
	 */
	bool LoadUserProfileFromName(UserProfile * userprofile, const std::string & name);

	/**
	 * Enable/disable autosave.
	 * @param enable if True autosave if activated.
	 */
	void enableAutoSave(bool enable) {
		_autoSave = enable;
	}

	void saveCurrentUserProfile();		//VOXOX - JRT - 2009.12.11 


private:
	void initEventHandlers();		//VOXOX - JRT - 2009.10.07 
	void uninitEventHandlers();

	/**
	 * Initializes the current UserProfile.
	 */
	void initializeCurrentUserProfile();

	/**
	 * Sets the last used UserProfile.
	 *
	 * @param the last used UserProfile
	 */
	void setLastUsedUserProfile(const UserProfile & userProfile);

	/**
	 * Saves the UserProfile.
	 *
	 * @param userProfile the UserProfile to save
	 */
	void saveUserProfile(UserProfile & userProfile);

	/*
	 * @see Profile::profileChangedEvent
	 */
	void profileChangedEventHandler();

	/**
	 * @see Timer::lastTimeoutEvent
	 */
	void saveTimerLastTimeoutEventHandler(Timer & sender);

	/**
	 * @see UserProfile::sipAccountValidityEvent
	 */
	void sipAccountConnectedEventHandler();
	void sipAccountConnectedEventHandlerThreadSafe();
	void sipAccountConnectionFailedEventHandler(EnumSipLoginState::SipLoginState, std::string loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
	void sipAccountConnectionFailedEventHandlerThreadSafe(EnumSipLoginState::SipLoginState, std::string loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 


	void userProfileInitializedEventHandler();
	void userProfileInitializedEventHandlerThreadSafe();

	/**
	 * Remove temporary profiles (.new directories)
	 */
	void cleanupUserProfileDirectories();

	/**
	 * Initializes external libraries (PhApi and Gaim).
	 */
	void initExtLibraries(const std::string & pathToProfile);

	/**
	 * Uninitializes external libraries (PhApi and Gaim).
	 */
	void uninitExtLibraries();

	RecursiveMutex _mutex;

	UserProfile * _currentUserProfile;

	UserProfile * _desiredUserProfile;

	/** True if _saveTimer is running. */
	bool _saveTimerRunning;

	/** True if autosave is enabled (default) */
	bool _autoSave;

	Timer _saveTimer;
};

#endif //OWUSERPROFILEHANDLER_H
