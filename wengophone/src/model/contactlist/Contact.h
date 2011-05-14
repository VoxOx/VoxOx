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

#ifndef OWCONTACT_H
#define OWCONTACT_H

#include "ContactProfile.h"
#include "../profile/Components.h"				//VOXOX - 2009.03.31
#include <util/Trackable.h>

#include <string>

/**
 * Contact inside an address book.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class Contact : public ContactProfile, public Trackable {
	friend class ContactList;
	friend class ContactXMLSerializer;
	friend class ContactXMLSerializer1;
public:

	/**
	 * A Contact has been modified/updated.
	 *
	 * @param sender the Contact modified/updated
	 */
	Event<void (Contact & sender)> contactChangedEvent;

	Contact(UserProfile & userProfile);

	Contact(const Contact & contact);

	Contact & operator=(const Contact & contact);

	Contact & operator=(const ContactProfile & contactProfile);

	~Contact();

	bool operator==(const Contact & contact) const;

	/**
	 * Set the preferred phone number (can also be a wengo id or a sip address).
	 *
	 * @param number the preferred phone number
	 */
	void setPreferredPhoneNumber(const std::string & number) { _preferredNumber = number; contactChangedEvent(*this); }

	/**
	 * Set the preferred IMContact to use.
	 *
	 * @param the imContact to set. The given reference must stay valid during
	 * the execution (this must be a reference to an IMContact of this Contact).
	 */
	void setPreferredIMContact(const IMContact & imContact) { _preferredIMContact = (IMContact *)&imContact; contactChangedEvent(*this); }

	/**
	 * Add an IMContact to the Contact.
	 *
	 * This method will send a request to ContactList that will send a request
	 * to the appropriated IMContactList. Then the ContactList will call
	 * _addIMContact that will actually add the IMContact.
	 *
	 * @param imContact IMContact to add
	 */
	void addIMContact(const IMContact & imContact);

	/**
	 * Remove an IMContact from the Contact.
	 *
	 * The process is the same as in addIMContact
	 *
	 * @param imContact IMContact to remove
	 */
	void removeIMContact(const IMContact & imContact);

	/**
	 * Check if an IMContact of the Contact is equivalent to the given
	 * IMContact.
	 *
	 * Equivalent IMContacts are of the same protocol. If an equivalent
	 * IMContact has been found and if it has no associated IMAccount,
	 * the IMAccount of the given IMContact is linked to the found
	 * IMContact.
	 *
	 * @param imContact the IMContact to check
	 * @return true if an equivalent IMContact has been found.
	 */
	bool checkAndSetIMContact(const IMContact & imContact);

	/**
	 * Gets the first valid IMContact given an IMAccount.
	 *
	 * A valid IMContact is a connected IMContact linked to the given IMAcccount.
	 *
	 * @return the found IMContact or NULL if not found.
	 */
	const IMContact * getFirstValidIMContact(const IMAccount & imAccount) const;

	/**VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	 * Avoid this contact to see my presence.
	 */
	void block();

	/**VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	 * Authorize this contact to see my presence.
	 */
	void unblock();

	UserProfile & getUserProfile() const {
		return _userProfile;
	}

	//VOXOX - JRT - 2009.03.31 - Add some accessors
//	std::string	getName();
	//End VOXOX.

	//VOXOX - JRT - 2009.03.31 - Add new functionality
	//void setName(const std::string& str) { _name = str; }
	//std::string getName() const { return _name; }

	//void setNickname(const std::string& str) { _nickname = str; }
	//std::string getNickname() const { return _nickname; }

	//void setMergedContact(const std::string& str) { _mergedContactUuid = str; }
	//std::string getMergedContact() const { return _mergedContactUuid; }
	//
	//	  MergedContacts&	getMergedContacts()					{ return _mergedContacts;	}
	//const MergedContacts&	getMergedContactsConst() const		{ return _mergedContacts;	}

	void  copyJustData   ( const ContactProfile& src );
	void  copyJustProfile( const ContactProfile& src );
	std::string getNetworkName();
	bool containsText( const std::string& text );
	bool startsWith( const std::string& text, const std::string& tgtValue, std::string& tgtString );
	//End VOXOX

	//VOXOX - JRT - 2009.07.13 
	void handleContactSetChanges( const IMContactSet& newImContactSet );
	void handleContactSetChanges( const IMContactSet& oldImContactSet, const IMContactSet& newImContactSet );

	bool isOnline()			 const;		//VOXOX - JRT - 2009.07.26 
	bool canBeCalled()		 const;		//VOXOX - JRT - 2009.07.26 
	bool canReceiveSMS()	 const;		//VOXOX - JRT - 2009.07.26 
	bool canReceiveFax()	 const;		//VOXOX - JRT - 2009.07.26 
	bool canReceiveEmail()	 const;		//VOXOX - JRT - 2009.07.26 
	bool canDoFileTransfer() const;		//VOXOX - JRT - 2009.07.27 

private:

	/**
	 * Copy a Contact.
	 */
	virtual void copy(const Contact & contact);

	/**
	 * Copy a ContactProfile.
	 */
	virtual void copy(const ContactProfile & contactProfile, bool justData = false );

	/**
	 * @see Profile::profileChangedEvent
	 */
	void profileChangedEventHandler(Profile & profile);

	/**
	 * @see IMContact::imContactChangedEvent
	 */
	void imContactChangedEventHandler(IMContact & sender);

	/**
	 * Actually add an IMContact to this Contact.
	 *
	 * This method must be called only by ContactList
	 *
	 * The IMContact is copied internally
	 *
	 * @param the IMContact to add
	 */
	void _addIMContact(const IMContact & imContact);

	/**
	 * Actually remove an IMContact to this Contact.
	 *
 	 * This method must be called only by ContactList
 	 *
	 * @param the IMContact to remove
	 */
	void _removeIMContact(const IMContact & imContact);

	/** Can't be used on a Contact.*/
	void setIcon(const OWPicture & icon);

	/**
	 * Merges this Contact with another.
	 *
	 * @param contact the Contact to merge with
	 */
	void merge(const Contact & contact);

	UserProfile & _userProfile;

	ContactList & _contactList;

	//VOXOX - JRT - 2009.03.31 - Add functionality
//	std::string		_name;
//	std::string		_nickname;
//
//	std::string		_mergedContactUuid;		//UUID of parent merged contact
//	MergedContacts	_mergedContacts;		//Hold child merged contacts.

//	Groups			_groups;				//This should always have at least ONE entry.  If not, force one.
	//End VOXOX
};

#endif //OWCONTACT_H
