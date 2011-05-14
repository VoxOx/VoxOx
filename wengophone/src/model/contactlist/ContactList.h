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

#ifndef OWCONTACTLIST_H
#define OWCONTACTLIST_H

#include "ContactGroup.h"
#include "Contact.h"

#include <imwrapper/EnumPresenceState.h>

#include <thread/RecursiveMutex.h>

#include <util/Event.h>
#include <util/OWPicture.h>
#include <util/Trackable.h>

#include <set>
#include "../profile/VoxListTemplate.h"	

class IMAccount;
class IMAccountManager;
class IMContact;
class IMContactList;
class IMContactListHandler;
class IMPresence;
class PresenceHandler;
class StringList;
class UserProfile;
class WsContactProfile;	//VOXOX - JRT - 2009.05.26 


//=============================================================================
//VOXOX - JRT - 2009.07.21 - Implement small class and list to gather info for various menus.
//=============================================================================

class ContactInfo2
{
public:
	ContactInfo2()	{ initVars();	}

	ContactInfo2& operator=( const ContactInfo2& src )
	{
		if ( this != &src )
		{
			setKey		  ( src.getKey()		 );
			setDisplayName( src.getDisplayName() );
			setWengoId    ( src.getWengodId()	 );
			setPresenceState( src.getPresenceState() );

			getTelephones() = const_cast<ContactInfo2&>(src).getTelephones();
		}

		return *this;
	}

	bool operator==( const ContactInfo2& src )
	{
		bool bEqual = false;

		if ( getKey()			== src.getKey()			&&
			 getDisplayName()	== src.getDisplayName() &&
			 getWengodId()		== src.getWengodId()	&&
			 getPresenceState() == src.getPresenceState() )
		{
			bEqual = true;
		}

		return bEqual;
	}

	std::string	getKey()		  const								{ return _key;			}
	std::string	getDisplayName()  const								{ return _displayName;	}
	std::string	getWengodId()	  const								{ return _wengoId;		}
	EnumPresenceState::PresenceState getPresenceState() const		{ return _presenceState;}

	Telephones& getTelephones()										{ return _telephones;	}

	void setKey          ( const std::string& val )					{ _key		     = val;	}
	void setDisplayName  ( const std::string& val )					{ _displayName   = val;	}
	void setWengoId      ( const std::string& val )					{ _wengoId	     = val;	}
	void setPresenceState( EnumPresenceState::PresenceState val )	{ _presenceState = val;	}

	void initVars()
	{
		_key		   = "";
		_displayName   = "";
		_wengoId	   = "";
		_presenceState = EnumPresenceState::PresenceStateUnknown;

		_telephones.DeleteAll();
	}

protected:

private:
	std::string	_key;
	std::string	_displayName;
	std::string	_wengoId;
	Telephones	_telephones;
	
	EnumPresenceState::PresenceState _presenceState;

};

//=============================================================================

class ContactInfoList2 : public VoxListTemplate<ContactInfo2>
{
public:
};

//=============================================================================


//=============================================================================
//VOXOX - JRT - 2009.04.07 - Implement a map based Contacts for better performance
//							 and consistent interface.
//=============================================================================

class Contacts : public VoxMapTemplate<std::string, Contact>
{
public:
	Contacts();
	virtual ~Contacts();

	Contact* FindByImContact  ( const IMContact&   imContact );
	Contact* FindByUuid       ( const std::string& uuid      );
	Contact* FindByContactId  ( const std::string& contactId );
	Contact* FindByContactIdUserNetwork( const std::string& contactId, int userNetwork );
	Contact* FindByVoxOxPhoneNumber( const std::string& phoneNumber );//VOXOX CHANGE by Rolando 04-27-09 - Used to get Contact by phonenumber
	Contact* FindByEmail( const std::string& email );			//VOXOX - CJC - 2009.06.11 
	Contact* FindByJSONId( int JsonId );						//VOXOX - JRT - 2009.06.29 
	
