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

#ifndef GAIMIMCONTACTLIST_H
#define GAIMIMCONTACTLIST_H

#include <string>

#include <imwrapper/IMContactList.h>
#include <thread/Mutex.h>

class GaimIMContactList : public IMContactList
{
	friend class GaimIMFactory;

public:

	GaimIMContactList(IMAccount imAccount);

	virtual void addContact(const std::string & groupName, const std::string & contactId);
	virtual void removeContact(const std::string & groupName, const std::string & contactId);
	virtual void addGroup(const std::string & groupName);
	virtual void removeGroup(const std::string & groupName);
	virtual void changeGroupName(const std::string & oldGroupName, const std::string & newGroupName);
	virtual void moveContactToGroup(const std::string & newGroupName,
									const std::string & oldGroupName, 
									const std::string & contactId);

	const IMAccount & getIMAccount() const { return _imAccount; }

	static bool addContactCbk(void * data);
	static bool removeContactCbk(void * data);
	static bool addGroupCbk(void * data);
	static bool removeGroupCbk(void * data);
	static bool changeGroupNameCbk(void * data);
	static bool moveContactToGroupCbk(void * data);

	static Mutex _mutex;
};

#endif //GAIMIMCONTACTLIST_H
