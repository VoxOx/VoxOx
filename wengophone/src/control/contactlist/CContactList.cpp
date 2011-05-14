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
#include "CContactList.h"

#include <model/WengoPhone.h>
#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactGroup.h>

#include <model/webservices/contactIntegration/WsContactProfile.h>
#include <control/webservices/contactIntegration/CWsContactProfile.h>

#include <control/CWengoPhone.h>

#include <presentation/PFactory.h>
#include <presentation/PContactList.h>

#include <thread/ThreadEvent.h>

#include <util/Logger.h>


//NOTE: we don't user _threadCounter for WengoPhone::postEvent because PFactory was causing the crash on exit/logout.
//	If we need same functionality for WengoPhone thread, then implement another ThreadCounter object.

CContactList::CContactList(ContactList & contactList, CWengoPhone & cWengoPhone)
	: _contactList(contactList),
	_cWengoPhone(cWengoPhone) 
{
	_destroying		   = false;	//VOXOX - JRT - 2009.09.22 
	_pContactList	   = NULL;
	_cWsContactProfile = NULL;

	_contactList.wsContactProfileCreatedEvent += boost::bind(&CContactList::wsContactProfileCreatedEventHandler, this, _1, _2);

	createWsContactProfile();	//Call now in case event already fired.

	//Create this in UI-thread.
	_threadCounter++;
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::initPresentationThreadSafe, this));
	PFactory::postEvent(event);
}

CContactList::~CContactList() 
{
	//VOXOX - JRT - 2009.10.08 - Other threads/events may be waiting on the mutex, so we must track those pending event
	//							   to be sure we don't delete early and get a crash.
	//							 So the logic is to:
	//								- Set _destroying flag (also so no new events are posted (may not need this) )
	//								- Unbind all event handlers (so no new events are posted)
	//								- Destroy _pContactList via postEvent (done with unbinding above).
	//								    This allows other pending events to complete before it is actually deleted.
	//								- Loop while any threads/events are running (we may need another flag here)
	//								- Continue with dtor.

	_destroying = true;

	//Created this in UI-thread, so destroy it in UI thread.
	_threadCounter++;
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::uninitPresentationThreadSafe, this));
	PFactory::postEvent(event);

	//Wait for all pending postEvents to complete, the last of which should destroy _pContactList.
	waitForOkToDestroy( 5000 );	

	//Complete dtor
	OWSAFE_DELETE( _cWsContactProfile );
}

//VOXOX - JRT - 2009.10.08 - This is needed to prevent various COE occurrences caused by the use of threads to handle events.  
//								We should NOT destroy this object while other threads are running.
void CContactList::waitForOkToDestroy( int maxWait )	//In milliseconds
{
	int waitInc   = 100;
	int waitTotal =   0;
	int count     = _threadCounter.getCount();

	while ( _threadCounter.getCount() > 0 && (waitTotal < maxWait) )
	{
		Thread::msleep( waitInc );
		waitTotal += waitInc;
	}

	if ( waitTotal > 0 )
	{
		LOG_INFO( "Waited " + String::fromDouble( waitTotal ) + " seconds for " + String::fromNumber( count ) + " events to complete." );
	}
}

void CContactList::initPresentationThreadSafe() 
{
	_pContactList = PFactory::getFactory().createPresentationContactList(*this);

	_contactList.contactGroupAddedEvent	     += boost::bind(&CContactList::contactGroupAddedEventHandler,	   this, _1, _2);
	_contactList.contactGroupRemovedEvent	 += boost::bind(&CContactList::contactGroupRemovedEventHandler,    this, _1, _2);
	_contactList.contactGroupsReorderedEvent += boost::bind(&CContactList::contactGroupsReorderedEventHandler, this, _1    );

	_contactList.contactAddedEvent		  += boost::bind(&CContactList::contactAddedEventHandler,		 this, _1, _2);
	_contactList.contactRemovedEvent	  += boost::bind(&CContactList::contactRemovedEventHandler,		 this, _1, _2);
	_contactList.contactMovedEvent		  += boost::bind(&CContactList::contactMovedEventHandler,		 this, _1, _2, _3, _4);
	_contactList.contactChangedEvent	  += boost::bind(&CContactList::contactChangedEventHandler,		 this, _1, _2);

	_contactList.setInitialLoadEvent	  += boost::bind(&CContactList::setInitialLoadEventHandler,		 this, _1, _2 );

	setInitialLoadEvent	  += boost::bind(&CContactList::setInitialLoadEventHandler, this, _1, _2 );

	_threadCounter--;
}