	Contact * FindByPhoneNumber(const std::string & number);	//VOXOX - CJC - 2009.06.14 
	void	 RemoveImAccount( const std::string imAccountId );
};

//=============================================================================

/**
 * Contact list.
 *
 * List of ContactGroup.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class ContactList : public Trackable {
	friend class Contact;
	friend class ContactListXMLSerializer;
	friend class ContactListXMLSerializer1;
	friend class ContactListJSONSerializer;	//VOXOX - JRT - 2009.06.27 
	friend class ContactXMLSerializer;
	friend class ContactXMLSerializer1;
public:

	/** Set of ContactGroup. */
//	typedef std::set<ContactGroup> ContactGroupSet;		//VOXOX - JRT - 2009.05.05 - Use new map-based class

	/** List of Contact. */
//	typedef std::list<Contact> Contacts;//VOXOX - JRT - 2009.04.07 

	ContactList(UserProfile & userProfile);

	~ContactList();

	//VOXOX - JRT - 2009.04.12 
	Event<void (ContactList& sender, int nOnOff)> setInitialLoadEvent;
	Event<void (ContactList& sender, WsContactProfile& wsContactProfile)> wsContactProfileCreatedEvent;	//VOXOX - JRT - 2009.05.26 
	//End VoxOx

	/**
	 * A ContactGroup has been added.
	 *
	 * @param sender this class
	 * @param contactGroup ContactGroup added
	 */
	Event<void (ContactList & sender, ContactGroup & contactGroup)> contactGroupAddedEvent;

	/**
	 * A ContactGroup has been removed.
	 *
	 * @param sender this class
	 * @param contactGroup ContactGroup removed
	 */
	Event<void (ContactList & sender, ContactGroup & contactGroup)> contactGroupRemovedEvent;

	/**
	 * A ContactGroup has been renamed.
	 *
	 * @param sender this class
	 * @param contactGroup ContactGroup renamed
	 */
	Event<void (ContactList & sender, ContactGroup & contactGroup)> contactGroupRenamedEvent;

	Event<void (ContactList & sender )> contactGroupsReorderedEvent;				//VOXOX - JRT - 2009.05.11 

	Event<void (ContactList & sender, ContactProfile& profile)> contactProfileGetEvent;	//VOXOX - JRT - 2009.05.26 
	Event<void (ContactList & sender, std::string& contactId )> contactProfileSetEvent;	//VOXOX - JRT - 2009.05.26 

	/**
	 * A Contact has been added to a ContactGroup.
	 *
	 * @param sender this class
	 * @param contactGroup the ContactGroup where the Contact has been added to
	 * @param contact the added Contact
	 */
	Event<void (ContactList & sender, Contact & contact)> contactAddedEvent;

	/**
	 * A Contact has been removed from a ContactGroup.
	 *
	 * @param sender this class
	 * @param contactGroup the ContactGroup where the Contact has been removed from
	 * @param contact the removed Contact
	 */
//	Event<void (ContactList & sender, Contact & contact)> contactRemovedEvent;
	Event<void (ContactList & sender, std::string& contactId )> contactRemovedEvent;	//VOXOX - JRT - 2009.04.28 

	/**
	 * @see contactProfilePresenceChangedEvent
	 * FIXME not finished yet, this is for notification/toaster
	 */
	Event<void (ContactList & sender, Contact & contact)> contactPresenceChangedEvent;

	/**
	 * A Contact has moved.
	 *
	 * @param sender this class
	 * @param groupName the destinaton group
	 * @param contact the removed Contact
	 */
	Event<void (ContactList & sender, ContactGroup & dstContactGroup,
		ContactGroup & srcContactGroup, Contact & contact)> contactMovedEvent;

	/**
	 * Emitted when a Contact has been changed.
	 *
	 * @param sender this class
	 * @param contact the changed Contact
	 */
	Event<void (ContactList & sender, Contact & contact)> contactChangedEvent;

	/**
	 * Adds a ContactGroup.
	 *
	 * If a ContactGroup with the given name exists, no ContactGroup is created
	 *
	 * @param name name of the ContactGroup to add.
	 */
