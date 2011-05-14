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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "UserProfileFileStorage1.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/contactlist/ContactListFileStorage1.h>
#include <model/profile/UserProfileXMLSerializer.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccountListFileStorage.h>

#include <util/File.h>
#include <util/Logger.h>

using namespace std;

static const std::string USERPROFILE_FILENAME = "userprofile.xml";
static const std::string PROFILES_DIR = "profiles/";
static const std::string BACKUPS_DIR = "backups/";

UserProfileFileStorage1::UserProfileFileStorage1(UserProfile & userProfile)
	: UserProfileStorage(userProfile) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	File::createPath(File::convertPathSeparators(config.getConfigDir() + BACKUPS_DIR));
}

UserProfileFileStorage1::~UserProfileFileStorage1() {
}

std::string UserProfileFileStorage1::getProfilePath(const std::string & profileName) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return File::convertPathSeparators(config.getConfigDir() + PROFILES_DIR + profileName + "/");
}

std::string UserProfileFileStorage1::getTempProfilePath(const std::string & profileName) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return File::convertPathSeparators(config.getConfigDir() + PROFILES_DIR + profileName + ".new/");
}

std::string UserProfileFileStorage1::getOldProfilePath(const std::string & profileName) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return File::convertPathSeparators(config.getConfigDir() + PROFILES_DIR + profileName + ".old/");
}

std::string UserProfileFileStorage1::getBackupProfilePath(const std::string & profileName) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return File::convertPathSeparators(config.getConfigDir() + BACKUPS_DIR + profileName + "/");
}

bool UserProfileFileStorage1::load(const std::string & profileName) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (profileName.empty()) {
		LOG_DEBUG("empty profileName given");
		return false;
	}

	bool result = false;

	if (!loadFromProfiles(profileName)) {
		LOG_ERROR("cannot load profile " + profileName + " from 'profiles'."
			" Trying from 'backups'.");
		File profileDir(getProfilePath(profileName));
		profileDir.remove();
		if (!loadFromBackups(profileName)) {
			LOG_ERROR("cannot load profile " + profileName + " from 'backups'.");
			profileCannotBeLoadedEvent(*this, profileName);
			File backupDir(getBackupProfilePath(profileName));
			backupDir.remove();
		} else {
			LOG_INFO("backup recovery successful");
			save(profileName);
			profileLoadedFromBackupsEvent(*this, profileName);
			result = true;
		}
	} else {
		LOG_DEBUG("profile " + profileName + " loaded successfully");
		result = true;
	}
	
	return result;
}

bool UserProfileFileStorage1::loadFromProfiles(const std::string & profileName) {
	return loadFromDir(getProfilePath(profileName));
}

bool UserProfileFileStorage1::loadFromBackups(const std::string & profileName) {
	return loadFromDir(getBackupProfilePath(profileName));
}

bool UserProfileFileStorage1::loadFromDir(const std::string & path) {
	if (!loadProfile(path)) {
		LOG_ERROR("cannot loadProfile: " + path);
		return false;
	}

	IMAccountListFileStorage imAccountListFileStorage(_userProfile.getIMAccountManager()._imAccountList);
	if (!imAccountListFileStorage.load(path)) {
		LOG_ERROR("cannot load IMAccountList: " + path);
		return false;
	}

	IMAccountList imAccountList = _userProfile.getIMAccountManager().getIMAccountListCopy();
	ContactListFileStorage1 contactListFileStorage1(*_userProfile._contactList, imAccountList);
	if (!contactListFileStorage1.load(path)) {
		LOG_ERROR("cannot load ContactList: " + path);
		return false;
	}

	if (!_userProfile.loadHistory(path)) {
		return false;
	}

	return true;
}

bool UserProfileFileStorage1::save(const std::string & profileName) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (profileName.empty()) {
		LOG_DEBUG("empty profileName given");
		return false;
	}

	std::string path = getProfilePath(profileName);
	std::string newPath = getTempProfilePath(profileName);
	std::string oldPath = getOldProfilePath(profileName);

	// Backuping the last saved profile
	if (File::exists(path)) {
		// Copying the profile in a .old folder
		File curProfDir(path);
		curProfDir.copy(oldPath);

		// Moving the copy in backups/
		File oldProfFile(oldPath);
		oldProfFile.move(getBackupProfilePath(profileName), true);
	}
	////

	// Removing a possible .new dir in profiles/
	if (File::exists(newPath)) {
		File newProfDir(newPath);
		newProfDir.remove();
	}
	////

	// Saving profile to .new dir
	File::createPath(newPath);

	if (!saveProfile(newPath)) {
		return false;
	}

	IMAccountList imAccountList = _userProfile.getIMAccountManager().getIMAccountListCopy();

	IMAccountListFileStorage imAccountListFileStorage(imAccountList);
	if (!imAccountListFileStorage.save(newPath)) {
		return false;	
	}

	ContactListFileStorage1 contactListFileStorage1(*_userProfile._contactList, imAccountList);
	if (!contactListFileStorage1.save(newPath)) {
		return false;
	}

	if (!_userProfile.saveHistory(newPath)) {
		return false;
	}
	////

	// If successful, move the 'dir.new' to 'dir' (overwrite it if exists) 
	File newProfDir(newPath);
	newProfDir.move(path, true);
	////
	
	return true;
}

bool UserProfileFileStorage1::loadProfile(const std::string & url) {
	bool result = false;

	FileReader file(url + USERPROFILE_FILENAME);
	if (file.open()) {
		string data = file.read();

		UserProfileXMLSerializer serializer(_userProfile);
		result = serializer.unserialize(data);
	}

	return result;
}

bool UserProfileFileStorage1::saveProfile(const std::string & url) {
	FileWriter file(url + USERPROFILE_FILENAME);
	UserProfileXMLSerializer serializer(_userProfile);

	file.write(serializer.serialize());
	return true;
}
