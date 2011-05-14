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
#include "ContactGroup.h"

#include "Contact.h"
#include "IMContactListHandler.h"

#define LOGGER_COMPONENT "ContactList"
#include <util/Logger.h>
#include <util/Uuid.h>

ContactGroup::ContactGroup(const std::string & groupName, EnumGroupType::GroupType type )
{
	initVars();
	setName( groupName );
	setType( type );
	updateKey();
}

ContactGroup::ContactGroup(const ContactGroup & contactGroup)
{
	copy( contactGroup );
}

void ContactGroup::initVars()
{
//	setUUID		( Uuid::generateString() );
	setId		( 0 );			//Used with JSON
	setIsOpen	( true );		//Default to open.
	setSortOrder( 0 );			//User-defined sort order.
	setType		( EnumGroupType::GroupType_User );

	setTempSortOrder( 0 );
}

void ContactGroup::copy( const ContactGroup& src )
{
	if ( &src != this )
	{
		setId       ( src.getId()		 );
		setName	    ( src.getName()		 );
		setIsOpen	( src.isOpen()		 );
		setSortOrder( src.getSortOrder() );
//		setUUID     ( src.getUUID()		 );
		setType		( src.getType()		 );
		setTempSortOrder( src.getTempSortOrder() );
	}
}
//End VoxOx


void ContactGroup::addContact(Contact & contact) 
{
	Mutex::ScopedLock lock(_mutex);

	_contactList.push_back(&contact);

	contactAddedEvent(*this, contact);
	LOG_DEBUG("Contact added to group " + _groupName);
}

void ContactGroup::removeContact(Contact & contact) 
{
	Mutex::ScopedLock lock(_mutex);

	for (ContactVector::iterator it = _contactList.begin(); it != _contactList.end(); ++it) 
	{
		if ((*(*it)) == contact) 
		{
			contactRemovedEvent(*this, *(*it));
			_contactList.erase(it);
			break;
		}
	}
}

Contact * ContactGroup::operator[](unsigned int i) const 
{
	Mutex::ScopedLock lock(_mutex);

	if (i <= size()) 
	{
		return _contactList[i];
	} 
	else 
	{
		return NULL;
	}
}

bool ContactGroup::operator==(const ContactGroup & contactGroup) const 
{
	bool bEqual = false;

	if ( _groupName == contactGroup._groupName)		//VOXOX - JRT - 2009.05.05 JRT-GRPS
	{
		bEqual = (getType() == contactGroup.getType());
	}

	return bEqual;
}

bool ContactGroup::operator<(const ContactGroup & contactGroup) const 
{
	return (_groupName < contactGroup._groupName);
}

void ContactGroup::setName(const std::string & groupName) 
{
	_groupName = groupName;
	updateKey();		//VOXOX - JRT - 2009.05.05 
	contactGroupModifiedEvent(*this);
}

void ContactGroup::setType( EnumGroupType::GroupType val )	
{ 
	_type = val;	
	updateKey();
}

std::list<std::string> ContactGroup::getMobilePhoneList() const 
{
	std::list<std::string> list;
	for (ContactVector::const_iterator it = _contactList.begin(); it != _contactList.end(); ++it) 
	{
		std::string getMobilePhone = (*it)->getMobilePhone();
		if( getMobilePhone != "" ) 
		{
			list.push_front(getMobilePhone);
		}
	}

	return list;
}

void ContactGroup::getCounts( int& total, int& online )
{
	total  = 0;
	online = 0;

	Mutex::ScopedLock lock(_mutex);

	for (ContactVector::const_iterator it = _contactList.begin(); it != _contactList.end(); ++it) 
	{
		if((*it)->isAvailable())
		{
			online++;
		}

		total++;
	}
}

std::string ContactGroup::getContactsOnline() 
{
	int onlineContacts = 0;
	int totalContacts  = 0;

	getCounts( totalContacts, onlineContacts );

	std::string returnString = " (" + String::fromNumber(onlineContacts) + "/" + String::fromNumber(totalContacts) + ")";

	return returnString;
}

int	ContactGroup::getHowManyContactsMustMove()
{
	int result = 0;

	for ( ContactGroup::ContactVector::const_iterator it = _contactList.begin(); it != _contactList.end(); ++it) 
	{
		if ( (*it)->getUserGroupCount() <= 1 )
		{
			result++;
		}
	}

	return result;
}

