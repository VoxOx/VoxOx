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

#include "PurpleIMContactList.h"
#include "PurpleEnumIMProtocol.h"

#include <util/Logger.h>
#include <thread/Thread.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern "C" 
{
#include <libpurple/blist.h>
}

Mutex PurpleIMContactList::_mutex;
Mutex PurpleIMContactList::_mutexContact;	//VOXOX - JRT - 2009.07.14 

PurpleIMContactList::PurpleIMContactList(IMAccount account) : IMContactList(account)
{
}

//-----------------------------------------------------------------------------
//Start Contact/Buddy related functions/methods
//-----------------------------------------------------------------------------

bool PurpleIMContactList::addContactCbk(void * dataIn)
{
//	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);
	Mutex::ScopedLock lock(PurpleIMContactList::_mutexContact);	//VOXOX - JRT - 2009.07.14 

	PurpleIMContactListCallbackData* cbData = (PurpleIMContactListCallbackData*)dataIn;

	PurpleAccount *gAccount = getPurpleAccount( cbData->getInstance() );

	PurpleGroup *gGroup = NULL;
	PurpleBuddy *gBuddy = NULL;  

	if ((gGroup = purple_find_group(cbData->getGroupName().c_str())) == NULL) 
	{
		gGroup = purple_group_new(cbData->getGroupName().c_str());
		purple_blist_add_group(gGroup, NULL);
	}

	gBuddy = purple_buddy_new(gAccount, cbData->getContactId().c_str(), cbData->getContactId().c_str() );
	purple_blist_add_buddy(gBuddy, NULL, gGroup, NULL);
	purple_account_add_buddy(gAccount, gBuddy);

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::addContact(const std::string & groupName, const std::string & contactId)
{
	PurpleIMContactListCallbackData* cbData = new PurpleIMContactListCallbackData;

	cbData->setInstance( this );
	cbData->setGroupName( groupName );
	cbData->setContactId( contactId );
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMContactList::addContactCbk ) );

//	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);
//	cbData->setCallbackId( purple_timeout_add(0, (GSourceFunc) PurpleIMContactList::addContactCbk, cbData) );

//	getContactVcard( contactId );		//VOXOX - JRT - 2009.05.19- Hold off for now.
}

//-----------------------------------------------------------------------------

bool PurpleIMContactList::removeContactCbk(void * dataIn)
{
//	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);
	Mutex::ScopedLock lock(PurpleIMContactList::_mutexContact);	//VOXOX - JRT - 2009.07.14 

	PurpleIMContactListCallbackData* cbData = (PurpleIMContactListCallbackData*)dataIn;
	PurpleAccount *gAccount = getPurpleAccount( cbData->getInstance() );

	PurpleGroup *gGroup = NULL;
	PurpleBuddy *gBuddy = NULL;

	if ( !cbData->getGroupName().empty() )
	{
		gGroup = purple_find_group( cbData->getGroupName().c_str() );
		gBuddy = purple_find_buddy_in_group(gAccount, cbData->getContactId().c_str(), gGroup );
	}
	else
	{
		gBuddy = purple_find_buddy(gAccount, cbData->getContactId().c_str() );
	}

	if (gBuddy) 
	{
		purple_account_remove_buddy(gAccount, gBuddy, gGroup);
		purple_blist_remove_buddy(gBuddy);
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::removeContact(const std::string & groupName, const std::string & contactId)
{
	PurpleIMContactListCallbackData* cbData = new PurpleIMContactListCallbackData;

	cbData->setInstance( this );

	cbData->setInstance( this );
	cbData->setGroupName( groupName );
	cbData->setContactId( contactId );
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMContactList::removeContactCbk ) );
}

//-----------------------------------------------------------------------------
//End Contact/Buddy related
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Group related functions/methods
//-----------------------------------------------------------------------------

bool PurpleIMContactList::addGroupCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);

	PurpleIMContactListCallbackData* cbData = (PurpleIMContactListCallbackData*)dataIn;

	PurpleGroup *gGroup = purple_find_group( cbData->getGroupName().c_str() );

	if (!gGroup) 
	{
		gGroup = purple_group_new( cbData->getGroupName().c_str() );
		purple_blist_add_group(gGroup, NULL);
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::addGroup(const std::string & groupName)
{
	PurpleIMContactListCallbackData* cbData = new PurpleIMContactListCallbackData;

	cbData->setInstance( this );
	cbData->setGroupName( groupName );
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMContactList::addGroupCbk ) );
}

//-----------------------------------------------------------------------------

bool PurpleIMContactList::removeGroupCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);

	PurpleIMContactListCallbackData* cbData = (PurpleIMContactListCallbackData*)dataIn;

	PurpleGroup *gGroup = purple_find_group( cbData->getGroupName().c_str() );

	if (gGroup) 
	{
		purple_blist_remove_group(gGroup);
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::removeGroup(const std::string & groupName)
{
	PurpleIMContactListCallbackData* cbData = new PurpleIMContactListCallbackData;

	cbData->setInstance( this );
	cbData->setGroupName( groupName );
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMContactList::removeGroupCbk ) );
}

