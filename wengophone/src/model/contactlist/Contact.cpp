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
#include "Contact.h"

#include <model/contactlist/ContactGroup.h>
#include <model/contactlist/ContactList.h>
#include <model/presence/PresenceHandler.h>
#include <model/profile/UserProfile.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMChat.h>

#include <util/Logger.h>
#include <util/OWPicture.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>
#include <util/Uuid.h>

#include <iostream>

using namespace std;

Contact::Contact(UserProfile & userProfile)
	: ContactProfile(),
	_userProfile(userProfile),
	_contactList(userProfile.getContactList()) {

	profileChangedEvent += boost::bind(&Contact::profileChangedEventHandler, this, _1);
}

Contact::Contact(const Contact & contact)
	: ContactProfile(contact),
	Trackable(),
	_userProfile(contact._userProfile),
	_contactList(contact._contactList) {

	copy(contact);
}

Contact::~Contact() {
}

Contact & Contact::operator=(const Contact & contact) {
	if (&contact != this) {
		copy(contact);
	}

	contactChangedEvent(*this);

	return *this;
}

Contact & Contact::operator=(const ContactProfile & contactProfile) {
	if (&contactProfile != this) {
		copy(contactProfile);
	}

	contactChangedEvent(*this);

	return *this;
}

void Contact::copy(const Contact & contact) {
	copy((const ContactProfile &)contact );

	//VOXOX - JRT - 2009.04.27 
//	getGroups()			= const_cast<Contact&>(contact).getGroups();			
	getMergedContacts() = const_cast<Contact&>(contact).getMergedContacts();
	//End VoxOx
}

//VOXOX - JRT - 2009.04.28  - Sometimes we just want to copy the data without any side-effects
void Contact::copyJustData( const ContactProfile& contactProfile )
{
	copy( contactProfile, true );
}

void Contact::copyJustProfile( const ContactProfile& contactProfile )
{
	Profile::copy(contactProfile, false);
}
//End VoxOx