//VOXOX - JRT - 2009.10.07 - So we don't get crash on after this object is destroyed but events are still 'handled'
void CContactList::uninitPresentationThreadSafe() 
{
	_contactList.contactGroupAddedEvent	     -= boost::bind(&CContactList::contactGroupAddedEventHandler,		this, _1, _2);
	_contactList.contactGroupRemovedEvent	 -= boost::bind(&CContactList::contactGroupRemovedEventHandler,		this, _1, _2);
	_contactList.contactGroupsReorderedEvent -= boost::bind(&CContactList::contactGroupsReorderedEventHandler,	this, _1    );

	_contactList.contactAddedEvent			 -= boost::bind(&CContactList::contactAddedEventHandler,			this, _1, _2);
	_contactList.contactRemovedEvent		 -= boost::bind(&CContactList::contactRemovedEventHandler,			this, _1, _2);
	_contactList.contactMovedEvent			 -= boost::bind(&CContactList::contactMovedEventHandler,			this, _1, _2, _3, _4);
	_contactList.contactChangedEvent		 -= boost::bind(&CContactList::contactChangedEventHandler,			this, _1, _2);

	_contactList.setInitialLoadEvent		 -= boost::bind(&CContactList::setInitialLoadEventHandler,			this, _1, _2 );

	setInitialLoadEvent						 -= boost::bind(&CContactList::setInitialLoadEventHandler,			this, _1, _2 );

	//Since _pContactList was creatd on UI thread, it must be deleted there (mostly because it deletes other UI objects)
	OWSAFE_DELETE( _pContactList );

	_threadCounter--;
}

Presentation * CContactList::getPresentation() const {
	return _pContactList;
}

CWengoPhone & CContactList::getCWengoPhone() const {
	return _cWengoPhone;
}

void CContactList::contactAddedEventHandler(ContactList & sender, Contact & contact) {
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	//We do not emit the event if the Contact has no group because the GUI does not support a Contact with no group.
	if ( contact.hasGroups() )
	{
		_threadCounter++;

		typedef ThreadEvent2<void (std::string, std::string), std::string, std::string> MyThreadEvent;
		std::string groupId = contact.getFirstUserGroupId();	//VOXOX - JRT - 2009.08.09 - OK usage.  We expect to have ONLY one user group.
		MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactAddedEventHandlerThreadSafe, this, _1, _2), contact.getKey(), groupId);
		PFactory::postEvent(event);
	}
}

void CContactList::contactAddedEventHandlerThreadSafe(std::string contactId, std::string groupId)	//VOXOX - JRT - 2009.08.09 - Multiple groups
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )	//VOXOX - JRT - 2009.10.08 
		getPContactList()->contactAddedEvent(contactId, groupId );
	
	_threadCounter--;
}

void CContactList::contactRemovedEventHandler(ContactList & sender, std::string& contactId ) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent1<void (std::string), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactRemovedEventHandlerThreadSafe, this, _1), contactId );
	PFactory::postEvent(event);
}

void CContactList::contactRemovedEventHandlerThreadSafe(std::string contactId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )		//VOXOX - JRT - 2009.10.08 
		getPContactList()->contactRemovedEvent(contactId);

	_threadCounter--;
}

void CContactList::contactMovedEventHandler(ContactList & sender, ContactGroup & dstContactGroup, ContactGroup & srcContactGroup, Contact & contact) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent3<void (std::string, std::string, std::string), std::string, std::string, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactMovedEventHandlerThreadSafe, this, _1, _2, _3),
		dstContactGroup.getKey(), srcContactGroup.getKey(), contact.getKey());
	PFactory::postEvent(event);
}

void CContactList::contactMovedEventHandlerThreadSafe(std::string dstGroupId, std::string srcGroupId, std::string contactId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )
		getPContactList()->contactMovedEvent(dstGroupId, srcGroupId, contactId);	//Updates presentation.
	
	_threadCounter--;
}