//	void addContactGroup(const std::string & name);
	void addContactGroup(const ContactGroup& group);	//VOXOX - JRT - 2009.05.07 JRT-GRPS

	/**
	 * Removes a ContactGroup.
	 *
	 * If no ContactGroup with the given name exists, nothing happens
	 *
	 * @param groupId the UUID of the ContactGroup to remove.
	 */
	void removeContactGroup(const std::string & groupId);

	/**
	 * Renames a ContactGroup.
	 *
	 * @param groupId the UUID of the group to rename
	 * @param name the desired name
	 */
	void renameContactGroup(const std::string & groupId, const std::string & name);

	//VOXOX - JRT - 2009.04.11 
	void setInitialLoad( int nOnOff );
	//End VoxOx

	/**
	 * Creates and adds a Contact to the ContactList.
	 *
	 * @return a reference to the new Contact
	 */
//	Contact& createContact();
	Contact* createContact();
	Contact* addContact		   ( Contact* contact );
	void	 addToContactGroups( Contact* pContact );	//VOXOX - JRT - 2009.05.09 JRT-GRPS

	/**
	 * Removes a Contact.
	 *
	 * @param the contact to remove
	 */
	void removeContact(Contact & contact);

	/**
	 * Merges two Contacts.
	 *
	 * Merging two Contact will result in a deletation of the source Contact
	 * after fulling the destination Contact with missing information that
	 * come from the source Contact.
	 *
	 * @param src the source Contact
	 * @param dst the destination Contact
	 */
	void mergeContacts(Contact & dst, Contact & src);

	/**
	 * Moves a Contact from to a group.
	 *
	 * @param contact the Contact to move
	 * @param dst the destination group
	 */
	void moveContactToGroup( const std::string& srcGroupId, const std::string & destGroupId, Contact & contact);	//VOXOX - JRT - 2009.08.09 - Multiple groups.

	/**
	 * Gets a ContactGroup given its UUID.
	 *
	 * @param groupId the group UUID of the desired ContactGroup
	 * @return a pointer to the ContactGroup, NULL if not found
	 */
	ContactGroup * getContactGroup(const std::string & groupId);	//VOXOX - JRT - 2009.05.05  - removed const

	/**
	 * @return a copy of the set of ContactGroups.
	 */
	//VOXOX - JRT - 2009.05.09 - This returns a COPY of the contactGroupSet.  That is what operator=() is for!  Use getContactGroupRef() below.
