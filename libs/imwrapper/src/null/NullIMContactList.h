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

#ifndef NULLIMCONTACTLIST_H
#define NULLIMCONTACTLIST_H

#include "NullIMFactory.h"

#include <imwrapper/IMContactList.h>

class IMAccount;

/**
 * Null IM contact list (stub).
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class NullIMContactList : public IMContactList {
	friend class NullIMFactory;
public:

	void addContact(const std::string & groupName, const std::string & contactId);

	void removeContact(const std::string & groupName, const std::string & contactId);

	void addGroup(const std::string & groupName);

	void removeGroup(const std::string & groupName);

	void moveContactToGroup(const std::string & newGroupName,
		const std::string & oldGroupName, const std::string & contactId);

	void changeGroupName(const std::string & oldGroupName, const std::string & newGroupName);

private:

	NullIMContactList(IMAccount & account);
};

#endif	//NULLIMCONTACTLIST_H
