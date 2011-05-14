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

#ifndef OWCCONTACTLIST_H
#define OWCCONTACTLIST_H

#include <control/Control.h>
#include <model/contactlist/EnumGroupType.h>	//VOXOX - JRT - 2009.05.07 JRT-GRPS
#include <model/contactlist/ContactList.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.07.21 

#include <util/StringList.h>
#include <util/Event.h>

#include <vector>
#include <string>

class Presentation;
class ContactList;
class ContactGroupSet;	//VOXOX - JRT - 2009.05.09 JRT-GRPS
class Contact;
class ContactGroup;
class ContactProfile;
class CWengoPhone;
class PContactList;
class IMContact;

class Telephones;			//VOXOX - JRT - 2009.07.26 
class EmailAddresses;		//VOXOX - JRT - 2009.07.26 

class WsContactProfile;		//VOXOX - JRT - 2009.05.26 
class CWsContactProfile;	//VOXOX - JRT - 2009.05.26 

//=============================================================================
//VOXOX - JRT - 2009.10.08 
//Keep track of any events/Threads that use this object so we can determine
//	when it is safe to destroy it without crashing.
//NOTE: We cannot rely on the objects Mutex since that does not provide a count of
//	threads waiting to use the object.  (If only it did!)
class ThreadCounter
{
public:
	ThreadCounter()
	{
		RecursiveMutex::ScopedLock lock(_mutex);
		_threadCount = 0;
	}

	virtual ~ThreadCounter()
	{
	}

	void operator++( int )	//Postfix
	{
		RecursiveMutex::ScopedLock lock(_mutex);
		_threadCount++;
	}

	void operator--( int )	//Postfix
	{
		RecursiveMutex::ScopedLock lock(_mutex);
		_threadCount--;
	}

	int getCount() const	{ return _threadCount;	}

	int _threadCount;
	mutable RecursiveMutex	_mutex;	
};

//=============================================================================

/**
 *
 * @ingroup control
 * @author Tanguy Krotoff
 */
class CContactList : public Control {
	friend class CUserProfile;
public:

	CContactList(ContactList & contactList, CWengoPhone & cWengoPhone);

	~CContactList();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	/**
	 * Gets a list of pair of <group UUID, group name>.
	 */
//	std::vector< std::pair<std::string, std::string> > getContactGroups() const;	//VOXOX - JRT - 2009.05.09 - Just reference the ContactGroupSet object.

	/**
	 * Gets the name of a ContactGroup given its UUID.
	 *
	 * @param groupId the UUID of the desired group
	 * @return the group name, a null string if not found
	 */
	std::string   getContactGroupName		   ( const std::string& groupId ) const;
	std::string   getContactGroupContactsOnline( const std::string& groupId ) const;
	ContactGroup* getContactGroup			   ( const std::string& groupId ) const;

	/**
	 * @see ContactList::getContactGroupIdFromName()
	 */
	std::string getContactGroupIdFromName(const std::string & groupName) const;

	/**
	 * Constructs a ContactProfile from a Contact contained in the model.
	 *
	 * If no Contact with the given contactId exists an empty ContactProfile
	 * is returned.
	 *
	 * Used by the GUI to get data from the model.
	 *
	 * @param contactId the Contact UUID od the desired Contact
	 * @return a ContactProfile based on the found Contact
	 */
	ContactProfile getContactProfile(const std::string & contactId) const;

	/**
	 * Constructs a ContactProfile from a Contact contained in the model.
	 *
	 * If no Contact with the given contactId exists an empty ContactProfile
	 * is returned.
	 *
	 * Used by the GUI to get data from the model.
	 *
	 * @param phoneNumber the Contact phoneNumber od the desired Contact
	 * @return a ContactProfile based on the found Contact
	 */
	ContactProfile getContactProfileByVoxOxPhoneNumber(const std::string & phoneNumber) const;	

	/**
	 * @see ContactList::findContactThatOwns()
	 */
	std::string findContactThatOwns(const IMContact & imContact);