//	ContactGroupSet getContactGroupSet() const {
//		return ContactGroupSet(_contactGroupSet);
//	}

	ContactGroupSet& getContactGroupSetRef()		{ return _contactGroupSet;	}	//VOXOX - JRT - 2009.05.01 

	void orderGroups( const std::string& dragGroupId, const std::string dropGroupId );	//VOXOX - JRT - 2009.05.11 

	/**
	 * Gets the list of Contacts.
	 *
	 * @return the list of Contacts.
	 */
	const Contacts & getContacts() const {
		return _contacts;
	}

	/**
	 * Finds the first Contact that owns an IMContact.
	 *
	 * @param imContact the IMContact to look for
	 * @return the Contact or NULL if not found
	 */
	Contact * findContactThatOwns(const IMContact & imContact) ;	//VOXOX - JRT - 2009.04.07 - Remove const

	/**
	 * Gets the Contact of given UUID.
	 *
	 * @param contactId the contact UUID
	 * @return the Contact or NULL if no found
	 */
	Contact * getContactByUuid(const std::string & uuid);
	//VOXOX - CJC - 2009.06.14 Renaming it because this is what it does
	Contact * getContactByVoxOxPhoneNumber(const std::string & phoneNumber); //VOXOX CHANGE - 04-27-09 - Used to get contact by phoneNumber

	//VOXOX CHANGE CJC GET CONTACT BY ITS ID
	Contact * getContactById (const std::string & contactId	);
	Contact * getContactById (const std::string & contactId, int userNetworkId	);
	Contact * getContactByKey(const std::string & key		);
	Contact * getContactByEmail(const std::string & email); //VOXOX - CJC - 2009.06.11 
	//VOXOX - CJC - 2009.06.23 Fix parameter name
	Contact * getContactByNumber(const std::string & phoneNumber); //VOXOX - CJC - 2009.06.11 
	Contact * getContactByJSONId( int JsonId );		//VOXOX - JRT - 2009.06.29 
	
	/**
	 * Gets the UUID of a ContactGroup from its name.
	 *
	 * @param groupName the name of the group to find
	 * @return the UUID of the group. An empty string if not found
	 */
	std::string getContactGroupIdFromName( const std::string &groupName);	//VOXOX - JRT - 2009.05.05 - Removed const
	std::string getContactGroupName      ( const std::string &groupId  );

	std::string getContactGroupContactsOnline(const std::string & groupId);
	StringList	getContactIds() const ;
	void		getContactsWithHomeNumber  ( ContactInfoList2& info ) const;
	void		getContactsWithMobileNumber( ContactInfoList2& info ) const;
	void		getContactsWithWengoCall   ( ContactInfoList2& info ) const;
	void		getContactsWithSipNumber   ( ContactInfoList2& info ) const;
	void		getContactsThatCanChat	   ( ContactInfoList2& info ) const;

	void getCounts( int& total, int& online ) const;	//VOXOX - JRT - 2009.09.03 

	std::string	geDefaultMoveToGroupName() const			{ return _defaultMoveToGroupName;	}

	void createWsContactProfile();
	WsContactProfile*	getWsContactProfile()		{ return _wsContactProfile;			}

	bool isFiltering() const						{ return !_filterGroupText.empty(); }		//VOXOX - JRT - 2009.09.07 

	void populateFilterGroup ( const std::string& text, const std::string& filterName );
	void updateUserNetworkIds( IMAccount& imAccount );

	IMAccount* getIMAccount( const std::string& imAccountId );		//VOXOX - JRT - 2009.09.22 

protected:	//VOXOX - JRT - 2009.06.23 
	/**
	 * Locks the ContactList.
	 *
	 * A locked ContactList can't be modified.
	 * The ContactList must be unlocked by calling unlock.
	 *
	 * This method is used by the model and the control.
	 */
	void lock() const;

	/**
	 * Unlocks the ContactList.
	 *
	 * This method is used by the model and the control.
	 */
	void unlock() const;

	Contact* addContactPrivate ( Contact* contact );	//VOXOX - JRT - 2009.08.10 

private:
	int getImContactUserNetworkId( IMContact& imContact );		//VOXOX - JRT - 2009.06.05 

	//VOXOX - JRT - 2009.04.12 
	void setInitialLoadEventHandler(IMContactListHandler & sender, int nOnOff);
	void removeContactFromAllGroups( Contact& contact );			//VOXOX - JRT - 2009.05.06 	

	/**
	 * @see IMAccountManager::imAccountRemovedEvent
	 */
	void imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMContactListHandler::newIMContactAddedEvent
	 */
	void newIMContactAddedEventHandler(IMContactListHandler & sender,
		const std::string & groupName, IMContact & newIMContact);

	/**
	 * @see IMContactListHandler::imContactRemovedEvent
	 */
	void imContactRemovedEventHandler(IMContactListHandler & sender,
		const std::string & groupName, IMContact & imContact);

	/**
	 * @see IMContactListHandler::imContactMovedEvent
	 */
	void imContactMovedEventHandler(IMContactListHandler & sender,
		const std::string & groupName, IMContact & imContact);

	/**
	 * @see IMContactListHandler::newContactGroupAddedEvent
	 */