void CContactList::contactGroupAddedEventHandler(ContactList & sender, ContactGroup & contactGroup) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent1<void (std::string), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactGroupAddedEventHandlerThreadSafe, this, _1), contactGroup.getKey());
	PFactory::postEvent(event);
}

void CContactList::contactGroupAddedEventHandlerThreadSafe(std::string groupId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )
		getPContactList()->contactGroupAddedEvent(groupId);
	
	_threadCounter--;
}

void CContactList::contactGroupRemovedEventHandler(ContactList & sender, ContactGroup & contactGroup) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;

	typedef ThreadEvent1<void (std::string), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactGroupRemovedEventHandlerThreadSafe, this, _1), contactGroup.getKey());
	PFactory::postEvent(event);
}

void CContactList::contactGroupRemovedEventHandlerThreadSafe(std::string groupId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )
		getPContactList()->contactGroupRemovedEvent(groupId);

	_threadCounter--;
}

void CContactList::contactGroupRenamedEventHandler(ContactList & sender, ContactGroup & contactGroup) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent1<void (std::string), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactGroupRenamedEventHandlerThreadSafe, this, _1), contactGroup.getKey());
	PFactory::postEvent(event);
}

void CContactList::contactGroupRenamedEventHandlerThreadSafe(std::string groupId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )
		getPContactList()->contactGroupRenamedEvent(groupId);

	_threadCounter--;
}

void CContactList::contactGroupsReorderedEventHandler(ContactList & sender) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactGroupsReorderedEventHandlerThreadSafe, this));
	PFactory::postEvent(event);
}

void CContactList::contactGroupsReorderedEventHandlerThreadSafe() 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if ( getPContactList() )
		getPContactList()->contactGroupsReorderedEvent();

	_threadCounter--;
}

void CContactList::setInitialLoadEventHandler(ContactList & sender, int nOnOff) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent1<void (int), int> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::setInitialLoadEventHandlerThreadSafe, this, _1), nOnOff );
	PFactory::postEvent(event);
}

void CContactList::setInitialLoadEventHandlerThreadSafe( int nOnOff ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if( getPContactList() )
		getPContactList()->setInitialLoadEvent( nOnOff );

	_threadCounter--;
}

void CContactList::contactChangedEventHandler(ContactList & sender, Contact & contact) 
{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	_threadCounter++;
	typedef ThreadEvent2<void (std::string, std::string), std::string, std::string> MyThreadEvent;
	std::string groupId = contact.getFirstUserGroupId();		//VOXOX - JRT - 2009.08.09 OK usage
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::contactChangedEventHandlerThreadSafe, this, _1, _2), contact.getKey(), groupId );	//VOXOX - JRT - 2009.04.26 
	PFactory::postEvent(event);
}

void CContactList::contactChangedEventHandlerThreadSafe(std::string contactId, std::string groupId ) 	//VOXOX - JRT - 2009.08.09 - Multiple groups.
{
//	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.10.08 

	if( getPContactList() )
		getPContactList()->contactChangedEvent(contactId, groupId );

	_threadCounter--;
}

std::string CContactList::getContactGroupName(const std::string & groupId) const 
{
	return _contactList.getContactGroupName( groupId );
}

ContactGroup* CContactList::getContactGroup( const std::string& groupId ) const
{
	return _contactList.getContactGroup(groupId);	//ContactList will handle locking
}

std::string CContactList::getContactGroupContactsOnline(const std::string & groupId) const 
{
	return _contactList.getContactGroupContactsOnline( groupId );	//ContactList will handle locking.
}


std::string CContactList::getContactGroupIdFromName(const std::string & groupName) const 
{
	return _contactList.getContactGroupIdFromName(groupName);
}

//VOXOX - JRT TODO - 2009.04.07 - This method results in copying the entire profile TWICE. 
//	Use it sparingly since, in most cases, caller needs a single data element.
//	Additionally, it triggers ProfileChanged events!  Dumb!
ContactProfile CContactList::getContactProfile(const std::string & contactId) const 
{
	ContactProfile result;
	Contact * contact = getContact(contactId);
	if (contact) 
	{
		result = *contact;
	}

	return result;
}

