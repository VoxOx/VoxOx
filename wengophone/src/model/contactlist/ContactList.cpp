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
#include "ContactList.h"

#include "IMContactListHandler.h"

#include <model/presence/PresenceHandler.h>
#include <model/profile/UserProfile.h>
#include <model/webservices/contactIntegration/WsContactProfile.h>	//VOXOX - JRT - 2009.05.26 

#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 
#include <thread/Thread.h>

#define LOGGER_COMPONENT "ContactList"
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>
#include <util/Uuid.h>

using namespace std;


std::string ContactList::_defaultMoveToGroupName = "Other";	//VOXOX - JRT - 2009.09.03 - This is where we move contacts when groups are deleted.

//=============================================================================
//VOXOX - JRT - 2009.04.07 - Implement a map based Contacts for better performance
//							 and consistence interface.
//=============================================================================

Contacts::Contacts()
{
}

//-----------------------------------------------------------------------------

Contacts::~Contacts()
{
}

//-----------------------------------------------------------------------------
	
Contact* Contacts::FindByImContact( const IMContact& imContact )
{
	return Lookup( imContact.getKey() );	//VOXOX - JRT - 2009.04.20 
}

//-----------------------------------------------------------------------------

Contact* Contacts::FindByUuid( const std::string& uuid )
{
	//VOXOX - JRT - 2009.04.21 - We have an issue with UUIDs being changed
	//		after Contact is added to ContactList.  For now, we will just iterate.
	Contact* pRet = NULL;

	for ( Contacts::const_iterator it = begin(); it != end(); ++it) 
	{
		if ((*it).second.getUUID() == uuid ) 
		{
			pRet = const_cast<Contact*>(&(*it).second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------
//We appear to have a problem with nomenclature.  UUID != ContactID  JRT - 2009.04.15
Contact* Contacts::FindByContactId( const std::string& contactId )
{
	return Lookup( contactId );
}

//-----------------------------------------------------------------------------

Contact* Contacts::FindByContactIdUserNetwork( const std::string& contactId, int userNetworkId )
{
	Contact* pRet = NULL;
	IMContact* imContact = NULL;

	for ( Contacts::const_iterator it = begin(); it != end(); ++it) 
	{
		imContact = (*it).second.getPreferredIMContact();

		if ( imContact && (imContact->getUserNetworkId() == userNetworkId) ) 
		{
			if ( (*it).second.getContactId() == contactId )		//NOT the key.
			{
				pRet = const_cast<Contact*>(&(*it).second);
				break;
			}
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

Contact* Contacts::FindByJSONId( int JsonId )
{
	Contact* pRet = NULL;

	for ( Contacts::const_iterator it = begin(); it != end(); ++it) 
	{
		if ( (*it).second.getId() == JsonId )
		{
			pRet = const_cast<Contact*>(&(*it).second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando 04-27-09 - Used to get Contact by phonenumber
Contact* Contacts::FindByVoxOxPhoneNumber( const std::string& phoneNumber )
{
	Contact* pRet = NULL;

	for ( Contacts::iterator it = begin(); it != end(); ++it) 
	{
		if ((*it).second.getVoxOxPhone() == phoneNumber ) 
		{
			pRet = const_cast<Contact*>(&(*it).second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------
//VOXOX - CJC - 2009.06.14 
Contact* Contacts::FindByEmail( const std::string& email )
{
	Contact* pRet = NULL;

	for ( Contacts::iterator it = begin(); it != end(); ++it) 
	{
			//VOXOX - CJC - 2009.06.11 I guess this will be pretty slow...
			EmailAddresses& rEmail = (*it).second.getEmailAddresses();

			if(rEmail.hasValid()){
			
				for ( EmailAddresses::iterator it2 = rEmail.begin(); it2 != rEmail.end(); it2++ )
				{
					if ( (*it2).isValid() )
					{
						std::string emailAddress = (*it2).getAddress();
						if(email== emailAddress){
							return pRet = const_cast<Contact*>(&(*it).second);
						}
					}
				}
			}
	}
	return pRet;

}

//VOXOX - CJC - 2009.06.14 
Contact* Contacts::FindByPhoneNumber( const std::string& number )
{
	Contact* pRet = NULL;

	for ( Contacts::iterator it = begin(); it != end(); ++it) 
	{
			//VOXOX - CJC - 2009.06.11 I guess this will be pretty slow...
			Telephones & rTelephone = (*it).second.getTelephones();

			if(rTelephone.hasValid()){
			
				for ( Telephones::iterator it2 = rTelephone.begin(); it2 != rTelephone.end(); it2++ )
				{
					if ( (*it2).isValid() )
					{
						//Lets try to macth the number as is
						std::string realNumber = number;
						std::string contactNumber = (*it2).getNumber();
						if(contactNumber== realNumber){
							return pRet = const_cast<Contact*>(&(*it).second);
						}else{
							//Get the last 7 digits and match it
							contactNumber = contactNumber.replace(0,contactNumber.length()-7,"");
							realNumber = realNumber.replace(0,realNumber.length()-7,"");
							if(contactNumber== realNumber){
								return pRet = const_cast<Contact*>(&(*it).second);
							}
						}
					}
				}
			}
	}
	return pRet;

}


//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.04.26 - NOTE! this 'find' method has a signficant side-effect.
//	The checkAndSetIMContact() method UPDATES IMContact, which could alter the map key!

//VOXOX - JRT - 2009.04.26 - This was being causing checkAndSetIMContact to be called 6 MILLION
//		times on start up for an account with 1178 contacts.
//Contact* Contacts::FindByImContact( const IMContact& imContact )
//{
//	Contact* pRet = NULL;
//
//	//NOTE - using C-style casts (Contact&) causes problems!  Avoid them, especially on iterators.
//	for (Contacts::iterator it = begin(); it != end(); ++it)	//Removed const_iterator
//	{
//		if ( ((*it).second).checkAndSetIMContact( imContact ) )
//		{
//			pRet = &(*it).second;
//			break;
//		}
//	}
//
//	return pRet;
//}

//-----------------------------------------------------------------------------
	
//VOXOX - JRT - 2009.06.24 - TODO: If this a REMOVE, what is being removed?
void Contacts::RemoveImAccount( const std::string imAccountId )	//VOXOX - JRT - 2009.05.05 - TODO: Review this.  It looks inefficient.
{																//May be faster if caller passes in ImAccount.
	for (Contacts::const_iterator it = begin(); it != end(); ++it) 
	{
		for (IMContactSet::const_iterator imContactIt = const_cast<Contact&>((*it).second).getIMContactSet().begin();	//Removed const_iterator
			imContactIt != const_cast<Contact&>((*it).second).getIMContactSet().end(); ++imContactIt) 
		{
			if (((*imContactIt).getIMAccountId()) == imAccountId) 
			{
				((IMContact &)*imContactIt).setIMAccount(NULL);
			}
		}
	}
}

//End VOXOX
//=============================================================================

ContactList::ContactList(UserProfile & userProfile)
	: _userProfile(userProfile),
	_imContactListHandler(userProfile.getIMContactListHandler()) 
{

	_imContactListHandler.newIMContactAddedEvent				+= boost::bind(&ContactList::newIMContactAddedEventHandler,    this, _1, _2, _3);
	_imContactListHandler.imContactRemovedEvent					+= boost::bind(&ContactList::imContactRemovedEventHandler,     this, _1, _2, _3);
//	_imContactListHandler.newContactGroupAddedEvent				+= boost::bind(&ContactList::newContactGroupAddedEventHandler, this, _1, _2 );
	_imContactListHandler.newContactGroupAdded2Event			+= boost::bind(&ContactList::newContactGroupAddedEventHandler, this, _1, _2);	//JRT-GRPS
	_imContactListHandler.contactGroupRemovedEvent				+= boost::bind(&ContactList::contactGroupRemovedEventHandler,  this, _1, _2);
	_imContactListHandler.imContactMovedEvent					+= boost::bind(&ContactList::imContactMovedEventHandler,	   this, _1, _2, _3);

	//VOXOX - JRT - 2009.04.12 
	_imContactListHandler.setInitialLoadEvent					+= boost::bind(&ContactList::setInitialLoadEventHandler,	   this, _1, _2);
	//End VoxOx

	_userProfile.getPresenceHandler().presenceStateChangedEvent += boost::bind(&ContactList::presenceStateChangedEventHandler, this, _1, _2, _3, _4, _5);
	_userProfile.getPresenceHandler().contactIconChangedEvent   += boost::bind(&ContactList::contactIconChangedEventHandler,   this, _1, _2, _3);
	_userProfile.getIMAccountManager().imAccountRemovedEvent	+= boost::bind(&ContactList::imAccountRemovedEventHandler,     this, _1, _2);

	_wsContactProfile = NULL;
	_filterGroupText  = "";
	_nextId			  = 0;
//	createWsContactProfile();	//VOXOX - JRT - 2009.05.26  - Call from CContactList due to WengoAccount initialization.
}

ContactList::~ContactList() 
{
	OWSAFE_DELETE( _wsContactProfile );
}

void ContactList::addContactGroup(const ContactGroup& group) 	//VOXOX - JRT - 2009.05.07 JRT-GRPS
{
	_addContactGroup(group);		//VOXOX - JRT - 2009.05.07 JRT-GRPS

	if ( group.isUser() )			//VOXOX - JRT - 2009.08.11 
	{
		_imContactListHandler.addGroup(group.getName());	//JRT-GRPS
	}
}

void ContactList::removeContactGroup(const string & groupId) 
{
	ContactGroup * contactGroup = getContactGroup(groupId);
	if (contactGroup) 
	{
		string groupName = contactGroup->getName();
		_removeContactGroup(groupId);
		_imContactListHandler.removeGroup(groupName);
	}
}

void ContactList::renameContactGroup(const std::string & groupId, const std::string & name) {
	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 

	if (!name.empty()) 
	{
		ContactGroup * contactGroup = getContactGroup(groupId);

		if (contactGroup) 
		{
			std::string oldName = contactGroup->getName();
			contactGroup->setName(name);
			_imContactListHandler.changeGroupName(oldName, name);
			contactGroupRenamedEvent(*this, *contactGroup);
		}
	}
}

void ContactList::setInitialLoadEventHandler(IMContactListHandler & sender, int nOnOff)
{
	setInitialLoad( nOnOff );
}

void ContactList::setInitialLoad( int nOnOff )
{
	setInitialLoadEvent( *this, nOnOff );
}

Contact* ContactList::createContact() 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact* pContact = new Contact( _userProfile );

	return pContact;
}

Contact* ContactList::addContact( Contact* contactIn )
{
	Contact* result = NULL;

	if ( contactIn->getGroups().GetCount() == 1 )
	{
		result = addContactPrivate( contactIn );
	}
	else
	{
		//VOXOX - JRT - 2009.08.10 - Multiple Group support
		Groups& groups = contactIn->getGroups();

		for ( Groups::iterator it = groups.begin(); it != groups.end(); it++ )
		{
			//Clone original contact
			Contact* newContact = new Contact(*contactIn);

			//Clear groups and add current Groups iterator.
			newContact->getGroups().DeleteAll();
			newContact->getGroups().Add( &(*it) );

			//Add Contact
			result = addContact( newContact );
		}

		//We must delete this since it is not passed to addContact.
		delete contactIn;		
		result = NULL;			//VOXOX - JRT - 2009.08.10 - We force this to null because caller should be providing a single Group if result is to be valid.
	}

	return result;
}

Contact* ContactList::addContactPrivate( Contact* contactIn )
{
	Contact* pContact = NULL;
	if ( contactIn )
	{
		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

		std::string contactId = contactIn->getContactId();	//OK  use of getContactId().

		contactIn->setUUID( Uuid::generateString() );	//VOXOX - JRT - 2009.04.21 - We set this only when adding to a list.
		contactIn->setId( ++_nextId );					//VOXOX - JRT - 2009.06.28 
		_contacts.Add(*contactIn);						//Copies

		pContact = _contacts.Lookup( contactIn->getKey() );

		if ( pContact )
		{
//			_addToContactGroup( pContact->getGroups().getFirst(), *pContact);
			addToContactGroups( pContact );				//VOXOX - JRT - 2009.05.09 JRT-GRPS
			pContact->contactChangedEvent += boost::bind(&ContactList::contactChangedEventHandler, this, _1);
			contactAddedEvent(*this, *pContact);
		}

		delete contactIn;
	}

	return pContact;
}

void ContactList::addToContactGroups( Contact* pContact )
{
	IMContact* imContact = pContact->getPreferredIMContact();

	//Add user group(s)
	std::string groupName = pContact->getGroups().getFirst();
	if ( groupName.empty() )	//VOXOX - JRT - 2009.05.09 - We may get this with bad serialization.
	{
		groupName = "Buddies";
	}

	ContactGroup contactGroup( groupName, EnumGroupType::GroupType_User);	
	_addToContactGroup( contactGroup, *pContact);					//VOXOX - JRT - 2009.05.07 JRT-GRPS
	LOG_DEBUG("IMContact added in group " + contactGroup.getName() + ": " + imContact->getContactId());

	addToNonUserGroups( pContact );
}

void ContactList::addToNonUserGroups( Contact* pContact )
{
	IMContact* imContact = pContact->getPreferredIMContact();

	//Add Network group.
	ContactGroup group2( pContact->getNetworkName(), EnumGroupType::GroupType_OrigNetwork);
	_addToContactGroup( group2, *pContact);	//VOXOX - JRT - 2009.05.07 JRT-GRPS

	LOG_DEBUG("IMContact added in group " + group2.getName() + ": " + imContact->getContactId());
}

void ContactList::removeContact(Contact & contact) {
	RecursiveMutex::ScopedLock lock(_mutex);

	//Remove the Contact from its group(s)
	//VOXOX - JRT - 2009.05.06 - JRT-GRPS
//	ContactGroup * contactGroup = getContactGroup(contact.getGroupId());
//	if (contactGroup) {
//		contactGroup->removeContact(contact);
//	}
	removeContactFromAllGroups( contact );
	//End JRT-GRPS

	//Remove all IMContacts
	std::string strKey = contact.getKey();		//VOXOX - JRT - 2009.04.28 - Needed to remove contact from _contacts.

	//We get a copy of the IMContactSet as it will modified while browsing it
	IMContactSet imContactSet = contact.getIMContactSet();

	for (IMContactSet::const_iterator it = imContactSet.begin();
		it != imContactSet.end(); ++it) {
		removeIMContact(contact, *contact.getIMContact(*it));
	}
	////

	//Remove the Contact from the ContactList
//VOXOX - JRT - 2009.04.07 - Use new map-based class
	Contact* pContact = _contacts.Lookup( strKey );	
	if ( pContact )
	{
		 _contacts.Delete( strKey );
//		contactRemovedEvent(*this, *pContact );
		contactRemovedEvent(*this, strKey );	//VOXOX - JRT - 2009.04.28 
	}
	//End Voxox
	/////
}

void ContactList::removeContactFromAllGroups( Contact& contact )
{
	ContactGroupInfoSet& infoSet = contact.getContactGroupInfoSet();
	infoSet.lock();	//VOXOX - JRT - 2009.06.24 

	for ( ContactGroupInfoSet::const_iterator it = infoSet.begin(); it != infoSet.end(); it++ )
	{
		ContactGroup* pGroup = getContactGroup( (*it).second.getKey() );

		if (pGroup) {

			pGroup->removeContact(contact);
		}
	}

	infoSet.unlock();	//VOXOX - JRT - 2009.06.24 
}

void ContactList::addIMContact(Contact & contact, const IMContact & imContact) {
	RecursiveMutex::ScopedLock lock(_mutex);

	ContactGroup * contactGroup = getContactGroup(contact.getFirstUserGroupId() );	//VOXOX - JRT - 2009.05.06 - JRT-GRPS OK

	if (contactGroup) {
		contact._addIMContact(imContact);
		_imContactListHandler.addIMContact(contactGroup->getName(), imContact);
	}
}

void ContactList::removeIMContact(Contact & contact, const IMContact & imContact) {
	RecursiveMutex::ScopedLock lock(_mutex);

//	ContactGroup * contactGroup = getContactGroup(contact.getGroupId());
	ContactGroup * contactGroup = getContactGroup(contact.getFirstUserGroupId() );	//VOXOX - JRT - 2009.05.06 - JRT-GRPS

	if (contactGroup) {
		_imContactListHandler.removeIMContact(contactGroup->getName(), imContact);
		contact._removeIMContact(imContact);
	}
}

void ContactList::newIMContactAddedEventHandler(IMContactListHandler & sender,
	const std::string & groupName, IMContact & newIMContact) {

	RecursiveMutex::ScopedLock lock(_mutex);

	if (!groupName.empty()) 
	{
		std::string requestContactId = newIMContact.getContactId();
		int			requestTimestamp = 0;
		int			userNetworkId    = 0;

		LOG_DEBUG("adding a new IMContact in group " + groupName + ": " + newIMContact.getContactId());

		//Find the Contact that owns the IMContact. Creating a new one if needed
		Contact * contact = findContactThatOwns(newIMContact);
		if (!contact) 
		{
			LOG_DEBUG("IMContact " + newIMContact.getContactId() + " not found. Adding a new Contact");
			contact = createContact();	
		}

		requestTimestamp = contact->getTimestamp();	//VOXOX - JRT - 2009.06.03 
		userNetworkId    = getImContactUserNetworkId( newIMContact );

		if (!contact->hasIMContact(newIMContact)) 
		{
			contact->_addIMContact(newIMContact);

			//Add group info to Contact so addToContactGroups can find it.
			//NOTE: Group is NOT the same as ContactGroup.  Group is merely a means to associate basic group info with a contact.
			Group grp( groupName, EnumGroupType::GroupType_User);
			contact->getGroups().Add( &grp );

			Contact* pContact = addContact(contact);
			//NOTE: use pContact after this point since 'contact' has been deleted.
		} 
		else 
		{
			//This event can be received although the IMContact is already present
			//in the ContactList. We assume that this is a move event.
			std::string srcGroupId = contact->getFirstUserGroupId();	//VOXOX - JRT - 2009.08.09 - Multiple groups.
			_moveContactToGroup( srcGroupId, groupName, *contact);
		}

		//VOXOX - JRT - 2009.05.25 - Since we should only be in this method if we are loading contacts from network (vs. cache)
		//	it is correct to requestContactProfile() in all cases.
//		LOG_INFO(" Request profile for " + newIMContact.getKey() );
		requestContactProfile( requestContactId, userNetworkId, requestTimestamp );
	}
}

void ContactList::imContactRemovedEventHandler(IMContactListHandler & sender,
	const std::string & groupName, IMContact & imContact) {

	RecursiveMutex::ScopedLock lock(_mutex);

	LOG_DEBUG("IMContact removed from group " + groupName + ": "
		+ imContact.getContactId());

	//Find the Contact that owns the IMContact. Creating a new one if needed
	Contact * contact = findContactThatOwns(imContact);
	if (!contact) {
//		contact = &(createContact());
		contact = createContact();	//VOXOX - JRT - 2009.04.26 
	}

	contact->_removeIMContact(imContact);

	LOG_DEBUG("IMContact removed: " + imContact.getContactId());
}

//void ContactList::newContactGroupAddedEventHandler(IMContactList & sender, const std::string & groupName ) {	
void ContactList::newContactGroupAddedEventHandler(IMContactList & sender, const ContactGroup& group ) {	//VOXOX - JRT - 2009.05.07 JRT-GRPS
//	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 - Locking move to ContactGroup class

//	_addContactGroup(groupName);
	_addContactGroup( group );	//VOXOX - JRT - 2009.05.07 JRT-GRPS
}

void ContactList::contactGroupRemovedEventHandler(IMContactList & sender, const std::string & groupName) {
//	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 - Locking move to ContactGroup class

	_removeContactGroup(groupName);
}

void ContactList::presenceStateChangedEventHandler(	PresenceHandler & sender, EnumPresenceState::PresenceState state,
													const std::string & alias, const std::string & statusMessage, const IMContact & imContact) 
{

	RecursiveMutex::ScopedLock lock(_mutex);

	//Find the Contact that owns the IMContact. Creating a new one if needed
	Contact * contact = findContactThatOwns(imContact);
	if (contact) 
	{
		//The PresenceState must not be changed if the PresenceState is UserDefined (used by PhApi to set the alias)
		if (!contact->getIMContact(imContact)) 
		{
			LOG_FATAL("bad algorithm in findContactThatOwns or in getIMContact");
		}

		if (state != EnumPresenceState::PresenceStateUserDefined) 
		{
//			contact->getIMContact(imContact)->setPresenceState(state);	//VOXOX - JRT - 2009.04.28 - intermittent crash when removing contact.
			IMContact* temp = contact->getIMContact(imContact);
			if ( temp )
			{
				temp->setPresenceState(state);

				IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imContact.getIMAccountId());
				if (imAccount->getProtocol() != EnumIMProtocol::IMProtocolWengo) 
				{
//					contact->getIMContact(imContact)->setAlias(alias);
//					contact->getIMContact(imContact)->setStatusMessage(statusMessage);
					temp->setAlias(alias);
					temp->setStatusMessage(statusMessage);
				}
		
				OWSAFE_DELETE(imAccount);
			}
		} 
		else 
		{
			contact->getIMContact(imContact)->setPresenceState(EnumPresenceState::PresenceStateOnline);
			contact->getIMContact(imContact)->setAlias(alias);
			contact->getIMContact(imContact)->setStatusMessage(statusMessage);
		}
	}
}

void ContactList::imContactMovedEventHandler(IMContactListHandler & sender, const std::string & groupName, IMContact & imContact) 
{
	//VOXOX - JRT - 2009.08.07 - We only get here from IMContactListHandler due to new IM contact.
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact * contact = findContactThatOwns(imContact);
		
	//VOXOX - JRT - 2009.08.07 - Modify to support multiple groups from other networks.
	if (contact) 
	{
		ContactGroup cg( groupName, EnumGroupType::GroupType_User );	//VOXOX - JRT - 2009.08.07 

		//This method can be called when a Contact has changed, not only because the Contact has moved.
//		ContactGroup * contactGroup = getContactGroup(contact->getFirstUserGroupId() );	//VOXOX - JRT - 2009.05.06 - JRT-GRPS
//		if (contactGroup && (contactGroup->getName() == groupName))		//VOXOX - JRT - 2009.08.07 
		if ( contact->isInGroup( cg.getKey() ) )
		{
			contactChangedEvent(*this, *contact);
		} 
		else 
		{
//			_moveContactToGroup(groupName, *contact);		//VOXOX - JRT - 2009.08.07 
			_addToContactGroup( cg, *contact );	
		}
	}
}

Contact * ContactList::findContactThatOwns(const IMContact & imContact) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	//VOXOX - JRT - 2009.04.26 - FindByImContact() has a side-effect which we must tend to with
	//	our code replacement.
//	return _contacts.FindByImContact( imContact );		//VOXOX - JRT - 2009.04.25 
//	std::string contactKey = _imAccountContactMap.FindByImContact( imContact );
//	Contact* pRet = _contacts.FindByUuid( contactKey );
//	Contact* pRet = _contacts.FindByContactId( imContact.getContactId() );
	Contact* pRet = _contacts.FindByImContact( imContact );

	if ( pRet )
	{
		bool bSuccess = pRet->checkAndSetIMContact( imContact );

		if ( !bSuccess )
			int xxx = 1;

	}
	else
	{
		int xxx = 1;
	}

	return pRet;
}

//void ContactList::_addContactGroup(const std::string & groupName) {
void ContactList::_addContactGroup(const ContactGroup& group) {	//VOXOX - JRT - 2009.05.07 JRT-GRPS
//	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 

	bool bAdded = false;
	ContactGroup* pNewGroup = _contactGroupSet.Add1( group, &bAdded );	//VOXOX - JRT - 2009.05.05 

	//If the ContactGroup was actually inserted (e.g did not exist)
	if ( bAdded ) 
	{
		contactGroupAddedEvent (*this, *pNewGroup );					//VOXOX - JRT - 2009.05.05 
		LOG_DEBUG("new ContactGroup added " + group.getName());
	} 
	else 
	{
		LOG_DEBUG("this ContactGroup already exists: " + group.getName());
	}
}

void ContactList::_removeContactGroup(const std::string & groupId) 
{
	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 

	//VOXOX - JRT - 2009.06.23 - TODO: move this logic to ContactGroup class.
	ContactGroup * contactGroup = getContactGroup(groupId);

	if (contactGroup) 
	{
		//Deleting every Contact in the ContactGroup
		//VOXOX - JRT - 2009.05.30 - operator= does not copy the _contactList, so it appears this never worked.
		ContactGroup contactGroupCopy = *contactGroup;		//But we still need for later in method.

		//VOXOX - JRT - 2009.09.03 - Because we may be removing entries from the ContactGroup::contactList, we need to
		//							 restart the iterator after each call to avoid crash due to corrupt iterator.
		bool bContinue = true;

		while ( bContinue )
		{
			bContinue = false;

			for (ContactGroup::ContactVector::const_iterator vectIt = contactGroup->_contactList.begin(); vectIt != contactGroup->_contactList.end(); ++vectIt) 
			{
				bool moveToOther = false;

				//VOXOX - JRT - 2009.09.03 - If Contact is in only one user group, move it to the "Other" group.
				if ( contactGroup->isUser() )
				{
					if ( (*vectIt)->getUserGroupCount() <= 1 )
					{
						moveToOther = (*vectIt)->isInGroup( contactGroup->getKey() );
					}
				}
				
				//Do the move or remove.
				if ( moveToOther )
				{
					_moveContactToGroup( contactGroup->getName(), _defaultMoveToGroupName, *(*vectIt));
					bContinue = true;
					break;
				}
				else		//Otherwise, just remove it from this user group.
				{
					(*vectIt)->removeFromGroup( groupId );		//VOXOX - JRT - 2009.05.30 
				}
			}
		}

		//Deleting the group
		_contactGroupSet.Delete( contactGroupCopy );			//VOXOX - JRT - 2009.05.05 
		contactGroupRemovedEvent(*this, contactGroupCopy);

		LOG_DEBUG("ContactGroup removed. Key: " + groupId);		//VOXOX - JRT - 2009.05.05 
	}
}

void ContactList::_addToContactGroup(const ContactGroup& group, Contact & contact) 	//VOXOX - JRT - 2009.05.07 JRT-GRPS
{
	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 

	if (!group.getName().empty())	//JRT-GRPS
	{
		addContactGroup(group);	//JRT-GRPS

		ContactGroup* pGroup = _contactGroupSet.FindByGroup( group );				//JRT-GRPS

		if ( pGroup )
		{
			LOG_DEBUG("adding a Contact to group " + group.getName());

			pGroup->addContact(contact);				//VOXOX - JRT - 2009.05.05 
			contact.addToGroup( *pGroup );				//VOXOX - JRT - 2009.05.06 JRT-GRPS
		} 
		else 
		{
			LOG_FATAL("the group " + group.getName() + " has not been added");
		}
	}
}

void ContactList::imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	_contacts.RemoveImAccount( imAccountId );
}

ContactGroup * ContactList::getContactGroup(const std::string & groupId)  	//VOXOX - JRT - 2009.05.05 - Removed const
{
//	RecursiveMutex::ScopedLock lock(_mutexContactGroup);	//VOXOX - JRT - 2009.06.22 - FindByKey() does locking.

	return _contactGroupSet.FindByKey( groupId );	//VOXOX - JRT - 2009.05.05 JRT-GRPS
}

//void ContactList::contactIconChangedEventHandler(PresenceHandler & sender, const IMContact & imContact, OWPicture icon)
void ContactList::contactIconChangedEventHandler(PresenceHandler & sender, const IMContact & imContact, const OWPicture& icon)	//VOXOX - JRT - 2009.08.18 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact * contact = findContactThatOwns(imContact);
	if (contact) 
	{
		contact->getIMContact(imContact)->setIcon(icon);
	} 
	else 
	{
		LOG_DEBUG("IMContact not found: " + imContact.getContactId());
	}
}

void ContactList::mergeContacts(Contact & dst, Contact & src) {
	RecursiveMutex::ScopedLock lock(_mutex);

	dst.merge(src);

	//Remove the source Contact without removing it from linked IMContactLists
	//Remove the Contact from its group
//	ContactGroup * contactGroup = getContactGroup(src.getGroupId());
//	if (contactGroup) {
//		contactGroup->removeContact(src);
//	}
	removeContactFromAllGroups( src );		//VOXOX - JRT - 2009.05.06 JRT-GRPS
	////

	//VOXOX - JRT - 2009.04.07 - Use new map class
	Contact* pContact = _contacts.Lookup( src.getKey() );
	if ( pContact )
	{
		std::string	strKey = pContact->getKey();
//		contactRemovedEvent(*this, *pContact);
		contactRemovedEvent(*this, strKey );	//VOXOX - JRT - 2009.04.28 
		_contacts.Delete( pContact->getKey() );
	}
	//End Voxox
}

//VOXOX - JRT - 2009.05.11 	
void ContactList::orderGroups( const std::string& dragGroupId, const std::string dropGroupId )
{
	getContactGroupSetRef().changeOrder( dragGroupId, dropGroupId );
	contactGroupsReorderedEvent(*this);
}
//End VoxOx

void ContactList::_moveContactToGroup( const std::string& srcGroupName, const std::string & destGroupName, Contact & contact) 	//JRT-GRPS - need type.  Only allow DND among user groups.
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool bMove = true;
//	ContactGroup* origGroup = getContactGroup(contact.getFirstUserGroupId());	//VOXOX - JRT - 2009.05.05 - JRT-GRPS
	ContactGroup* origGroup = _contactGroupSet.FindByGroup( ContactGroup(srcGroupName,  EnumGroupType::GroupType_User) );				//VOXOX - JRT - 2009.08.09 - Multiple groups
	ContactGroup* destGroup = _contactGroupSet.FindByGroup( ContactGroup(destGroupName, EnumGroupType::GroupType_User) );

	//Add group if it does not exist.
	if ( destGroup == NULL )
	{
		ContactGroup contactGroup(destGroupName, EnumGroupType::GroupType_User);	//VOXOX - JRT - 2009.05.07 JRT-GRPS TODO: may need type parameter.
		addContactGroup( contactGroup );	
		destGroup = _contactGroupSet.FindByGroup( ContactGroup(destGroupName, EnumGroupType::GroupType_User) );
	}

	if ( origGroup  )
	{
		bMove = origGroup != destGroup;
	}
	else
	{
		bMove = false;		//VOXOX - JRT - 2009.07.08  - we some crashes because origGroup == NULL.  Weird.
	}

	//VOXOX - JRT - 2009.08.05 - If contact is already in this group, let's not go thru the motions of moving it unnecessarily.
	if ( bMove )
	{
		if ( contact.isInGroup( destGroup->getKey() ) )
		{
			bMove = false;
		}
	}

	if ( bMove )
	{
		if ( origGroup )
		{
			origGroup->removeContact( contact );
		}

		if ( destGroup )
		{
			if ( origGroup )
			{
				contact.removeFromGroup( *origGroup );
			}

			destGroup->addContact( contact );
			contact.addToGroup     ( *destGroup );
			
			contactMovedEvent(*this, *destGroup, *origGroup, contact);

			//This block changes the group on the IM network server(s).	//VOXOX - JRT - 2009.07.29 
			for (IMContactSet::const_iterator it = contact.getIMContactSet().begin(); it != contact.getIMContactSet().end(); ++it) 
			{
				LOG_DEBUG( "Moving IMContact(" + (*it).getKey() + ") from group " + origGroup->getName() + " to " + destGroup->getName() );
				_imContactListHandler.moveContactToGroup( destGroup->getName(), origGroup->getName(), (IMContact &)*it);
			}
		}
	}
}

void ContactList::moveContactToGroup( const std::string& srcGroupId, const std::string& destGroupId, Contact & contact)	//VOXOX - JRT - 2009.08.09 - Multiple groups
{
	//VOXOX - JRT - 2009.05.05 - This code is EXACTLY the same as _moveContactToGroup().  Just call that.
//	_moveContactToGroup( destGropuId, contact );
	_moveContactToGroup( srcGroupId, destGroupId, contact );

//	RecursiveMutex::ScopedLock lock(_mutex);
//
//	ContactGroup * contactGroup = getContactGroup(contact.getGroupId());
//	std::string oldGroupName;
//
//	if (contactGroup) {
//		oldGroupName = contactGroup->getName();
//	}
//
//	if (oldGroupName != dst) {
//		addContactGroup(dst);
//
//		ContactGroupSet::iterator oldIt = _contactGroupSet.find(ContactGroup(oldGroupName));
//		ContactGroupSet::iterator newIt = _contactGroupSet.find(ContactGroup(dst));
//
//		for (IMContactSet::const_iterator it = contact.getIMContactSet().begin();
//			it != contact.getIMContactSet().end();
//			++it) {
//			_imContactListHandler.moveContactToGroup((*newIt).getName(), oldGroupName, (IMContact &)*it);
//		}
//
//		if ((newIt != _contactGroupSet.end())
//			&& (oldIt != _contactGroupSet.end())) {
//			((ContactGroup &)(*oldIt)).removeContact(contact);
//			((ContactGroup &)(*newIt)).addContact(contact);
//
////			contact.setGroupId((*newIt).getUUID());
//			contact.setGroupId((*newIt).getKey1());	//VOXOX - JRT - 2009.05.05 
//			contactMovedEvent(*this, (ContactGroup &)*newIt, (ContactGroup &)*oldIt, contact);
//		}
//	}
}

Contact * ContactList::getContactByUuid(const std::string & uuid )
{
	RecursiveMutex::ScopedLock lock(_mutex);
	return _contacts.FindByUuid( uuid );
}
//VOXOX CHANGE CJC GET CONTACT BY ITS ID
Contact * ContactList::getContactById(const std::string & contactId)
{
	return _contacts.FindByContactId( contactId );	//VOXOX - JRT - 2009.08.03 - Does locking
}

Contact * ContactList::getContactById(const std::string & contactId, int userNetworkId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact*  result = NULL;
	IMAccount*   imAccount  = _userProfile.getIMAccountManager().getIMAccount( userNetworkId );

	if ( imAccount )
	{
		std::string contactKey = contactId + imAccount->getKey();

		result = _contacts.FindByContactIdUserNetwork( contactId, userNetworkId );
	}

	return result;
}

Contact* ContactList::getContactByKey(const std::string & key) 
{
	return _contacts.FindByContactId( key );			//Does locking.
}



//VOXOX CHANGE by Rolando 04-27-09 - Used to get Contact by phonenumber
Contact * ContactList::getContactByVoxOxPhoneNumber(const std::string & phoneNumber) {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _contacts.FindByVoxOxPhoneNumber( phoneNumber );
}
//VOXOX - CJC - 2009.06.11 
Contact * ContactList::getContactByEmail(const std::string & email) {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _contacts.FindByEmail( email );
}

//VOXOX - CJC - 2009.06.11 
Contact * ContactList::getContactByNumber(const std::string & phoneNumber) {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _contacts.FindByPhoneNumber( phoneNumber );
}

Contact * ContactList::getContactByJSONId( int JsonId ) {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _contacts.FindByJSONId( JsonId );
}



void ContactList::contactChangedEventHandler(Contact & sender) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if ( _userProfile.isConnected() && !_userProfile.isDestroying() )		//VOXOX - JRT - 2009.09.21 - prevent crash on exit if presence changed.
	{
		contactChangedEvent(*this, sender);
	}
}

void ContactList::lock() const {
	_mutex.lock();
}

void ContactList::unlock() const {
	_mutex.unlock();
}

std::string ContactList::getContactGroupIdFromName(const std::string & groupName) 	//JRT-GRPS - This probably won't work anymore.
{
	RecursiveMutex::ScopedLock lock(_mutex);

	std::string result;

	ContactGroup tmpGroup( groupName, EnumGroupType::GroupType_User );
	ContactGroup* pGroup = _contactGroupSet.FindByGroup( tmpGroup );

	result = (pGroup ? pGroup->getKey() : "");

	return result;
}

//VOXOX - JRT - 2009.05.26 
void ContactList::createWsContactProfile()
{
	_wsContactProfile = new WsContactProfile( _userProfile );
	wsContactProfileCreatedEvent(*this, *_wsContactProfile);
	_wsContactProfile->wsContactProfileSetEvent += boost::bind(&ContactList::wsContactProfileSetEventHandler, this, _1, _2, _3, _4, _5 );
	_wsContactProfile->wsContactProfileGetEvent += boost::bind(&ContactList::wsContactProfileGetEventHandler, this, _1, _2, _3 );
}

void ContactList::requestContactProfile( const std::string contactKey, int userNetworkId, int timestamp )
{
	if ( _wsContactProfile )		//VOXOX - JRT - 2009.05.27 - TODO: we sometimes get here before _wsContactProfile has been ctor'd
	{
		if ( userNetworkId > 0 )	//VOXOX - JRT - 2009.07.22 
		{
			_wsContactProfile->getContactProfile( contactKey, userNetworkId, timestamp );
		}
		else
		{
			LOG_WARN( " Skipping getContactProfile because userNetworkId == 0: " + contactKey );
		}
	}
}

void ContactList::wsContactProfileSetEventHandler( WsContactProfile& sender, int id, const std::string& contactId,
												   int userNetworkId, int timestamp )
{
	Contact* contact = getContactById( contactId, userNetworkId );

	if ( contact )
	{
		contact->setTimestamp( timestamp );
	}
}

void ContactList::wsContactProfileGetEventHandler( WsContactProfile& sender, int id, ContactProfile& profile )
{
	IMContact* imContact = profile.getPreferredIMContact();	//Should be first and only.

	if ( imContact )
	{
		//We must convert our IMContact::_serverProtocolId to an actual IMAccount before we do the find.
		IMAccount* imAccount = NULL;

		//TODO: revisit this after server-side changes.
		if ( imContact->getUserNetworkId() > 0 )	//Preferred method
		{
			imAccount = _userProfile.getIMAccountManager().getIMAccount( imContact->getUserNetworkId() );	//TODO: we are NOT getting this from server.

			if ( imAccount == NULL )
				int xxx = 1;
		}
		else
		{
			imAccount = _userProfile.getIMAccountManager().getIMAccount( imContact->getServerProtocolId() );	//TODO: assumes ONE account per network Protocol.
			if ( imAccount == NULL )
				int xxx = 1;
		}

		imContact->setIMAccount( imAccount );

		Contact* contact = findContactThatOwns( *imContact );

		if ( contact )
		{
			contact->copyJustProfile( profile );	//Let's avoid copying groups, IMContacts, etc.
			contactChangedEvent(*this, *contact);
		}
		else
		{
			LOG_WARN( "Cannot find Contact for IMContact" + std::string( imContact->getKey() ) );
		}
	}
	else
	{
		LOG_WARN( "No preferred Contact" );
	}
}
//End VoxOx

//VOXOX - JRT - 2009.05.30 
void ContactList::populateFilterGroup( const std::string& text, const std::string& filterName )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if ( _filterGroupText != text )
	{
		int nCount = 0;
		setInitialLoad( true );

		ContactGroup group( filterName, EnumGroupType::GroupType_Filter );
		_filterGroupText = text;

		if ( _filterGroupText.empty() )
		{
			_removeContactGroup( group.getKey() );
		}
		else
		{
			_removeContactGroup( group.getKey() );	//Ensure we get a clean start.
			_addContactGroup( group );
		
			ContactGroup* pGroup = _contactGroupSet.FindByGroup( group );

			if ( pGroup )
			{
				for ( Contacts::iterator it = _contacts.begin(); it != _contacts.end(); it++ )
				{
					if ( (*it).second.containsText( _filterGroupText ) )
					{
	//					_addToContactGroup( group, (*it).second );	//Causes deadlock

						pGroup->addContact((*it).second);			//So do it manually.
						(*it).second.addToGroup( *pGroup );

						nCount++;
					}
				}

				contactGroupAddedEvent (*this, group );		//Force UI to redraw.  TODO: new event?
			}
		}
	
		setInitialLoad( false );
	}
}

int ContactList::getImContactUserNetworkId( IMContact& imContact )
{
	int userNetworkId  = imContact.getIMAccount()->getUserNetworkId();

	if ( userNetworkId == 0 )
	{
		IMAccount* imAccount = _userProfile.getIMAccountManager().getIMAccount( *imContact.getIMAccount() );

		if ( imAccount )
		{
			userNetworkId = imAccount->getUserNetworkId();
		}
	}

//	assert( serverNetworkId != 0 );

	return userNetworkId;
}

//VOXOX - JRT - 2009.06.03 - This HACK is needed because WengoPhone chose to CLONE ImAccount info in each contact.
//	TODO: do NOT clone IMAccount info in each IMContact.
void ContactList::updateUserNetworkIds( IMAccount& imAccount )
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.07.22 JRT-XXX

	QtEnumIMProtocol::ServerProtocolId	serverProtocolId = imAccount.getServerProtocolId();
	int									userNetworkId    = imAccount.getUserNetworkId();	//VOXOX - JRT - 2009.06.09 

	if ( userNetworkId > 0 )
	{
		LOG_INFO( " updating UserNetwork IDs.  Contact count: " + String::fromNumber( const_cast<Contacts&>(getContacts()).GetCount()) );

		_contacts.lock();	//VOXOX - JRT - 2009.07.22 

		for ( Contacts::iterator it = _contacts.begin(); it != _contacts.end(); it++ )
		{
			IMContactSet& rImContacts = const_cast<IMContactSet&>((*it).second.getIMContactSet());

			for ( IMContactSet::iterator it2 = rImContacts.begin(); it2 != rImContacts.end(); it2++ )
			{
				IMAccount* tempImAccount = const_cast<IMAccount*>((*it2).getIMAccount());
				if ( tempImAccount )					//We may have some leftover, cached contacts.  TODO: fix this so related contacts are removed.
				{
					if ( *tempImAccount == imAccount )
					{
						if ( userNetworkId != tempImAccount->getUserNetworkId() )
						{
							tempImAccount->setUserNetworkId( userNetworkId );

							if ( serverProtocolId != QtEnumIMProtocol::ServerProtocolIdNone )
							{
								tempImAccount->setServerProtocolId( serverProtocolId );
							}

							//Since we only now have proper serverNetworkId, let's request the Contact Profile.
							requestContactProfile( (*it).second.getContactId(), userNetworkId, (*it).second.getTimestamp() );
						}
					}
				}
			}
		}

		_contacts.unlock();	//VOXOX - JRT - 2009.07.22 
	}
	else
	{
		LOG_WARN( " UserNetworkID == 0" );
	}
}

//-----------------------------------------------------------------------------

std::string ContactList::getContactGroupName( const std::string &groupId )
{
	std::string result = "";

//	lock();

	ContactGroup * contactGroup = getContactGroup(groupId);	//Does any needed locking
	if (contactGroup) 
	{
		result = contactGroup->getName();
	}

//	unlock();

	return result;
}

//-----------------------------------------------------------------------------

std::string ContactList::getContactGroupContactsOnline(const std::string & groupId)
{
	std::string result = "";
	//VOXOX - JRT - 2009.07.13 
//JRT-XXX	lock();

	ContactGroup * contactGroup = getContactGroup(groupId);

	if (contactGroup) 
	{
		result = contactGroup->getContactsOnline();
	}

//JRT-XXX	unlock();	//VOXOX - JRT - 2009.07.13 

	return result;
}

//-----------------------------------------------------------------------------

StringList ContactList::getContactIds() const 
{
	StringList result;

//JRT-XXX	lock();	//VOXOX - JRT - 2009.07.13 

	const Contacts & contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		result += const_cast<Contact&>((*it).second).getKey();
	}

//JRT-XXX	unlock();	//VOXOX - JRT - 2009.07.13 

	return result;
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.27 
void ContactList::getContactsWithHomeNumber( ContactInfoList2& infoList ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	ContactInfo2 contactInfo;
	std::string	 number = "";

	const Contacts& contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		number = const_cast<Contact&>((*it).second).getHomePhone();

		if ( !number.empty() )
		{
			contactInfo.initVars();

			contactInfo.setKey		  ( (*it).second.getKey()		  );
			contactInfo.setDisplayName( (*it).second.getDisplayName() );
			contactInfo.getTelephones().addMobileNumber( number  );

			infoList.Add( &contactInfo );
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.27 
void ContactList::getContactsWithMobileNumber( ContactInfoList2& infoList ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	ContactInfo2 contactInfo;
	std::string	 number = "";

	const Contacts& contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		number = const_cast<Contact&>((*it).second).getMobilePhone();

		if ( !number.empty() )
		{
			contactInfo.initVars();

			contactInfo.setKey		  ( (*it).second.getKey()		  );
			contactInfo.setDisplayName( (*it).second.getDisplayName() );
			contactInfo.getTelephones().addMobileNumber( number  );

			infoList.Add( &contactInfo );
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.27 
void ContactList::getContactsWithSipNumber( ContactInfoList2& infoList ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact*	 contact = NULL;
	ContactInfo2 contactInfo;
	std::string	 mobileNumber = "";

	const Contacts & contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		contact = const_cast<Contact*>(&(*it).second);
			
		if ( contact->hasAvailableSIPNumber() ) 
		{
			contactInfo.initVars();

			contactInfo.setKey		  ( (*it).second.getKey()		  );
			contactInfo.setDisplayName( (*it).second.getDisplayName() );
			contactInfo.getTelephones().addVoxOxNumber( contact->getFirstFreePhoneNumber()  );

			infoList.Add( &contactInfo );
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.27 
void ContactList::getContactsWithWengoCall( ContactInfoList2& infoList ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact*	 contact = NULL;
	ContactInfo2 contactInfo;
	std::string	 mobileNumber = "";

	const Contacts & contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		contact = const_cast<Contact*>(&(*it).second);

		if ( contact->hasAvailableWengoId() )
		{
			if ( contact->getPresenceState() != EnumPresenceState::PresenceStateOffline && 
				 contact->getPresenceState() != EnumPresenceState::PresenceStateUnknown && 
				 contact->getPresenceState() != EnumPresenceState::PresenceStateUnavailable) 
			{
				contactInfo.initVars();

				contactInfo.setKey		    ( contact->getKey()		      );
				contactInfo.setDisplayName  ( contact->getDisplayName()   );
				contactInfo.setPresenceState( contact->getPresenceState() );

				contactInfo.getTelephones().addVoxOxNumber( contact->getFirstAvailableWengoIMContact().getDisplayContactId());

				infoList.Add( &contactInfo );
			}
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.27 
void ContactList::getContactsThatCanChat( ContactInfoList2& infoList ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	Contact*	 contact = NULL;
	ContactInfo2 contactInfo;
	std::string	 mobileNumber = "";

	const Contacts & contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		contact = const_cast<Contact*>(&(*it).second);

		if ( contact->getPreferredIMContact() != NULL )
		{
			if ( contact->hasIM() && contact->isOnline() )
			{
//				tmpContactProfile.hasIM() && tmpContactProfile.getPresenceState() != EnumPresenceState::PresenceStateOffline) {

				contactInfo.initVars();

				contactInfo.setKey		    ( contact->getKey()		      );
				contactInfo.setDisplayName  ( contact->getDisplayName()   );
				contactInfo.setPresenceState( contact->getPresenceState() );

				infoList.Add( &contactInfo );
			}
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.09.03 
void ContactList::getCounts( int& total, int& online ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	total  = 0;
	online = 0;

	const Contacts & contacts = getContacts();

	for (Contacts::const_iterator it = contacts.begin(); it != contacts.end(); ++it) 
	{
		if ( (*it).second.isAvailable())
		{
			online++;
		}

		total++;
	}
}

//-----------------------------------------------------------------------------

IMAccount* ContactList::getIMAccount( const std::string& imAccountId )		//VOXOX - JRT - 2009.09.22 
{
	return _userProfile.getIMAccountManager().getIMAccount( imAccountId );
}

//-----------------------------------------------------------------------------
