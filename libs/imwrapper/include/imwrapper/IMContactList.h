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

#ifndef OWIMCONTACTLIST_H
#define OWIMCONTACTLIST_H

#include "IMAccount.h"

#include <util/Event.h>
#include <util/Interface.h>
#include <util/Trackable.h>

#include <string>

class ContactGroup;	//VOXOX - JRT - 2009.05.07 JRT-GRPS

/**
 * FIXME: Currently an IMContact can be in only one group (because of Gaim limitation)
 */
class IMContactList : Interface, public Trackable {
public:

	//VOXOX - JRT - 2009.04.11 
	/**
	 * Emitted when an IMContact has been added remotely.
	 *
	 * @param sender this class
	 * @param nOnOff - started/stopped
	 */
	Event< void (IMContactList & sender, int nOnOff ) > setInitialLoadEvent;
	//End VoxOx

	/**
	 * Emitted when an IMContact has been added remotely.
	 *
	 * @param sender this class
	 * @param contactId the new contact
	 */
	Event< void (IMContactList & sender, std::string groupName, std::string contactId) > newContactAddedEvent;

	/**
	 * Emitted when an IMContact has been removed remotely.
	 *
	 * @param sender this class
	 * @param contactId the removed contact
	 */
	Event< void (IMContactList & sender, std::string groupName, std::string contactId) > contactRemovedEvent;

	/**
	 * Emitted when an IMContact has been moved remotely.
	 *
	 * @param sender this class
	 * @param contactId the moved contact
	 */
	Event< void (IMContactList & sender, std::string groupName, std::string contactId) > contactMovedEvent;

	/**
	 * Emitted when an IMContact has been added remotely.
	 *
	 * @param sender this class
	 * @param groupName the added group
	 */
	Event< void (IMContactList & sender, std::string groupName) > newContactGroupAddedEvent;	//Retain old version for interface to LibPurple.
	Event< void (IMContactList & sender, ContactGroup& group )  > newContactGroupAdded2Event;	//VOXOX - JRT - 2009.05.07 JRT-GRPS

	/**
	 * Emitted when a group has been removed.
	 *
	 * @param sender this class
	 * @param groupName the removed group
	 */
	Event< void (IMContactList & sender, std::string groupName) > contactGroupRemovedEvent;

	virtual ~IMContactList() {};

	/**
	 * Remotely adds a contact.
	 *
	 * If the group does not exist, it is automatically created.
	 * @param groupName the group name to add the contact in
	 * @param contactId the contact to add
	 */
	virtual void addContact(const std::string & groupName, const std::string & contactId) = 0;

	/**
	 * Remotely deletes a contact.
	 *
	 * @param groupName the group name of the contact
	 * @param contactId the contact to remove
	 */
	virtual void removeContact(const std::string & groupName, const std::string & contactId) = 0;

	/**
	 * Remotely adds a group.
	 *
	 * @param groupName the group name to add the contact in
	 */
	virtual void addGroup(const std::string & groupName) = 0;

	/**
	 * Remotely deletes a group.
	 *
	 * @param groupName the group name of the contact
	 */
	virtual void removeGroup(const std::string & groupName) = 0;

	/**
	 * Moves a contact to another group.
	 *
	 * @param newGroupName the new group name
	 * @param oldGroupName the old group name
	 * @param contactId the contact to move
	 */
	virtual void moveContactToGroup(const std::string & newGroupName,
		const std::string & oldGroupName, const std::string & contactId) = 0;

	/**
	 * Changes group name.
	 *
	 * @param oldGroupName the old group name
	 * @param newGroupName the new group name
	 */
	virtual void changeGroupName(const std::string & oldGroupName, const std::string & newGroupName) = 0;

//	virtual void getContactVcard( const std::string & contactId) = 0;	//VOXOX - JRT - 2009.05.19 - Hold off

	void setIMAccount(const IMAccount & imAccount) {
		_imAccount = imAccount;
	}

	/**
	 * @return the associated IMAccount.
	 */
	std::string getIMAccountId() const {
//		return _imAccount.getUUID();
		return const_cast<IMAccount&>(_imAccount).getKey();	//VOXOX - JRT - 2009.04.24
	}

protected:

	IMContactList(IMAccount imAccount)
		: _imAccount(imAccount) {}

	IMAccount _imAccount;
};

#endif	//OWIMCONTACTLIST_H