//VOXOX - JRT TODO - 2009.04.07 - This method results in copying the entire profile TWICE. 
//	Use is sparingly since, in most cases, caller needs a single data element.
ContactProfile CContactList::getContactProfileByVoxOxPhoneNumber(const std::string & phoneNumber) const 
{
	ContactProfile result;
	Contact * contact = getContactByVoxOxPhoneNumber(phoneNumber);
	if (contact) 
	{
		result = *contact;
	}

	return result;
}

Contact * CContactList::getContact(const std::string & contactId) const 
{
	return _contactList.getContactByKey(contactId);	
}

Contact * CContactList::getContactByVoxOxPhoneNumber(const std::string & phoneNumber) const 
{
	return _contactList.getContactByVoxOxPhoneNumber(phoneNumber);
}

//VOXOX CHANGE by Rolando - 2009.07.09 
Contact * CContactList::getContactByNumber(const std::string & phoneNumber) const 
{
	return _contactList.getContactByNumber(phoneNumber);//VOXOX CHANGE by Rolando - 2009.07.09 
}

Contact * CContactList::getContactById(const std::string & contactId) const {
	return _contactList.getContactById(contactId);
}

Contact * CContactList::getContactByUuid(const std::string & uuid) const {
	return _contactList.getContactByUuid(uuid);
}

Contact* CContactList::getContactByKey(const std::string & key ) const {
	return _contactList.getContactByKey( key );
}

QtEnumIMProtocol::IMProtocol CContactList::getContactQtProtocol( const std::string & key ) const
{
	QtEnumIMProtocol::IMProtocol protocol = QtEnumIMProtocol::IMProtocolUnknown;

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		protocol = contact->getQtIMProtocol();
	}

	return protocol;
}

std::string	CContactList::getContactIconData( const std::string & key ) const
{
	std::string result = "";

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		result = contact->getIcon().getData();
	}

	return result;
}

std::string	CContactList::getContactSocialAddress( const std::string & key ) const
{
	std::string result = "";

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		result = contact->getContactSocialAddress();
	}

	return result;
}

std::string	CContactList::getContactMobileNumber( const std::string & key ) const
{
	std::string result = "";

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		result = contact->getMobilePhone();
	}

	return result;
}

std::string	CContactList::getContactFirstWengoId( const std::string & key ) const
{
	std::string result = "";

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		result = contact->getFirstWengoId();
	}

	return result;
}

std::string	CContactList::getContactDisplayName( const std::string & key ) const
{
	std::string result = "";

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		result = contact->getDisplayName();
	}

	return result;
}

std::string	CContactList::getContactBestShortDisplayName( const std::string & key ) const
{
	std::string result = "";

	Contact* contact = getContactByKey( key );
	
	if ( contact )
	{
		result = contact->getBestShortDisplayName();
	}

	return result;
}

//VOXOX - JRT - 2009.07.26 
bool CContactList::isContactOnline( const std::string& key ) const
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		result = contact->isOnline();
	}

	return result;
}

bool CContactList::canCallContact( const std::string& key ) const
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		result = contact->canBeCalled();
	}

	return result;
}

bool CContactList::canContactReceiveSMS( const std::string& key ) const
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		result = contact->canReceiveSMS();
	}

	return result;
}

bool CContactList::canContactReceiveFax( const std::string& key ) const
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		result = contact->canReceiveFax();
	}

	return result;
}

bool CContactList::canContactReceiveEmail( const std::string& key ) const
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		result = contact->canReceiveEmail();
	}

	return result;
}

bool CContactList::canContactDoFileTransfer( const std::string& key ) const
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		result = contact->canDoFileTransfer();
	}

	return result;
}
	
//VOXOX - JRT - 2009.07.26 
bool CContactList::addContactPhones( const std::string& key, Telephones& phones )
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		contact->getTelephones().Merge( phones );
		updateContact( *contact );
		
		result = true;
	}

	return result;
}
	
//VOXOX - JRT - 2009.07.26 
bool CContactList::addContactEmailAddresses( const std::string& key, EmailAddresses& emails )
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		contact->getEmailAddresses().Merge( emails );
		updateContact( *contact );
		
		result = true;
	}

	return result;
}