void Contact::copy(const ContactProfile & contactProfile, bool justData) 
{
	_uuid = contactProfile._uuid;
	profileChangedEvent += boost::bind(&Contact::profileChangedEventHandler, this, _1);

	// Sets groups
	if ( justData )
	{
//		_groupId = contactProfile.getGroupId();
//		_contactGroupInfoSet = const_cast<ContactProfile&>(contactProfile).getContactGroupInfoSet();	//VOXOX - JRT - 2009.07.08 - Done below.
//		_imContactSet        = const_cast<ContactProfile&>(contactProfile)._imContactSet;				//VOXOX - JRT - 2009.05.10 
	}
	else
	{
		_contactList.lock();

		//VOXOX - JRT - 2009.05.06 TODO: review this when we enable multiple user groups.
		std::string oldGroupId = getFirstUserGroupId();
		std::string newGroupId = const_cast<ContactProfile&>(contactProfile).getFirstUserGroupId();

		ContactGroup * newContactGroup = _contactList.getContactGroup( newGroupId );	//VOXOX - JRT - 2009.05.06 JRT-GRPS
		ContactGroup * oldContactGroup = _contactList.getContactGroup( oldGroupId );

		if (oldContactGroup && newContactGroup && (oldContactGroup->getKey() != newContactGroup->getKey())) 	//VOXOX - JRT - 2009.05.05 
		{
			_contactList.moveContactToGroup( oldContactGroup->getName(), newContactGroup->getName(), *this);
		} 
		else if (!oldContactGroup && newContactGroup) 
		{
			ContactGroup contactGroup( newContactGroup->getName(), EnumGroupType::GroupType_User );
			_contactList._addToContactGroup( contactGroup, *this);	//VOXOX - JRT - 2009.05.07 JRT-GRPS
		}

		_contactList.unlock();
	}
	////

	Profile::copy(contactProfile);
	_preferredIMContact  = contactProfile._preferredIMContact;
//	_groupId			 = contactProfile._groupId;
	_contactGroupInfoSet = const_cast<ContactProfile&>(contactProfile).getContactGroupInfoSet();	//VOXOX - JRT - 2009.05.06 - JRT-GRPS
	getGroups()			 = const_cast<ContactProfile&>(contactProfile).getGroups();					//VOXOX - JRT - 2009.05.10 JRT-GRPS
	_presenceState		 = contactProfile._presenceState;

	//VOXOX - JRT - 2009.06.29 - NOTE: I've always hated this code being here.  Now it is causing
	//			app to freeze when adding contacts.  So let's try just copying the IMContactSet!
	//			Too bad it breaks AddContact().  NO invites are sent.
	//VOXOX - JRT - 2009.07.13 - Moved to separate method for readability.
//	handleContactSetChanges( contactProfile._imContactSet );
	_imContactSet = contactProfile._imContactSet;		//VOXOX - JRT - 2009.07.13 

	//VOXOX - JRT - 2009.04.27 - re-establish the event handlers.
	for (IMContactSet::iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	{
		IMContact& temp = const_cast<IMContact&>(*it);
		temp.imContactChangedEvent += boost::bind(&Contact::imContactChangedEventHandler, this, _1);
//		(*it).imContactChangedEvent += boost::bind(&Contact::imContactChangedEventHandler, this, _1);
	}
	//End VoxOx
}

//VOXOX - JRT - 2009.07.13 
void Contact::handleContactSetChanges( const IMContactSet& newImContactSet )
{
	handleContactSetChanges( _imContactSet, newImContactSet );
}

//VOXOX - JRT - 2009.07.13 
void Contact::handleContactSetChanges( const IMContactSet& oldImContactSet, const IMContactSet& newImContactSet )
{
	// Removes not present IMContacts
	for (IMContactSet::const_iterator it = oldImContactSet.begin(); it != oldImContactSet.end();) 
	{
		if (!newImContactSet.hasIMContact(*it)) 
		{
			IMContactSet::const_iterator curIt = it++;
			_contactList.removeIMContact(*this, *curIt);
		} 
		else 
		{
			it++;
		}
	}

	// Adds present IMContacts
	for (IMContactSet::const_iterator it = newImContactSet.begin(); it != newImContactSet.end(); ++it) 
	{
		if (!hasIMContact(*it)) 
		{
			_contactList.addIMContact(*this, *it);
		}
	}
}

bool Contact::operator==(const Contact & contact) const {
	return (ContactProfile::operator == (contact));
}

void Contact::addIMContact(const IMContact & imContact) {
	_contactList.addIMContact(*this, imContact);
}

void Contact::removeIMContact(const IMContact & imContact) {
	_contactList.removeIMContact(*this, imContact);
}

void Contact::_addIMContact(const IMContact & imContact) {
	pair<IMContactSet::const_iterator, bool> result = _imContactSet.insert(imContact);

	if (result.second) {
		IMContact & newIMContact = (IMContact &)(*result.first);

		newIMContact.imContactChangedEvent += boost::bind(&Contact::imContactChangedEventHandler, this, _1);

		_userProfile.getPresenceHandler().subscribeToPresenceOf(*result.first);
		contactChangedEvent(*this);
	}
}

void Contact::_removeIMContact(const IMContact & imContact) 
{
	//VOXOX - JRT - 2009.06.06 
	IMContact* pImContact = _imContactSet.findByContact(imContact);

	if (pImContact) 
	{
		_userProfile.getPresenceHandler().unsubscribeToPresenceOf(imContact);
		contactChangedEvent(*this);
		_imContactSet.Delete( imContact );
	}
}

bool Contact::checkAndSetIMContact(const IMContact & imContact) {
	for (IMContactSet::iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) {
		if ((*it) == imContact) {
			if ((*it).getIMAccountId().empty()) {
				IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount(imContact.getIMAccountId());
				((IMContact &)(*it)).setIMAccount(imAccount);
				OWSAFE_DELETE(imAccount);
			}
			return true;
		}
	}

	return false;
}

const IMContact * Contact::getFirstValidIMContact(const IMAccount & imAccount) const {
	const IMContact * result = NULL;

	//VOXOX - JRT - 2009.04.27 - TODO: move this to IMContactSet class.
	for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	{
		if ( (*it).getIMAccountId() == imAccount.getKey() )	//VOXOX - JRT - 2009.04.24
		{
			if ( (*it).isValid() )
			{
				result = &(*it);
				break;
			}
		}
	}

	return result;
}

void Contact::imContactChangedEventHandler(IMContact & sender) {
	updatePresenceState();
	contactChangedEvent(*this);
}
///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
void Contact::block() {
	for (IMContactSet::const_iterator it = _imContactSet.begin() ; it != _imContactSet.end() ; it++) {
		_userProfile.getPresenceHandler().blockContact(*it);
	}

	getPreferredIMContact()->setBlocked(true);
	contactChangedEvent(*this);
}

///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
void Contact::unblock() {
	for (IMContactSet::const_iterator it = _imContactSet.begin() ; it != _imContactSet.end() ; it++) {
		_userProfile.getPresenceHandler().unblockContact(*it);
	}

	getPreferredIMContact()->setBlocked(false);
	contactChangedEvent(*this);
}


void Contact::profileChangedEventHandler(Profile & profile) {
	contactChangedEvent(*this);
}

void Contact::setIcon(const OWPicture & icon) {
}

std::string Contact::getNetworkName()
{
	std::string networkName = "";
	
	IMContact* imContact = getPreferredIMContact();

	QtEnumIMProtocol::IMProtocol qtProtocol = QtEnumIMProtocolMap::getInstance().toQtIMProtocol( imContact->getProtocol(), imContact->isIMAccountVoxOx(), imContact->isIMAccountGtalk() );
	
	networkName = QtEnumIMProtocolMap::getInstance().toString( qtProtocol );

	return networkName;
}

void Contact::merge(const Contact & contact) {
	if (getFirstName().empty()) {
		setFirstName( contact.getFirstName() );
	}

	if ( getLastName().empty()) 
	{
		setLastName( contact.getLastName() );
	}

	if ( getSex() == EnumSex::SexUnknown) 
	{
		setSex( contact.getSex() );
	}

	if ( getCompany().empty()) 
	{
		setCompany( contact.getCompany() );
	}

	//TODO: how can we detect that birthdate is not filled
	//_birthdate = contact._birthdate;

	//VOXOX - JRT - 2009.03.30
	getUrls().Merge( const_cast<Contact&>(contact).getUrls() );
	//End VOXOX

	//VOXOX - JRT - 2009.03.30
	getTelephones().Merge( const_cast<Contact&>(contact).getTelephones() );
	//End VOXOX

	//VOXOX - JRT - 2009.03.30
	getEmailAddresses().Merge( const_cast<Contact&>(contact).getEmailAddresses());
	//End VOXOX

	//TODO: how can we detect that streetAddress is not filled
	//_streetAddress = contact._streetAddress;
	//VOXOX - JRT - 2009.03.30
	getStreetAddresses().Merge( const_cast<Contact&>(contact).getStreetAddresses() );
	//End VOXOX

	if ( getNotes().empty()) 
	{
		setNotes( contact.getNotes() );
	}

	if (!_preferredIMContact) 
	{
		_preferredIMContact = contact._preferredIMContact;
	}

	for (IMContactSet::const_iterator it = contact._imContactSet.begin();
		it != contact._imContactSet.end(); ++it) 
	{
		if (!hasIMContact(*it)) {
			_imContactSet.insert(*it);
		}
	}

	//VOXOX - JRT - 2009.05.06 - JRT-GRPS
//	if (_groupId.empty()) {
//		_groupId = contact._groupId;
//	}
	_contactGroupInfoSet = const_cast<Contact&>(contact).getContactGroupInfoSet();
	//End JRT-GRPS

	contactChangedEvent(*this);
}

bool Contact::containsText( const std::string& textIn )
{
	std::string tgtValue = "";
	String text = textIn;
	text = text.toLowerCase();

	if ( startsWith( text, getDisplayName(), tgtValue ) )
	{
		return true;
	}

	if ( startsWith( text, getCompleteName(), tgtValue ) )
	{
		return true;
	}

	const IMContactSet& rContacts = getIMContactSet();

	for ( IMContactSet::const_iterator it = rContacts.begin(); it != rContacts.end(); it++ )
	{
//		if ( (*it).isValid() )
		{
			if ( startsWith( text, (*it).getContactId(), tgtValue ) )
			{
				return true;
			}

			if ( startsWith( text, (*it).getAlias(), tgtValue ) )
			{
				return true;
			}
		}
	}

	if ( startsWith( text, getAlias(), tgtValue ) )
	{
		return true;
	}

	if ( startsWith( text, getContactId(), tgtValue ) )
	{
		return true;
	}

	if ( startsWith( text, getFirstName(), tgtValue ) )
	{
		return true;
	}

	if ( startsWith( text, getLastName(), tgtValue ) )
	{
		return true;
	}

	if ( startsWith( text, getShortDisplayName(), tgtValue ) )
	{
		return true;
	}


	//Telephones
	Telephones& rPhones = getTelephones();

	for ( Telephones::const_iterator it = rPhones.begin(); it != rPhones.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			if ( startsWith( text, (*it).getNumber(), tgtValue ) )
			{
				return true;
			}
		}
	}

	//Emails
	EmailAddresses& rEmail = getEmailAddresses();
	for ( EmailAddresses::const_iterator it = rEmail.begin(); it != rEmail.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			if ( startsWith( text, (*it).getAddress(), tgtValue ) )
			{
				return true;
			}
		}
	}

	//URLS - TODO

	return false;
}