void ContactGroup::updateKey()
{
	std::string tempName = String::toLowerCase( getName() );	//VOXOX - JRT - 2009.08.09 - Group names are case-insensitive.
	_key = tempName + EnumGroupType::toString( getType() );
}

//=============================================================================


//=============================================================================

ContactGroupSet::ContactGroupSet()
{
	_nOrderInc = 10;
	_bOrdered  = false;
	_nNextId   = 0;
}

//-----------------------------------------------------------------------------

ContactGroupSet::~ContactGroupSet()
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 - May be in use, so lock to avoid COE.
}

//-----------------------------------------------------------------------------
//Overrides to handle Manual Ordering.
ContactGroup* ContactGroupSet::Add1( const ContactGroup& rItem, bool* pAdded )
{
	return Add1( &rItem, pAdded );
}

//-------------------------------------------------------------------------------

ContactGroup* ContactGroupSet::Add1( const ContactGroup* pItem, bool* pAdded )
{
	ContactGroup* result = NULL;

	int nOrder = this->size() * getOrderInc();
	const_cast<ContactGroup*>(pItem)->setSortOrder( nOrder );
	const_cast<ContactGroup*>(pItem)->setId( ++_nNextId );
	result = Add( pItem, pAdded );

	return result;
}

//-----------------------------------------------------------------------------

ContactGroup* ContactGroupSet::FindByGroup( const ContactGroup& group )
{
	return FindByKey( group.getKey() );	//Does locking
}

//-----------------------------------------------------------------------------

ContactGroup* ContactGroupSet::FindByKey( const std::string& key )
{
	return Lookup( key );	//Does locking
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.14 
ContactGroup* ContactGroupSet::getFirstUserGroup()
{
	ContactGroup* result = NULL;

	//Ensure we have one.
	if ( this->empty() )
	{
		bool added = false;
		ContactGroup group( "Buddies", EnumGroupType::GroupType_User );	//VOXOX - JRT - 2009.07.14 - TODO: get real default group name.
		this->Add1( group, &added );
	}

	for ( ContactGroupSet::iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).second.isUser() )
		{
			result = &(*it).second;
			break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
	
void ContactGroupSet::changeOrder( const std::string& dragGroupKey, const std::string dropGroupKey )
{
	if ( !isOrdered() )
	{
		initOrdering();
	}

	assert( isOrdered() );

	ContactGroup* pDrop = FindByKey( dropGroupKey );
	ContactGroup* pDrag = FindByKey( dragGroupKey );

	if ( pDrop )
	{
		if ( pDrag )
		{
			pDrag->setSortOrder( pDrop->getSortOrder() + 1 );
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		assert(false);
	}

	reassignOrdering();
}

//-----------------------------------------------------------------------------

void ContactGroupSet::initOrdering()
{
	int nOrder    = 10;

	//Map should be in alpha order, so just iterate the map.
	for ( ContactGroupSet::iterator it = begin(); it != end(); it++ )
	{
		(*it).second.setSortOrder( nOrder );
		nOrder += getOrderInc();
	}

	_bOrdered = true;
}

//-----------------------------------------------------------------------------

void ContactGroupSet::reassignOrdering()
{
	int nNewOrder = size() * getOrderInc();
	int nTgtOrder = nNewOrder + getOrderInc();

	ContactGroup* pGroup = FindNextLesserOrder( nTgtOrder );

	//Set new sort orders in temp memvar
	while ( pGroup )
	{
		nTgtOrder = pGroup->getSortOrder();	//Save old order for next loop.
		pGroup->setTempSortOrder( nNewOrder );
		nNewOrder -= getOrderInc();

		pGroup = FindNextLesserOrder( nTgtOrder );
	}

	//Now move tempOrders to Order
	int nOrder = 0;
	for ( ContactGroupSet::iterator it = begin(); it != end(); it++ )
	{
		nOrder = (*it).second.getTempSortOrder();
		(*it).second.setSortOrder( nOrder );
	}
}

//-----------------------------------------------------------------------------

ContactGroup* ContactGroupSet::FindNextLesserOrder( int nTgtOrder )
{
	ContactGroup* pRet = NULL;

	for ( ContactGroupSet::iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).second.getSortOrder() < nTgtOrder )
		{
			if ( pRet )
			{
				if ( (*it).second.getSortOrder() > pRet->getSortOrder() )
				{
					pRet = &(*it).second;
				}
			}
			else
			{
				pRet = &(*it).second;
			}
		}
	}

	return pRet;
}

//=============================================================================


//=============================================================================
	
ContactGroupInfo::ContactGroupInfo( const std::string& key, EnumGroupType::GroupType type )
{
	_key  = key;
	_type = type;
}

//-----------------------------------------------------------------------------

ContactGroupInfo::~ContactGroupInfo()
{
}

//=============================================================================

ContactGroupInfoSet::ContactGroupInfoSet()
{
}

//-----------------------------------------------------------------------------

ContactGroupInfoSet::~ContactGroupInfoSet()
{
}

//-----------------------------------------------------------------------------

ContactGroupInfo* ContactGroupInfoSet::FindByGroupInfo( const ContactGroupInfo& groupInfo )
{
	return FindByKey( groupInfo.getKey() );
}

//-----------------------------------------------------------------------------

ContactGroupInfo* ContactGroupInfoSet::FindByKey( const std::string& key )
{
	return Lookup( key );
}

//-----------------------------------------------------------------------------

ContactGroupInfo* ContactGroupInfoSet::FindFirstUserGroupInfo() 
{
	ContactGroupInfo* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).second.isUser() )
		{
			pRet = &(const_cast<ContactGroupInfo&>((*it).second));
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

ContactGroupInfo* ContactGroupInfoSet::FindFirstOrigNetworkGroupInfo() 
{
	ContactGroupInfo* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).second.isOrigNetwork() )
		{
			pRet = &(const_cast<ContactGroupInfo&>((*it).second));
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

ContactGroupInfo* ContactGroupInfoSet::FindFirstFilterGroupInfo() 
{
	ContactGroupInfo* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).second.isFilter() )
		{
			pRet = &(const_cast<ContactGroupInfo&>((*it).second));
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

std::string ContactGroupInfoSet::GetFirstUserGroupKey()	// const	//Mostly for backward compatibility.
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	ContactGroupInfo* pGroup = FindFirstUserGroupInfo();
	std::string       result = (pGroup ? pGroup->getKey() : "");

	return result;
}