bool CContactList::addMessageTranslationSettings( const std::string& key, const MessageTranslation& messageTranslation )//VOXOX - CJC - 2010.01.21 
{
	bool result = false;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		contact->setMessageTranslation(messageTranslation);
		updateContact( *contact );
		
		result = true;
	}

	return result;
}

MessageTranslation CContactList::getMessageTranslationSettings(const std::string & key){

	MessageTranslation translation;

	Contact* contact = getContactByKey( key );
	
	if( contact )
	{
		translation =  contact->getMessageTranslation();
		
	}

	return translation;
}
void CContactList::getContactsWithHomeNumber( ContactInfoList2& info )
{
	_contactList.getContactsWithHomeNumber( info );
}

void CContactList::getContactsWithMobileNumber( ContactInfoList2& info )
{
	_contactList.getContactsWithMobileNumber( info );
}

void CContactList::getContactsWithSipNumber( ContactInfoList2& info )
{
	_contactList.getContactsWithSipNumber( info );
}

void CContactList::getContactsWithWengoCall( ContactInfoList2& info )
{
	_contactList.getContactsWithWengoCall( info );
}

void CContactList::getContactsThatCanChat( ContactInfoList2& info )
{
	_contactList.getContactsThatCanChat( info );
}


StringList CContactList::getContactIds() const 
{
	return _contactList.getContactIds();
}

void CContactList::addContact(const ContactProfile & contactProfile) 
{
	typedef ThreadEvent1<void (ContactProfile contactProfile), ContactProfile> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::addContactThreadSafe, this, _1), contactProfile);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::addContactThreadSafe(ContactProfile contactProfile) 
{
	ensureGroup( contactProfile );		//VOXOX - JRT - 2009.07.14 

	Contact* contact = _contactList.createContact();

//	*contact = contactProfile;
	contact->copyJustData( contactProfile );	//VOXOX - JRT - 2009.07.13 

	//VOXOX - JRT - 2009.07.13 - HACK - clear out IMContactSet so next line can handle changes.
	contact->getIMContactSetNonConst().clear();
	contact->handleContactSetChanges( contactProfile.getIMContactSet() );		//VOXOX - JRT - 2009.07.13 
	contact->getIMContactSetNonConst() = contactProfile.getIMContactSet();		//Now add them back.  Needed for incoming. HACK!!!!!!!!!!!!!!

	//VOXOX - JRT - 2009.08.13 - ContactList::addContact() is looking for info in Groups, not ContactGroupInfo, so set it here.
	contact->getGroups().DeleteAll();
	const ContactGroupInfoSet& cgis = contactProfile.getContactGroupInfoSetConst();
	
	for ( ContactGroupInfoSet::const_iterator it = cgis.begin(); it != cgis.end(); it++ )
	{
		ContactGroup* cg = getContactGroup( (*it).second.getKey() );

		if ( cg )
		{
			Group grp( cg->getName(), cg->getType() );
			contact->getGroups().Add( &grp );
		}
	}

	Contact* pContact = _contactList.addContact( contact );						//ContactList will delete 'contact'
	saveContactProfile( *pContact );							//VOXOX - JRT - 2009.09.23 - Sync with server. (If from CM, then we may have ContactProfile data).

	LOG_DEBUG("adding new contact=" + pContact->getFirstName() + " in group=" + pContact->getFirstUserGroupId());	//VOXOX - JRT - 2009.05.06 JRT-GRPS OK
}


//VOXOX - JRT - 2009.07.14 
void CContactList::ensureGroup( ContactProfile& contactProfile )
{
	std::string groupName = "Buddies";
	bool		hasGroups = contactProfile.hasGroups();

	//VOXOX - JRT - 2009.08.21 - get best group name.
	if ( !hasGroups )
	{
		Groups& groups = contactProfile.getGroups();

		if ( groups.GetCount() > 0 )
		{
			for ( Groups::const_iterator it = groups.begin(); it != groups.end(); it++ )
			{
				groupName = (*it).getName();
				break;
			}
		}
	}

	if ( !hasGroups )
	{
		ContactGroup* contactGroup = NULL;
		ContactGroup cg( groupName, EnumGroupType::GroupType_User );

		contactGroup = getContactList().getContactGroupSetRef().FindByKey( cg.getKey() );

		if ( !contactGroup )
		{
			contactGroup = getContactList().getContactGroupSetRef().Add( &cg );
		}

		if ( contactGroup )
		{
			ContactGroupInfo cgi( contactGroup->getKey(), contactGroup->getType() );
			contactProfile.getContactGroupInfoSet().Add( cgi );
		}
	}
}

