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

#ifndef CONTACTGROUP_H
#define CONTACTGROUP_H

#include "EnumGroupType.h"
#include <util/Event.h>
#include <thread/Mutex.h>
#include <thread/RecursiveMutex.h>

#include <string>

#include "../profile/VoxListTemplate.h"		//VOXOX - JRT - 2009.05.05 - For map

class Contact;
class ContactList;

//=============================================================================

/**
 * A group of contacts (i.e family, friends ect...).
 *
 * Contains a list of Contact.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class ContactGroup {
	friend class ContactList;
public:

	/**
	 * A ContactGroup has been modified/updated.
	 *
	 * @param sender ContactGroup modified/updated
	 */
	Event<void(ContactGroup & sender)> contactGroupModifiedEvent;

	/**
	 * A Contact has been added.
	 *
	 * @param sender this class
	 * @param contact Contact added
	 */
	Event<void(ContactGroup & sender, Contact & contact)> contactAddedEvent;

	/**
	 * A Contact has been removed.
	 *
	 * @param sender this class
	 * @param contact Contact removed
	 */
	Event<void(ContactGroup & sender, Contact & contact)> contactRemovedEvent;

//	ContactGroup(const std::string & groupName);
	ContactGroup(const std::string & groupName, EnumGroupType::GroupType type );	//VOXOX - JRT - 2009.05.05 

	ContactGroup(const ContactGroup & contactGroup);

	void setName(const std::string & groupName);	//VOXOX - JRT - 2009.05.01 - Made public

	std::string getName() const					{ return _groupName; }
	int			getId() const					{ return _id;		 }

	/**
	 * Permits to use ContactGroup as an array.
	 *
	 * @see List::operator[]
	 * @param i index inside the array
	 * @return the Contact that corresponds to the index i inside the ContactGroup or NULL
	 */
	Contact * operator[](unsigned i) const;

	/**
	 * @return the number of Contact in this ContactGroup
	 */
	unsigned size() const						{ return _contactList.size(); }

	/**
	 * @return a list containing all mobile phone number
	 */
	std::list<std::string> getMobilePhoneList() const;

	/**
	 * Gets the UUID of this ContactGroup.
	 *
	 * @return the UUID of this ContactGroup
	 */
//	std::string getUUID() const {		//VOXOX - JRT - 2009.05.05 
//		return _uuid;
//	}

	std::string getContactsOnline();				//TODO: This should be a UI method.
	void		getCounts( int& total, int& online );
	std::string	getKey() const								{ return _key;	}	//VOXOX - JRT - 2009.05.05 - For use in contact group collection.
	

	/**
	 * Compare two groups.
	 *
	 * @param contactGroup group to compare
	 * @return true if equal
	 */
	bool operator==(const ContactGroup & contactGroup) const;

	bool operator<(const ContactGroup & contactGroup) const;

//	std::string serialize();

//	bool unserialize(const std::string & data);

	//VOXOX - JRT - 2009.04.30 
	bool isOpen()		const							{ return _bOpen;	}
	int	 getSortOrder() const							{ return _nOrder;	}
	int	 getTempSortOrder() const						{ return _nTempOrder;	}
	EnumGroupType::GroupType getType() const			{ return _type;		}

	void setIsOpen   ( bool val )						{ _bOpen   = val;	}
	void setSortOrder( int  val ) 						{ _nOrder  = val;	}
	void setId       ( int  val )						{ _id	   = val;	}
	void setType     ( EnumGroupType::GroupType val );

	bool isUser()		 const							{ return getType() == EnumGroupType::GroupType_User;		}
	bool isOrigNetwork() const							{ return getType() == EnumGroupType::GroupType_OrigNetwork;	}
	bool isNetwork()	 const							{ return getType() == EnumGroupType::GroupType_Network;		}
	bool isFilter()		 const							{ return getType() == EnumGroupType::GroupType_Filter;		}

	void copy( const ContactGroup& src );

	void setTempSortOrder( int val )					{ _nTempOrder = val;	}

	int	getHowManyContactsMustMove();

private:
	void initVars();		//VOXOX - JRT - 2009.04.30 
	void updateKey();

//	void setUUID( const std::string& val )				{ _uuid = val;	}	//VOXOX - JRT - 2009.05.05 
	/**
	 * Sets the name of the group.
	 *
	 * @param groupName the desired name
	 */