//-----------------------------------------------------------------------------

std::string ContactGroupInfoSet::GetOrigNetworkGroupKey()
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	ContactGroupInfo* pGroup = FindFirstOrigNetworkGroupInfo();
	std::string       result = (pGroup ? pGroup->getKey() : "");

	return result;
}

//-----------------------------------------------------------------------------

std::string ContactGroupInfoSet::GetFilterGroupKey()
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	ContactGroupInfo* pGroup = FindFirstFilterGroupInfo();
	std::string       result = (pGroup ? pGroup->getKey() : "");

	return result;
}

//-----------------------------------------------------------------------------

int ContactGroupInfoSet::GetCountByType( EnumGroupType::GroupType tgtType )
{
	int count = 0;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).second.getType() == tgtType )
		{
			count++;
		}
	}

	return count;
}

//-----------------------------------------------------------------------------

bool ContactGroupInfoSet::IsFilter( const std::string& key ) const
{
	bool result = false;

	if ( !key.empty() )
	{
		ContactGroupInfo* cgi = const_cast<ContactGroupInfoSet*>(this)->FindByKey( key );

		if ( cgi )
		{
			result = cgi->isFilter();
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

bool ContactGroupInfoSet::CanDrag( const std::string& key ) const
{
	bool result = false;

	if ( !key.empty() )
	{
		ContactGroupInfo* cgi = const_cast<ContactGroupInfoSet*>(this)->FindByKey( key );

		if ( cgi )
		{
			switch( cgi->getType() )
			{
			case EnumGroupType::GroupType_User:
				result = true;
				break;

			case EnumGroupType::GroupType_Filter:
			case EnumGroupType::GroupType_Network:
			case EnumGroupType::GroupType_OrigNetwork:
				result = false;
				break;

			default:
				result = false;
				assert(false);		//New type?
			}
		}
	}

	return result;
}


//=============================================================================