//-----------------------------------------------------------------------------

bool PurpleIMContactList::changeGroupNameCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);

	PurpleIMContactListCallbackData* cbData = (PurpleIMContactListCallbackData*)dataIn;

	PurpleGroup *gGroup = purple_find_group( cbData->getOldGroupName().c_str() );
	
	if (gGroup) 
	{
		purple_blist_rename_group(gGroup, cbData->getNewGroupName().c_str() );
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::changeGroupName(const std::string & oldGroupName, const std::string & newGroupName)
{
	PurpleIMContactListCallbackData* cbData = new PurpleIMContactListCallbackData;

	cbData->setInstance( this );
	cbData->setOldGroupName( oldGroupName );
	cbData->setNewGroupName( newGroupName );
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMContactList::changeGroupNameCbk ) );
}

//-----------------------------------------------------------------------------

bool PurpleIMContactList::moveContactToGroupCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);

	PurpleIMContactListCallbackData* cbData = (PurpleIMContactListCallbackData*)dataIn;

	PurpleAccount* gAccount  = getPurpleAccount( cbData->getInstance() );
	PurpleGroup*   gOldGroup = purple_find_group( cbData->getOldGroupName().c_str() );

	if (gOldGroup) 
	{
		PurpleBuddy *gBuddy = purple_find_buddy_in_group(gAccount, cbData->getContactId().c_str(), gOldGroup);
		if (gBuddy) 
		{
			PurpleGroup *gNewGroup = purple_find_group( cbData->getNewGroupName().c_str() );
			if (gNewGroup) 
			{
				gNewGroup = purple_group_new(cbData->getNewGroupName().c_str());
				purple_blist_add_buddy(gBuddy, NULL, gNewGroup, NULL);
			}
		}
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::moveContactToGroup(const std::string & newGroupName, const std::string & oldGroupName, const std::string & contactId)
{
	PurpleIMContactListCallbackData* cbData = new PurpleIMContactListCallbackData;

	cbData->setInstance( this );

	cbData->setInstance( this );
	cbData->setNewGroupName( newGroupName );
	cbData->setOldGroupName( oldGroupName );
	cbData->setContactId   ( contactId    );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMContactList::moveContactToGroupCbk ) );
}

//-----------------------------------------------------------------------------

PurpleAccount* PurpleIMContactList::getPurpleAccount( PurpleIMContactList* imContactList )
{
	return purple_accounts_find( imContactList->getIMAccount().getLogin().c_str(), 
							     PurpleIMPrcl::GetPrclId(imContactList->getIMAccount().getProtocol()));
}

//-----------------------------------------------------------------------------

int PurpleIMContactList::timeoutAdd( void* cbData, GSourceFunc callbackFunc )
{
	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);
	return purple_timeout_add(0, callbackFunc, cbData);
}

//-----------------------------------------------------------------------------

void PurpleIMContactList::timeoutRemove( PurpleIMContactListCallbackData* cbData )
{
	int waitTime = 1;
	int totalWaitTime = 0;
	while ( cbData->getCallbackId() == 0 )
	{
		Thread::msleep( waitTime );
		totalWaitTime += waitTime;

#ifdef _WIN32
		char msg[200];
		sprintf_s( msg, "PurpleIMContactList::timeoutRemove - waiting for valid callbackId. %d ms\n", totalWaitTime );
		OutputDebugString( msg );
#endif
	}

#ifdef _WIN32
	if ( totalWaitTime > 0 )
	{
		char msg[200];
		sprintf_s( msg, "\n" );
		OutputDebugString( msg );
	}
#endif

	purple_timeout_remove( cbData->getCallbackId() );
}

//-----------------------------------------------------------------------------

//VOXOX - JRT - 2009.05.18 - Hold off for now.
//bool PurpleIMContactList::getContactVcardCbk(void * data)
//{
//	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);
//
//	PurpleIMContactList * imContactList = ((misc_t *)data)->instance;
//	const char * contactId =(const char *)((misc_t *)data)->data_ptr1;
//	PurpleAccount *gAccount = purple_accounts_find(imContactList->getIMAccount().getLogin().c_str(),
//								PurpleIMPrcl::GetPrclId(imContactList->getIMAccount().getProtocol()));
//
//	PurpleConnection		 *gc	    = purple_account_get_connection(gAccount);
//
//	if ( gc ) 
//	{
//		serv_get_info( gc, contactId );
//	}
//
//	purple_timeout_remove(((misc_t *)data)->cbk_id);
//
//	free((misc_t *)data);
//
//	return TRUE;
//}
//
//void PurpleIMContactList::getContactVcard( const std::string& contactId )
//{
//	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
//	memset(data, 0, sizeof(misc_t));
//
//	data->instance = this;
//	data->data_ptr1 = strdup(contactId.c_str());
//
//	Mutex::ScopedLock lock(PurpleIMContactList::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMContactList::getContactVcardCbk, data);
//}

//-----------------------------------------------------------------------------
