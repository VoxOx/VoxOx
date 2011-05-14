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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include <coipmanager/Session.h>

#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactList.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>
#include <imwrapper/IMContactSet.h>

Session::Session(UserProfile & userProfile) 
	: _userProfile(userProfile) {
}

Session::Session(const Session & session) 
	: CoIpModule(),
	_userProfile(session._userProfile) {
	_contactIdList = session._contactIdList;
}

Session::~Session() {
}

void Session::addContact(const std::string & contactId) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	_contactIdList.push_back(contactId);
}

void Session::removeContact(const std::string & contactId) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	for (StringList::iterator it = _contactIdList.begin();
		it != _contactIdList.end();
		++it) {
		if ((*it) == contactId) {
			_contactIdList.erase(it);
		}
	}
}

void Session::setContactList(const StringList & contactList) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	_contactIdList = contactList;
}

IMAccount Session::findFirstValidAccount(const std::string & contactId) const {
//	Contact * contact = _userProfile.getContactList().getContact(contactId);
	Contact * contact = _userProfile.getContactList().getContactByUuid(contactId);	//VOXOX - JRT - 2009.04.20 - Use renamed method.
	if (contact) {
		//Contact workingContact(*contact);
		//IMAccountList imAccountList = _userProfile.getIMAccountManager().getIMAccountListCopy();
		//for (IMAccountList::const_iterator upIt = imAccountList.begin();
		//	upIt != imAccountList.end();
		//	++upIt) {
		//	for (IMContactSet::const_iterator imIt = workingContact.getIMContactSet().begin();
		//		imIt != workingContact.getIMContactSet().end();
		//		++imIt) {
		//		if ((*upIt).getUUID() == (*imIt).getIMAccountId()) {
		//			return (*upIt);
		//		}
		//	}
		//}

		IMAccountList& rList = const_cast<IMAccountList&>(_userProfile.getIMAccountManager().getIMAccountListConst()); 
		IMAccount*     pAcct = rList.findFirstValidAccount( contact->getIMContactSet() );

		if ( pAcct )
		{
			return *pAcct;
		}
	}

	return IMAccount();
}

Account * Session::findCommonAccount(const StringList & contactIdList) const {
	return NULL;
}

IMContactSet Session::getLinkedIMContacts(const Account & account) const {
	IMContactSet result;

/*	for (StringList::const_iterator it = _contactIdList.begin();
		it != _contactIdList.end();
		++it) {
		Contact * myContact = _userProfile.getContactList().getContact(*it);
		const IMContact * imContact = myContact->getFirstValidIMContact(account);
		if (imContact) {
			result.insert(*imContact);
		}
	}
*/
	return result;
}
