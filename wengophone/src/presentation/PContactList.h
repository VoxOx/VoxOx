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

#ifndef OWPCONTACTLIST_H
#define OWPCONTACTLIST_H

#include "Presentation.h"

#include <string>

/**
 *
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 */
class PContactList : public Presentation {
public:

	/**
	 * A toaster must be showed with a contact description inside.
	 *
	 * FIXME not finished yet
	 *
	 * @param contactId contact UUID to show
	 */
	//Event<void (const std::string & contactId)> showToaster;

	//VOXOX - JRT - 2009.04.11 
	virtual void setInitialLoadEvent(const int nOnOff ) = 0;
	//EndVoxOx

	/**
	 * Called by the control when a group has been added.
	 *
	 * @param contactGroupId the contact group UUID
	 */
	virtual void contactGroupAddedEvent(const std::string & groupId) = 0;

	/**
	 * Called by the control when a group has been removed.
	 *
	 * @param contactGroupId the contact group UUID
	 */
	virtual void contactGroupRemovedEvent(const std::string & groupId) = 0;

	/**
	 * Called by the control when a group has been renamed.
	 *
	 * @param contactGroupId the contact group UUID
	 */
	virtual void contactGroupRenamedEvent(const std::string & groupId) = 0;

	virtual void contactGroupsReorderedEvent() = 0;		//VOXOX - JRT - 2009.05.11 

	/**
	 * Called by the control when a contact has been added.
	 *
	 * @param contactId the contact UUID
	 */
//	virtual void contactAddedEvent(const std::string & contactId) = 0;
	virtual void contactAddedEvent(const std::string & contactId, const std::string& groupId) = 0;	//VOXOX - JRT - 2009.08.09 - Multiple group support

	/**
	 * Called by the control when a contact has been removed.
	 *
	 * @param contactId the contact UUID
	 */
	virtual void contactRemovedEvent(const std::string & contactId) = 0;

	/**
	 * Called by the control when a contact has moved.
	 *
	 * @param dstGroupId the contact group UUID
	 * @param srcGroupId the contact group UUID
	 * @param contactId the contact UUID
	 */
	virtual void contactMovedEvent(const std::string & dstGroupId, const std::string & srcGroupId, const std::string & contactId) = 0;

	/**
	 * Called by the control when a contact has changed.
	 *
	 * @param contactId the UUID of the changed Contact
	 */
//	virtual void contactChangedEvent(const std::string & contactId) = 0;
	virtual void contactChangedEvent(const std::string & contactId, const std::string& groupId) = 0;	//VOXOX - JRT - 2009.08.09 - Multiple group support.
};

#endif	//OWPCONTACTLIST_H
