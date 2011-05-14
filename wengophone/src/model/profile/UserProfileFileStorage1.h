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

#ifndef OWUSERPROFILEFILESTORAGE1_H
#define OWUSERPROFILEFILESTORAGE1_H

#include "UserProfileStorage.h"

#include <thread/RecursiveMutex.h>
#include <util/Event.h>

/**
 * Saves the UserProfile object, its IMAccountHandler and its ContactList.
 * If the save is successful, a backup is made to recover the profile in case
 * of problem while saving it.
 *
 * @ingroup model
 * @author Philippe Bernery
 */
class UserProfileFileStorage1 : public UserProfileStorage {
public:

	/**
	 * Gets the full path to the profile named 'profileName'.
	 */
	static std::string getProfilePath(const std::string & profileName);

	UserProfileFileStorage1(UserProfile & userProfile);

	virtual ~UserProfileFileStorage1();

	/**
	 * Will load the profile named 'profileName'.
	 * 
	 * If an error occured while trying the profile,
	 * the method will try to load the profile from 
	 * the backup directory.
	 * If it works a profileLoadedFromBackupsEvent
	 * will be emitted.
	 * If the backup cannot be loaded, it will be erased if it 
	 * exists and a profileCannotBeLoadedEvent will be emitted.
	 */  
	virtual bool load(const std::string & profileName);

	virtual bool save(const std::string & profileName);

private:

	/**
	 * Gets the temporary save profile dir.
	 */
	static std::string getTempProfilePath(const std::string & profileName);

	/**
	 * Gets the old profile save dir.
	 */
	static std::string getOldProfilePath(const std::string & profileName);

	/**
	 * Gets the full path to the backup profile named 'profileName'.
	 */
	static std::string getBackupProfilePath(const std::string & profileName);

	/**
	 * Tries to load a profile from the 'profiles' dir.
	 * 
	 * @return false if error, true otherwise
	 */  
	bool loadFromProfiles(const std::string & profileName);

	/**
	 * Tries to load a profile from the 'backups' dir.
	 * 
	 * @return false if error, true otherwise
	 */  
	bool loadFromBackups(const std::string & profileName);

	/**
	 * Loads a profile from a dir.
	 * 
	 * @return false if error, true otherwise
	 */
	bool loadFromDir(const std::string & path);

	/**
	 * Loads the UserProfile object from url/userprofile.xml
	 * 
	 * @return false if error, true otherwise
	 */
	bool loadProfile(const std::string & url);

	/**
	 * Saves the UserProfile object in url/userprofile.xml
	 * 
	 * @return false if error, true otherwise
	 */
	bool saveProfile(const std::string & url);

	/** For thread-safe operations. */
	RecursiveMutex _mutex;

};

#endif	//OWUSERPROFILEFILESTORAGE1_H