void CContactList::removeContact(const std::string & contactId) {
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::removeContactThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::removeContactThreadSafe(std::string contactId) {
	Contact * contact = getContact(contactId);
	if (contact) {
		_contactList.removeContact(*contact);
	}
}

void CContactList::blockContact(const std::string & contactId) {
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::blockContactThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}
void CContactList::blockContactThreadSafe(std::string contactId) {
	Contact * contact = getContact(contactId);
	if (contact) {
		contact->block();
	}
}

void CContactList::unblockContact(const std::string & contactId) {
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::unblockContactThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::unblockContactThreadSafe(std::string contactId) {
	Contact * contact = getContact(contactId);
	if (contact) {
		contact->unblock();
	}
}

void CContactList::updateContact(const ContactProfile & contactProfile)
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent1<void (ContactProfile contactProfile), ContactProfile> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::updateContactThreadSafe, this, _1), contactProfile);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::updateContactThreadSafe(ContactProfile contactProfile) {
	Contact * contact = getContact(contactProfile.getKey());
	if (contact) 
	{
		(*contact) = contactProfile;			//Saves in memory.  Will be serialized at later time.
		saveContactProfile( contactProfile );	//Save on server.
	}
}

//void CContactList::updateContactGroups( const ContactProfile & contactProfile, const ContactGroupInfoSet& oldGroups, const ContactGroupInfoSet& newGroups )	//VOXOX - JRT - 2009.09.23 
//{
//	ContactGroupInfoSet::const_iterator itOld = oldGroups.begin();
//	ContactGroupInfoSet::const_iterator itNew = newGroups.begin();
//
//	if ( const_cast<ContactGroupInfoSet&>(oldGroups).GetUserCount() == 1 && 
//		 const_cast<ContactGroupInfoSet&>(newGroups).GetUserCount() == 1 )
//	{
//		if ( (*itOld).second.getKey() != (*itNew).second.getKey() )
//		{
//			moveContact( contactProfile, (*itOld).second.getKey(), (*itNew).second.getKey() );
//		}
//	}
//	else	//We have multiple group changes.
//	{
//
//	}
//
//	//	updateContact(contactProfile);
//}

//VOXOX - JRT - 2009.05.06 - JRT-GRPS
void CContactList::moveContact( const ContactProfile & contactProfile, const std::string& oldGroupId, const std::string newGroupId )
{
	const_cast<ContactProfile&>(contactProfile).removeFromGroup( oldGroupId );
	const_cast<ContactProfile&>(contactProfile).addToGroup     ( newGroupId, EnumGroupType::GroupType_User );	//JRT-GRPS
	updateContact(contactProfile);
}

void CContactList::moveContactToGroup( const std::string& contactKey, const std::string& srcGroupName, const std::string& newGroupId, bool bIsGroupName ) 
{
	std::string destGroupName = "";

	if ( bIsGroupName )
	{
//		ContactGroup contactGroup( newGroupId, EnumGroupType::GroupType_User);
//		addContactGroup( contactGroup );
		//_contactList.moveContactToGroup() will handle the add.
		destGroupName = newGroupId;
	}
	else
	{
		ContactGroup* pGroup = this->_contactList.getContactGroupSetRef().FindByKey( newGroupId  );
		destGroupName = (pGroup ? pGroup->getName() : "" );
	}

	Contact* pContact = getContactList().getContactByKey( contactKey );
	if ( pContact )
	{
		_contactList.moveContactToGroup( srcGroupName, destGroupName, *pContact );
	}
	else
	{
		assert(false);		//Invalid contactKey.
	}
}
//End VoxOx

//void CContactList::addContactGroup(const std::string & name) {
void CContactList::addContactGroup(const ContactGroup& group) {	//VOXOX - JRT - 2009.05.07 JRT-GRPS
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	/*
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::addContactGroupThreadSafe, this, _1), name);
	_modelThread.postEvent(event);
	*/
	//FIXME: Here we do not change the thread because QtProfileDetails needs this method to be blocking
	//We should change the code in QtProfileDetails
//	_contactList.addContactGroup(name);
	_contactList.addContactGroup( group );	//VOXOX - JRT - 2009.05.07 JRT-GRPS
}