	/**
	 * Gets all existing Contact Ids.
	 *
	 * @return a string list containing the UUID of the Contacts.
	 */
	StringList getContactIds() const;

	/**
	 * Adds a Contact to the ContactList.
	 *
	 * This method is used by GUI to add a Contact into the model.
	 *
	 * The Contact will be created from the given ContactProfile.
	 *
	 * @param contactProfile the ContactProfile that will be used to create the Contact
	 */
	void addContact(const ContactProfile & contactProfile);

	/**
	 * @see ContactList::removeContact()
	 */
	void removeContact(const std::string & contactId);

	/**VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	 * Block a contact
	 *
	 * @param contactId std string with the Contact ID to block
	 */
	void blockContact(const std::string & contactId);

	/**VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	 * Unblock a contact
	 *
	 * @param contactId std string with the Contact ID to unblock
	 */
	void unblockContact(const std::string & contactId);

	/**
	 * Updates a Contact from a ContactProfile.
	 *
	 * @param contactProfile the ContactProfile which will full the Contact
	 */
	void updateContact(const ContactProfile & contactProfile);	//VOXOX - JRT - 2009.05.25 TODO: make private so user must call CUserProfile.

//	void updateContactGroups( const ContactProfile & contactProfile, const ContactGroupInfoSet& oldGroups, const ContactGroupInfoSet& newGroups );	//VOXOX - JRT - 2009.09.23 
	void moveContact		( const ContactProfile & contactProfile, const std::string& oldGroupId, const std::string newGroupId );	//VOXOX - JRT - 2009.05.25 TODO: make private and add to CUserProfile.
	void moveContactToGroup ( const std::string& contactKey, const std::string& srcGroupId, const std::string& newGroupId, bool bIsGroupName ) ;	//VOXOX - JRT - 2009.08.09 - Multiple groups.

	/**
	 * Adds a contact group.
	 *
	 * @param name name of the ContactGroup to add
	 */
//	void addContactGroup(const std::string & name);
	void addContactGroup(const ContactGroup& group);

	/**
	 * @see ContactList::removeContactGroup()
	 */
	void removeContactGroup(const std::string & groupId);

	/**
	 * @see ContactList::renameContactGroup()
	 */
	void renameContactGroup(const std::string & groupId, const std::string & name);

	/**
	 * @see ContactList::mergeContacts()
	 */
	void merge(const std::string & dstContactId, const std::string & srcContactId);

	/**
	 * @see ContactList::orderGroups()
	 */
	void orderGroups(const std::string & dragGroupId, const std::string & dropGroupId);

	/**
	 * Returns a pointer to the Contact associated with the given phoneNumber
	 * or NULL if not found.
	 *
	 * Used by the Control to retrieve a Contact from a contact phoneNumber given by
	 * the GUI.
	 *
	 * @param phoneNumber the phone number of the desired Contact
	 * @return the found Contact
	 */
	Contact * getContactByNumber(const std::string & phoneNumber) const;//VOXOX CHANGE by Rolando - 2009.07.09 

	MessageTranslation getMessageTranslationSettings(const std::string & key);	
	/**
	 * FIXME: this part should not exist. It is here until QtChatRoomInviteDlg
	 * will be refactored.
	 */
	ContactList&	 getContactList()		{ return _contactList; }
	ContactGroupSet& getContactGroupSet();

	//VOXOX CHANGE CJC GET CONTACT BY ITS ID
	//VOXOX - JRT - 2009.04.18 - Made public
	Contact*	getContactByKey  (const std::string & key )		 const;
	Contact*	getContactById   (const std::string & contactId) const;
	Contact*	getContactByUuid (const std::string & uuid)		 const;	
//	std::string getContactGroupId(const std::string& contactId ) const;	//VOXOX - JRT - 2009.08.07 - Not used and may cause issues with multiple groups.

