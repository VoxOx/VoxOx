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

#ifndef OWSESSION_H
#define OWSESSION_H

#include <coipmanager/CoIpModule.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccount.h>
#include <imwrapper/IMContactSet.h>
#include <thread/RecursiveMutex.h>
#include <util/Event.h>
#include <util/StringList.h>

#include <string>

class UserProfile;

/**
 * Base class for all kind of Session.
 *
 * @author Philippe Bernery
 */
class Session : public CoIpModule {
public:

	Session(UserProfile & userProfile);

	Session(const Session & session);

	virtual ~Session();

	/**
	 * Adds a Contact to the Session.
	 *
	 * Thread safe.
	 */
	virtual void addContact(const std::string & contactId);

	/**
	 * Removes a Contact from the Session.
	 *
	 * Thread safe.
	 *
	 * No effect if contactId not found.
	 */
	virtual void removeContact(const std::string & contactId);

	/**
	 * Sets the list of Contact by giving a StringList.
	 *
	 * Thread safe.
	 *
	 * The list is copied.
	 */
	 virtual void setContactList(const StringList & contactList);

	 StringList getContactList() const {
		 return _contactIdList;
	 }

protected:

	/**
	 * Looks for the first Account that is connected and where the Contact 
	 * associated with contactId has a connected IMContact.
	 */
	IMAccount findFirstValidAccount(const std::string & contactId) const;

	/**
	 * Finds the first Account that is common to the UserProfile and
	 * Contacts given in parameter. 
	 * That is to say Contact must be linked to this Account.
	 *
	 * @return the found Account or NULL if no common Account found.
	 */
	Account * findCommonAccount(const StringList & contactIdList) const;

	/**
	 * Finds for each Contact the IMContact that is linked to the given
	 * Account.
	 */
	IMContactSet getLinkedIMContacts(const Account & account) const;

	/** Link to UserProfile. */
	UserProfile & _userProfile;

	/** Mutex for Thread safe operation. */
	RecursiveMutex _mutex;

	/** List of Contact Ids. */
	StringList _contactIdList;
};

#endif //OWSESSION_H