//void CContactList::addContactGroupThreadSafe(std::string name) {
//	_contactList.addContactGroup(name);
//}
void CContactList::addContactGroupThreadSafe( const ContactGroup& group) {
	_contactList.addContactGroup(group);
}

void CContactList::removeContactGroup(const std::string & groupId) {
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::removeContactGroupThreadSafe, this, _1), groupId);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::removeContactGroupThreadSafe(std::string groupId) {
	_contactList.removeContactGroup(groupId);
}

void CContactList::renameContactGroup(const std::string & groupId, const std::string & name) {
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent2<void (std::string contactId, std::string name), std::string, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::renameContactGroupThreadSafe, this, _1, _2), groupId, name);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::renameContactGroupThreadSafe(std::string groupId, std::string name) {
	_contactList.renameContactGroup(groupId, name);
}

void CContactList::merge(const std::string & dstContactId, const std::string & srcContactId) {
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent2<void (std::string dstContactId, std::string srcContactId), std::string, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::mergeThreadSafe, this, _1, _2), dstContactId, srcContactId);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::mergeThreadSafe(std::string dstContactId, std::string srcContactId) {
	Contact * dstContact = getContact(dstContactId);
	Contact * srcContact = getContact(srcContactId);
	if (dstContact && srcContact) {
		_contactList.mergeContacts(*dstContact, *srcContact);
	}
}

void CContactList::orderGroups( const std::string & dragGroupId, const std::string & dropGroupId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent2<void (std::string dstContactId, std::string srcContactId), std::string, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::orderGroupsThreadSafe, this, _1, _2), dragGroupId, dropGroupId);
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::orderGroupsThreadSafe( std::string dragGroupId, const std::string dropGroupId) 
{
	_contactList.orderGroups( dragGroupId, dropGroupId);
}

std::string CContactList::findContactThatOwns(const IMContact & imContact) {
	std::string result;

	Contact * contact = _contactList.findContactThatOwns(imContact);
	if (contact) {
		result = contact->getKey();	//VOXOX - JRT - 2009.04.26 
	}

	return result;
}

ContactGroupSet& CContactList::getContactGroupSet()	
{ 
	return _contactList.getContactGroupSetRef(); 
}


//VOXOX - JRT - 2009.05.26 
void CContactList::createWsContactProfile()
{
	if ( !_contactList.getWsContactProfile() ) 
	{
		_contactList.createWsContactProfile();
	}
}

void CContactList::wsContactProfileCreatedEventHandler( ContactList& sender, WsContactProfile& wsContactProfile ) 
{
	if ( _contactList.getWsContactProfile() ) 
	{
		_cWsContactProfile = new CWsContactProfile(_cWengoPhone, wsContactProfile);
	}
}

void CContactList::saveContactProfile( const ContactProfile& profile ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent1<void (ContactProfile profile), ContactProfile> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::saveContactProfileThreadSafe, this, _1), profile );
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::saveContactProfileThreadSafe( const ContactProfile& profile ) 
{
	_cWsContactProfile->setContactProfile( profile );
}


void CContactList::populateFilterGroup( const std::string& text, const std::string& filterName ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.09.22 

	if ( isDestroying() )						//VOXOX - JRT - 2009.09.22 
		return;

	typedef ThreadEvent2<void (std::string text, std::string filterName), std::string, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CContactList::populateFilterGroupThreadSafe, this, _1, _2), text, filterName );
	WengoPhone::getInstance().postEvent(event);
}

void CContactList::populateFilterGroupThreadSafe( const std::string text, const std::string& filterName )
{
	_contactList.populateFilterGroup( text, filterName );
}

//VOXOX - JRT - 2009.09.03 
int	CContactList::getHowManyContactsMustMove( const std::string & groupId )	
{
	int result = 0;

	ContactGroup* cg = getContactGroupSet().FindByKey( groupId );

	if ( cg )
	{
		result = cg->getHowManyContactsMustMove();
	}

	return result;
}