//	void setName(const std::string & groupName);	//VOXOX - JRT - 2009.05.01 - Made public

	/**
	 * Add a Contact to the ContactGroup.
	 *
	 * Must only be called by ContactList
	 *
	 * @param contact the Contact to add
	 */
	void addContact(Contact & contact);

	/**
	 * Remove a Contact from the ContactGroup.
	 *
	 * Must only be called by ContactList
	 *
	 * @param contact the Contact to remove
	 */
	void removeContact(Contact & contact);

	/** Defines the vector of Contact. */
	typedef std::vector<Contact *> ContactVector;

	/** Name of the ContactGroup. */
	std::string _groupName;

	/** List of Contact associated with the ContactGroup. */
	ContactVector _contactList;

	/** The UUID of this ContactGroup. */
//	std::string _uuid;		//VOXOX - JRT - 2009.05.05 

	mutable Mutex _mutex;

	//VOXOX - JRT - 2009.04.30 - UI-related items
	int			_id;			//Used with JSON		//VOXOX - JRT - 2009.06.28 
	std::string	_key;			//To replace UUID.
	bool		_bOpen;			//Is group currently open/closed.
	int			_nOrder;		//User-defined sort order.
	int			_nTempOrder;
	EnumGroupType::GroupType	_type;
	//End VoxOx
};

//=============================================================================


//=============================================================================

class ContactGroupSet : public VoxMapTemplate<std::string, ContactGroup>
{
public:
	ContactGroupSet();
	virtual ~ContactGroupSet();

	ContactGroup* FindByGroup( const ContactGroup& group );
	ContactGroup* FindByKey  ( const std::string&  key   );
	ContactGroup* getFirstUserGroup();

	void changeOrder( const std::string& dragGroupKey, const std::string dropGroupKey );

	//Overrides to handle Manual Ordering.
	ContactGroup* Add1( const ContactGroup& rItem, bool* pAdded );
	ContactGroup* Add1( const ContactGroup* pItem, bool* pAdded );

	void lock()							{ _mutex.lock();	}
	void unlock()						{ _mutex.unlock();	}

protected:
	bool isOrdered()					{ return _bOrdered;	}
	int	 getOrderInc()					{ return _nOrderInc;}
	void initOrdering();
	void reassignOrdering();
	ContactGroup* FindNextLesserOrder( int nTgtOrder );

private:
	bool _bOrdered;
	int	 _nOrderInc;
	int	 _nNextId;

	mutable RecursiveMutex _mutex;
};

//=============================================================================


//=============================================================================
//Let's create a small subset of the ContactGroup to use as keys to the
//	main ContactGroupSet.  This is used in each Contact object.
//=============================================================================

class ContactGroupInfo
{
public:
	ContactGroupInfo( const std::string& key, EnumGroupType::GroupType type );
	virtual ~ContactGroupInfo();

	std::string				  getKey() const			{ return _key;	}
	EnumGroupType::GroupType getType() const			{ return _type;	}

	void setKey ( const std::string& str )				{ _key  = str;	}
	void setType( EnumGroupType::GroupType val )		{ _type = val;	}

	bool isUser()		 const							{ return getType() == EnumGroupType::GroupType_User;		}
	bool isOrigNetwork() const							{ return getType() == EnumGroupType::GroupType_OrigNetwork;	}
	bool isNetwork()	 const							{ return getType() == EnumGroupType::GroupType_Network;		}
	bool isFilter()		 const							{ return getType() == EnumGroupType::GroupType_Filter;		}

private:
//	mutable Mutex _mutex;

	std::string					_key;
	EnumGroupType::GroupType	_type;
};

//=============================================================================

//=============================================================================

class ContactGroupInfoSet : public VoxMapTemplate<std::string, ContactGroupInfo>
{
public:
	ContactGroupInfoSet();
	virtual ~ContactGroupInfoSet();

	ContactGroupInfo* FindByGroupInfo( const ContactGroupInfo& groupInfo );
	ContactGroupInfo* FindByKey      ( const std::string&   key  );

	ContactGroupInfo* FindFirstUserGroupInfo();
	ContactGroupInfo* FindFirstOrigNetworkGroupInfo();
	ContactGroupInfo* FindFirstFilterGroupInfo();

	std::string GetFirstUserGroupKey();

	std::string GetOrigNetworkGroupKey();
	std::string GetFilterGroupKey();

	int GetCountByType( EnumGroupType::GroupType tgtType );

	int GetUserCount()					{ return GetCountByType( EnumGroupType::GroupType_User );  }

	bool IsFilter( const std::string& key ) const;
	bool CanDrag ( const std::string& key ) const;

};


//=============================================================================

#endif	//CONTACTGROUP_H