bool Contact::startsWith( const std::string& text, const std::string& tgtValueIn, std::string& tgtString )
{
	tgtString = tgtValueIn;	//This is just for easier debugging.
	String tgtValue = tgtValueIn;
	tgtValue = tgtValue.toLowerCase();
	return ( tgtValue.find( text ) == 0 );
}

//VOXOX - JRT - 2009.07.26 
bool Contact::isOnline() const
{
	return EnumPresenceState::isOnlineEx( getPresenceState() );
}

bool Contact::canBeCalled() const
{
	bool result = false;

	if( getPreferredIMContact()->isIMAccountVoxOx()  || getPreferredIMContact()->getQtProtocol() == QtEnumIMProtocol::IMProtocolSkype )//VOXOX - CJC - 2009.10.13 Check for Skype Calls
	{
		result = true;
	}
	else
	{
		result = const_cast<Contact*>(this)->getTelephones().hasValid();
	}

	return result;
}

bool Contact::canReceiveSMS() const
{
	bool result = false;

	std::string phone = const_cast<Contact*>(this)->getMobilePhone();

	result = !phone.empty();

	return result;
}

bool Contact::canReceiveFax() const
{
	bool result = false;

	std::string phone = const_cast<Contact*>(this)->getFax();

	result = !phone.empty();

	return result;
}

bool Contact::canReceiveEmail() const
{
	bool result = const_cast<Contact*>(this)->getEmailAddresses().hasValid();
	return result;
}

bool Contact::canDoFileTransfer() const
{
	bool result = false;

	if ( !getFirstWengoId().empty() && isAvailable() ) 
	{
		//VOXOX - JRT - 2009.07.27 - TODO: is this Available by definition, so we don't need the subsequent presence check?
		IMContact imContact = getFirstAvailableWengoIMContact();

//		if ( ( imContact.getPresenceState() != EnumPresenceState::PresenceStateOffline) &&
//			 ( imContact.getPresenceState() != EnumPresenceState::PresenceStateUnknown) &&
//			 ( imContact.getPresenceState() != EnumPresenceState::PresenceStateUnavailable)) 
		if ( !EnumPresenceState::isOfflineEx( imContact.getPresenceState() ) )	//VOXOX - JRT - 2009.09.10 
		{
			result = true;
		}
	}
	
	return result;
}


