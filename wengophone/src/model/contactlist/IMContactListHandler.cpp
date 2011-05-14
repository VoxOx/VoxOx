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
#include "IMContactListHandler.h"

#include <model/profile/UserProfile.h>
#include <model/contactlist/ContactGroup.h>	//VOXOX - JRT - 2009.05.07 JRT-GRPS
#include <imwrapper/IMContactList.h>
#include <imwrapper/IMWrapperFactory.h>

#define LOGGER_COMPONENT "ContactList"
#include <util/Logger.h>
#include <util/SafeDelete.h>

IMContactListHandler::IMContactListHandler(UserProfile & userProfile) 
	: _userProfile(userProfile) 
{
	userProfile.getIMAccountManager().imAccountAddedEvent   += boost::bind(&IMContactListHandler::imAccountAddedEventHandler,   this, _1, _2);
	userProfile.getIMAccountManager().imAccountRemovedEvent += boost::bind(&IMContactListHandler::imAccountRemovedEventHandler, this, _1, _2);
}

IMContactListHandler::~IMContactListHandler() 
{
}

void IMContactListHandler::addIMContact(const std::string & groupName, const IMContact & imContact) 
{
	//VOXOX - JRT - 2009.09.21 - We have two CRs here that indicate imContact::imAccount is NULL.  They both occurred
	//							 for same user on same date.  Let's just keep an eye on it.
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(*(const_cast<IMContact&>(imContact).getIMAccount()));
	if (imAccount) {
		IMContactListMap::iterator it = _imContactListMap.find(imAccount->getKey());

		if (it != _imContactListMap.end()) 
		{
			(*it).second->addContact(groupName, imContact.getContactId());
		} 
		else 
		{
			LOG_DEBUG("this IMAccount is not registered: " + imAccount->getLogin());
		}
		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::removeIMContact(const std::string & groupName, const IMContact & imContact) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount( *(const_cast<IMContact&>(imContact).getIMAccount()) );
	if (imAccount) 
	{
		IMContactListMap::iterator it = _imContactListMap.find(imAccount->getKey());

		if (it != _imContactListMap.end()) 
		{
			//VOXOX - JRT - 2009.08.09 - In libpurple, this just removes the IMContact from the Group.
			//							 If there are no more groups for this IMContact, then the contact is removed from buddy list.
			(*it).second->removeContact(groupName, imContact.getContactId());
		} 
		else 
		{
			LOG_DEBUG("this IMAccount is not registered: " + imAccount->getLogin());
		}
		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::setInitialLoadEventHandler( IMContactList & sender, int nOnOff )
{
	setInitialLoadEvent( *this, nOnOff );
}

void IMContactListHandler::newContactAddedEventHandler(IMContactList & sender,
	std::string groupName, std::string contactId) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(sender.getIMAccountId());

	if (imAccount) 
	{
		IMContact imContact(*imAccount, contactId);
		IMContact* pAcct = _imContactList.Add( imContact );

		if ( pAcct )
		{
			newIMContactAddedEvent(*this, groupName, *pAcct);
		}

		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::contactRemovedEventHandler(IMContactList & sender, std::string groupName, std::string contactId) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(sender.getIMAccountId());
	if (imAccount) 
	{
		IMContact imContact(*imAccount, contactId);
		IMContact* pContact = _imContactList.findByIMContact( imContact );

		if ( pContact ) 
		{
			imContactRemovedEvent(*this, groupName, *pContact );
			_imContactList.Delete( *pContact );
		} 
		else 
		{
			LOG_ERROR("contact not found");
		}
		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::imAccountAddedEventHandler(IMAccountManager & sender, std::string imAccountId) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);

	if (imAccount) 
	{
		IMContactListMap::iterator it = _imContactListMap.find(imAccountId);

		if (it == _imContactListMap.end()) 
		{
			IMContactList * imContactList = IMWrapperFactory::getFactory().createIMContactList(*imAccount);
			if (imContactList) 
			{
				imContactList->newContactAddedEvent		 += boost::bind(&IMContactListHandler::newContactAddedEventHandler,		 this, _1, _2, _3);
				imContactList->contactRemovedEvent		 += boost::bind(&IMContactListHandler::contactRemovedEventHandler,		 this, _1, _2, _3);
				imContactList->contactMovedEvent		 += boost::bind(&IMContactListHandler::contactMovedEventHandler,		 this, _1, _2, _3);
				imContactList->newContactGroupAddedEvent += boost::bind(&IMContactListHandler::newContactGroupAddedEventHandler, this, _1, _2);
				imContactList->contactGroupRemovedEvent  += boost::bind(&IMContactListHandler::contactGroupRemovedEventHandler,  this, _1, _2);

				imContactList->setInitialLoadEvent		 += boost::bind(&IMContactListHandler::setInitialLoadEventHandler,		 this, _1, _2);

				_imContactListMap.insert(std::pair<std::string, IMContactList *>(imAccountId, imContactList));
			} 
			else 
			{
				LOG_DEBUG("cannot create an IMContactList");
			}
		} 
		else 
		{
			LOG_ERROR("this IMAccount has already been added " + imAccount->getLogin());
		}
		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imAccountId);
	if (imAccount) 
	{
		IMContactListMap::iterator it = _imContactListMap.find(imAccount->getKey());
		if (it != _imContactListMap.end()) 
		{
			delete (*it).second;
			_imContactListMap.erase(it);
		} 
		else 
		{
			LOG_ERROR("this IMAccount has not been added " + imAccount->getLogin());
		}
		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::contactMovedEventHandler(IMContactList & sender, std::string groupName, std::string contactId) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(sender.getIMAccountId());
	
	if (imAccount) 
	{
		IMContact imContact(*imAccount, contactId);

		bool inList = _imContactList.contains( imContact );

		if ( inList )
		{
			LOG_DEBUG("an IMContact has moved:" + contactId + ".  New Group: " + groupName );
			imContactMovedEvent(*this, groupName, imContact);
		} 
		else 
		{
			LOG_DEBUG("this IMContact(" + contactId + ") from group " + groupName
				+ " has not been added yet. Adding it as new IMContact.");
			newContactAddedEventHandler(sender, groupName, contactId);
		}

		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::newContactGroupAddedEventHandler(IMContactList & sender, std::string groupName ) 
{
	ContactGroup contactGroup( groupName, EnumGroupType::GroupType_User );
	newContactGroupAdded2Event(sender, contactGroup);
//	newContactGroupAddedEvent(sender, groupName);
}

void IMContactListHandler::newContactGroupAdded2EventHandler(IMContactList & sender, ContactGroup& group ) 	//VOXOX - JRT - 2009.05.07 JRT-GRPS
{
	newContactGroupAdded2Event(sender, group);	//VOXOX - JRT - 2009.05.07 JRT-GRPS
}

void IMContactListHandler::contactGroupRemovedEventHandler(IMContactList & sender, std::string groupName) 
{
	contactGroupRemovedEvent(sender, groupName);
}

void IMContactListHandler::addGroup(const std::string & groupName) 
{
	for (IMContactListMap::const_iterator it = _imContactListMap.begin(); it != _imContactListMap.end(); it++) 
	{
		(*it).second->addGroup(groupName);
	}
}

void IMContactListHandler::removeGroup(const std::string & groupName) 
{
	for (IMContactListMap::const_iterator it = _imContactListMap.begin(); it != _imContactListMap.end(); it++) 
	{
		(*it).second->removeGroup(groupName);
	}
}

void IMContactListHandler::moveContactToGroup(const std::string & newGroupName, const std::string & oldGroupName, const IMContact & imContact) 
{
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imContact.getIMAccountId());

	if (imAccount) 
	{
		IMContactListMap::iterator it = _imContactListMap.find(imAccount->getKey());
		if (it != _imContactListMap.end()) 
		{
			//Update servers via PurpleIMContactList::moveContactToGroup()
			(*it).second->moveContactToGroup(newGroupName, oldGroupName, imContact.getContactId());
		} 
		else 
		{
			LOG_DEBUG("this IMAccount is not registered: "  + imAccount->getLogin());
		}
		OWSAFE_DELETE(imAccount);
	}
}

void IMContactListHandler::changeGroupName(const std::string & oldGroupName, const std::string & newGroupName) 
{
	for (IMContactListMap::const_iterator it = _imContactListMap.begin(); it != _imContactListMap.end(); it++) 
	{
		(*it).second->changeGroupName(oldGroupName, newGroupName);
	}
}

//VOXOX - JRT - 2009.05.19 - Hold off for now
//void IMContactListHandler::getContactVcard( const IMContact& imContact ) 
//{
//	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(*(const_cast<IMContact&>(imContact).getIMAccount()));
//	if (imAccount) 
//	{
//		IMContactListMap::iterator it = _imContactListMap.find(imAccount->getKey());
//
//		if (it != _imContactListMap.end()) 
//		{
//			(*it).second->getContactVcard( imContact.getContactId());
//		} 
//		else 
//		{
//			LOG_DEBUG("this IMAccount is not registered: " + imAccount->getLogin());
//		}
//		OWSAFE_DELETE(imAccount);
//	}
//}
//End VoxOx


void IMContactListHandler::unregisterAllEvents() 
{
	for (IMContactListMap::const_iterator it = _imContactListMap.begin(); it != _imContactListMap.end(); it++) 
	{
		(*it).second->newContactAddedEvent		-= boost::bind(&IMContactListHandler::newContactAddedEventHandler,		this, _1, _2, _3);
		(*it).second->contactRemovedEvent		-= boost::bind(&IMContactListHandler::contactRemovedEventHandler,		this, _1, _2, _3);
		(*it).second->contactMovedEvent			-= boost::bind(&IMContactListHandler::contactMovedEventHandler,			this, _1, _2, _3);
		(*it).second->newContactGroupAddedEvent -= boost::bind(&IMContactListHandler::newContactGroupAddedEventHandler, this, _1, _2);
		(*it).second->contactGroupRemovedEvent	-= boost::bind(&IMContactListHandler::contactGroupRemovedEventHandler,	this, _1, _2);

		(*it).second->setInitialLoadEvent		-= boost::bind(&IMContactListHandler::setInitialLoadEventHandler,		this, _1, _2);
	}
}