	void populateFilterGroup		  ( const std::string& text, const std::string& filterName );
	void populateFilterGroupThreadSafe( const std::string text,  const std::string& filterName );
	QtEnumIMProtocol::IMProtocol getContactQtProtocol		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.21 
	
	std::string					 getContactDisplayName		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	std::string					 getContactBestShortDisplayName( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	std::string					 getContactIconData			   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	std::string					 getContactSocialAddress	   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	std::string					 getContactMobileNumber		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	std::string					 getContactFirstWengoId		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	
	bool						 isContactOnline			   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	bool						 canCallContact				   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	bool						 canContactReceiveSMS		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	bool						 canContactReceiveFax		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	bool						 canContactReceiveEmail		   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 
	bool						 canContactDoFileTransfer	   ( const std::string& key ) const;					//VOXOX - JRT - 2009.07.26 

	bool						 addContactPhones			   ( const std::string& key, Telephones&     phones );	//VOXOX - JRT - 2009.07.26 
	bool						 addContactEmailAddresses	   ( const std::string& key, EmailAddresses& emails );	//VOXOX - JRT - 2009.07.26 

	//VOXOX - JRT - 2009.07.27 - Menu oriented methods
	void						 getContactsWithHomeNumber  ( ContactInfoList2& info );
	void						 getContactsWithMobileNumber( ContactInfoList2& info );
	void						 getContactsWithSipNumber   ( ContactInfoList2& info );
	void						 getContactsWithWengoCall   ( ContactInfoList2& info );
	void						 getContactsThatCanChat     ( ContactInfoList2& info );

	int							getHowManyContactsMustMove( const std::string & groupId );	//VOXOX - JRT - 2009.09.03 

	bool						isFiltering() 						{ return getContactList().isFiltering(); }		//VOXOX - JRT - 2009.09.07 
	bool						isDestroying()						{ return _destroying;					 }		//VOXOX - JRT - 2009.09.22 
	bool						addMessageTranslationSettings	   ( const std::string& key,const MessageTranslation & messageTranslation );	//VOXOX - JRT - 2009.07.26 

private:
	void  ensureGroup( ContactProfile& contactProfile );

	void initPresentationThreadSafe();
	void uninitPresentationThreadSafe();				//VOXOX - JRT - 2009.10.07 

	PContactList* getPContactList() const		{ return _pContactList;	}	//VOXOX - JRT - 2009.10.08 

	/**
	 * Returns a pointer to the Contact associated with the given contactId
	 * or NULL if not found.
	 *
	 * Used by the Control to retrieve a Contact grom a contact UUID given by
	 * the GUI.
	 *
	 * @param contactId the UUID of the desired Contact
	 * @return the found Contact
	 */
	Contact * getContact(const std::string & contactId) const;

	/**
	 * Returns a pointer to the Contact associated with the given phoneNumber
	 * or NULL if not found.
	 *
	 * Used by the Control to retrieve a Contact from a contact phoneNumber given by
	 * the GUI.
	 *
	 * @param phoneNumber the phone number of the desired Contact
	 * @return the found Contact
	 */
	Contact * getContactByVoxOxPhoneNumber(const std::string & phoneNumber) const;	

	/**
	 * @see ContactList::contactAddedEvent
	 */
	void contactAddedEventHandler(ContactList & sender, Contact & contact);

	void contactAddedEventHandlerThreadSafe(std::string contactId, std::string groupId);	//VOXOX - JRT - 2009.08.09 - Multiple groups

	/**
	 * @see ContactList::contactRemovedEvent
	 */
	void contactRemovedEventHandler(ContactList & sender, std::string& contactId );

	void contactRemovedEventHandlerThreadSafe(std::string contactId );

	/**
	 * @see ContactList::contactGroupAddedEvent
	 */
	void contactGroupAddedEventHandler(ContactList & sender, ContactGroup & contactGroup);

	void contactGroupAddedEventHandlerThreadSafe(std::string groupId);

	/**
	 * @see ContactList::contactGroupRemovedEvent
	 */
	void contactGroupRemovedEventHandler(ContactList & sender, ContactGroup & contactGroup);

	void contactGroupRemovedEventHandlerThreadSafe(std::string groupId);

	/**
	 * @see ContactList::contactGroupRenamedEvent
	 */
	void contactGroupRenamedEventHandler(ContactList & sender, ContactGroup & contactGroup);

	void contactGroupRenamedEventHandlerThreadSafe(std::string groupId);

	//VOXOX - JRT - 2009.05.11 
	void contactGroupsReorderedEventHandler(ContactList & sender);
	void contactGroupsReorderedEventHandlerThreadSafe();

	/**
	 * @see ContactList::contactGroupMovedEvent
	 */
	void contactMovedEventHandler(ContactList & sender, ContactGroup & dstContactGroup,
		ContactGroup & srcContactGroup, Contact & contact);

	void contactMovedEventHandlerThreadSafe(std::string dstGroupId, std::string srcGroupId, std::string contactId);

	//VOXOX - JRT - 2009.04.12 
	Event<void (ContactList & sender, int nOnOff)> setInitialLoadEvent;
	void setInitialLoadEventHandler(ContactList & sender, int nOnOff);

	void setInitialLoadEventHandlerThreadSafe( int nOnOff );
	//End VoxOx

	//VOXOX - JRT - 2009.05.26 
	CWsContactProfile*	_cWsContactProfile;

	void createWsContactProfile();
	void wsContactProfileCreatedEventHandler( ContactList& sender, WsContactProfile& wsContactProfile );

	void saveContactProfile          ( const ContactProfile& profile );
	void saveContactProfileThreadSafe( const ContactProfile& profile );
	//End VoxOx


	/**
	 * @see ContactList::contactChangedEvent
	 */
	void contactChangedEventHandler(ContactList & sender, Contact & contact);

	void contactChangedEventHandlerThreadSafe(std::string contactId, std::string groupId);	//VOXOX - JRT - 2009.08.09 - Multiple groups.

	/**
	 * @see addContact
	 */
	void addContactThreadSafe(ContactProfile contactProfile);

	/**
	 * @see removeContact
	 */
	void removeContactThreadSafe(std::string contactId);

	/**VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	 * @see blockContact
	 */
	void blockContactThreadSafe(std::string contactId);

	/**VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	 * @see unblockContact
	 */
	void unblockContactThreadSafe(std::string contactId);

	/**
	 * @see updateContact
	 */
	void updateContactThreadSafe(ContactProfile contactProfile);

	/**
	 * @see addContactGroup
	 */
//	void addContactGroupThreadSafe(std::string name);
	void addContactGroupThreadSafe( const ContactGroup& group );	//VOXOX - JRT - 2009.05.07 JRT-GRPS

	/**
	 * @see removeContactGroup
	 */
	void removeContactGroupThreadSafe(std::string groupId);

	/**
	 * @see renameContactGroup
	 */
	void renameContactGroupThreadSafe(std::string groupId, std::string name);

	/**
	 * @see merge
	 */
	void mergeThreadSafe(std::string dstContactId, std::string srcContactId);

	/**
	 * @see orderGroups
	 */
	void orderGroupsThreadSafe( std::string dstContactId, std::string srcContactId);


	void waitForOkToDestroy( int maxWait );	//VOXOX - JRT - 2009.10.07 

	/** Direct link to the model. */
	ContactList & _contactList;

	/** Direct link to the presentation via an interface. */
	PContactList * _pContactList;

	CWengoPhone & _cWengoPhone;

	mutable RecursiveMutex	_mutex;				//VOXOX - JRT - 2009.09.22 
	bool					_destroying;		//VOXOX - JRT - 2009.09.22 

	ThreadCounter			_threadCounter;		//VOXOX - JRT - 2009.10.08 
};

#endif	//OWCCONTACTLIST_H