//	void newContactGroupAddedEventHandler(IMContactList & sender, const std::string & groupName);
	void newContactGroupAddedEventHandler(IMContactList & sender, const ContactGroup& group );	//VOXOX - JRT - 2009.05.07 JRT-GRPS

	/**
	 * @see IMContactListHandler::contactGroupRemovedEvent
	 */
	void contactGroupRemovedEventHandler(IMContactList & sender, const std::string & groupName);

	/**
	 * @see PresenceHandler::presenceStateChangedEventHandler
	 */
	//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
	void presenceStateChangedEventHandler(PresenceHandler & sender,
		EnumPresenceState::PresenceState state,
		const std::string & alias,const std::string & statusMessage, const IMContact & imContact);

	/**
	 * @see PresenceHandler::contactIconChangedEvent
	 */
//	void contactIconChangedEventHandler(PresenceHandler & sender, const IMContact & imContact, OWPicture icon);
	void contactIconChangedEventHandler(PresenceHandler & sender, const IMContact & imContact, const OWPicture& icon);	//VOXOX - JRT - 2009.08.18 

	/**
	 * @see Contact::contactChangedEvent
	 */
	void contactChangedEventHandler(Contact & sender);


	/**
	 * Add an IMContact to a Contact.
	 *
	 * It sends an asynchronous command to IMContactListHandler.
	 *
	 * Must only be called by Contact
	 *
	 * @param contact the Contact that wants to add an IMContact
	 * @param imContact the IMContact to add
	 */
	void addIMContact(Contact & contact, const IMContact & imContact);

	/**
	 * Remove an IMContact from a Contact.
	 *
	 * It sends an asynchronous command to IMContactListHandler.
	 *
	 * Must only be called by Contact
	 *
	 * @param contact the Contact that wants to remove an IMContact
	 * @param imContact the IMContact to remove
	 */
	void removeIMContact(Contact & contact, const IMContact & imContact);

	/**
	 * Moves a Contact to a group without modifying IM lists.
	 */
	void _moveContactToGroup( const std::string& srcGroupName, const std::string & destGroupName, Contact & contact);	//VOXOX - JRT - 2009.08.09 - Multiple groups.

	/**
	 * Actually add a ContactGroup.
	 *
	 * @param groupName the group name
	 */
//	void _addContactGroup(const std::string & groupName);
	void _addContactGroup(const ContactGroup& group);	//VOXOX - JRT - 2009.05.07 JRT-GRPS

	/**
	 * Actually remove a ContactGroup.
	 *
	 * @param groupId the group UUID
	 */
	void _removeContactGroup(const std::string & groupId);

	/**
	 * Actually add the Contact to a ContactGroup.
	 *
	 * @param contact the Contact
	 * @param groupName the ContactGroup
	 */
//	void _addToContactGroup(const std::string & groupName, Contact & contact);
	void _addToContactGroup(const ContactGroup& group, Contact& contact);	//VOXOX - JRT - 2009.05.07 JRT-GRPS
	void addToNonUserGroups( Contact*pContact );

	/** Set of ContactGroup. */
	ContactGroupSet _contactGroupSet;

	/** Vector of Contact. */
	Contacts _contacts;

	UserProfile & _userProfile;

	IMContactListHandler & _imContactListHandler;

	WsContactProfile*	_wsContactProfile;
	void wsContactProfileSetEventHandler( WsContactProfile& sender, int id, const std::string& contactId, int userNetworkId, int timestamp );
	void wsContactProfileGetEventHandler( WsContactProfile& sender, int id, ContactProfile& profile );
	void requestContactProfile( const std::string contactId, int userNetworkId, int timestamp );
	//End VoxOx

	std::string _filterGroupText;
	int			_nextId;

	mutable RecursiveMutex _mutex;
	mutable RecursiveMutex _mutexContactGroup;	//VOXOX - JRT - 2009.06.22 

	static std::string _defaultMoveToGroupName;		//VOXOX - JRT - 2009.09.03 - This is where we move contacts when groups are deleted.


};

#endif	//OWCONTACTLIST_H
