/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef USERPROFILESTORAGE_H
#define USERPROFILESTORAGE_H

#include <serialization/Storage.h>

#include <util/Event.h>

class UserProfile;

/**
 * UserProfile storage.
 *
 * @ingroup model
 * @author Philippe Bernery
 */
class UserProfileStorage : public Storage {
public:

	/**
	 * Emitted when loading the profile failed and getting the backup
	 * worked.
	 */
	Event < void (UserProfileStorage & sender, std::string profileName) > profileLoadedFromBackupsEvent;

	/**
	 * Emitted when loading the profile failed with both normal and backup way.
	 */
	Event < void (UserProfileStorage & sender, std::string profileName) > profileCannotBeLoadedEvent;
	
	UserProfileStorage(UserProfile & userProfile)
		: _userProfile(userProfile) { }

	virtual ~UserProfileStorage() { }

protected:

	UserProfile & _userProfile;
};

#endif	//USERPROFILESTORAGE_H
