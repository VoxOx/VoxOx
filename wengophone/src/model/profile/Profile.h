/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006 Wengo
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

#ifndef PROFILE_H
#define PROFILE_H

#include "EnumSex.h"
#include "StreetAddress.h"
#include "Components.h"

#include <imwrapper/EnumPresenceState.h>
#include <imwrapper/IMAccount.h>		//VOXOX - JRT - 2009.08.03 

#include <util/Date.h>
#include <util/Event.h>
#include <util/Interface.h>
#include <util/OWPicture.h>
#include <util/List.h>

class ProfileData;

/**
 * Handle common data between a Contact and UserProfile.
 *
 * @author Philippe Bernery
 * @author Mathieu Stute
 */
class Profile : Interface {
	friend class ProfileXMLSerializer;
	friend class ProfileData;			//VOXOX - JRT - 2009.08.03 - We should NOT need this.

public:
	/**
	 * Emitted when a property has been changed.
	 *
	 * @param sender this class
	 */
	Event< void(Profile & sender) > profileChangedEvent;

	Profile();

	Profile(const Profile & profile);

	Profile& operator=( const Profile& src );	//VOXOX - JRT - 2009.08.03 

	bool operator==(const Profile & profile) const;

	virtual ~Profile() {}

	void initLists();

	void setFirstName(const std::string & firstName) { _firstName = firstName; profileChangedEvent(*this); }
	std::string getFirstName() const { return _firstName; }

	void setLastName(const std::string & lastName) { _lastName = lastName; profileChangedEvent(*this); }
	std::string getLastName() const { return _lastName; }

	std::string getCompleteName() const { return _firstName + " " + _lastName; }

	void setSex(EnumSex::Sex sex) { _sex = sex; profileChangedEvent(*this); }
	EnumSex::Sex getSex() const { return _sex; }

	void setBirthdate(const Date & birthdate) { _birthdate = birthdate; profileChangedEvent(*this); }
	Date getBirthdate() const { return _birthdate; }

	void setCompany(const std::string & company) { _company = company; profileChangedEvent(*this); }
	std::string getCompany() const { return _company; }

	void setTitle(const std::string & title ) { _title = title; profileChangedEvent(*this); }
	std::string getTitle() const { return _title; }

	//VOXOX - JRT - 2009.03.24 - Replace separate URL entries with a list.  Retain set/get accessors.
	Urls& getUrls()										{ return _urls;	}
	const Urls& getUrlsConst() const					{ return _urls;	}


	//This is 'main'
	virtual void setWebsite( const std::string & url ) { getUrls().setUrl( "main",  url.c_str() ); profileChangedEvent(*this); }
	std::string getWebsite()							{ return getUrls().getUrl( "main"  ); }


	//VOXOXCHANGE CJC
	//ADD VALUE TO SAVE VOXOX NUMBER
	//VOXOX - JRT - 2009.03.24 - Replace separate phone entries with a list.  Retain set/get accessors.
	Telephones& getTelephones()							{ return _telephones;	}
	const Telephones& getTelephonesConst() const		{ return _telephones;	}

	MessageTranslation getMessageTranslation()			{return _messageTranslation; }
	void  setMessageTranslation(const MessageTranslation & messageTranslation){_messageTranslation = messageTranslation;}
	virtual void setVoxOxPhone		( const std::string & number ) { getTelephones().setNumber( "voxox",  number.c_str() ); profileChangedEvent(*this); }
	virtual void setMobilePhone		( const std::string & number ) { getTelephones().setNumber( "mobile", number.c_str() ); profileChangedEvent(*this); }
	virtual void setHomePhone		( const std::string & number ) { getTelephones().setNumber( "home",   number.c_str() ); profileChangedEvent(*this); }
	virtual void setWorkPhone		( const std::string & number ) { getTelephones().setNumber( "work",   number.c_str() ); profileChangedEvent(*this); }
	virtual void setOtherPhone		( const std::string & number ) { getTelephones().setNumber( "other",  number.c_str() ); profileChangedEvent(*this); }
			void setWengoPhoneNumber( const std::string & number ) { getTelephones().setNumber( "wengo",  number.c_str() ); profileChangedEvent(*this); }
			void setFax				( const std::string & number ) { getTelephones().setNumber( "fax",    number.c_str() ); profileChangedEvent(*this); }
	
	std::string getVoxOxPhone()		  	{ return getTelephones().getNumber( "voxox"  ); }
	std::string getMobilePhone()	  	{ return getTelephones().getNumber( "mobile" ); }
	std::string getHomePhone()		  	{ return getTelephones().getNumber( "home"   ); }
	std::string getWorkPhone()		  	{ return getTelephones().getNumber( "work"   ); }
	std::string getOtherPhone()		  	{ return getTelephones().getNumber( "other"  ); }
	std::string getWengoPhoneNumber() 	{ return getTelephones().getNumber( "wengo"  ); }
	std::string getFax()			  	{ return getTelephones().getNumber( "fax"    ); }

	bool hasPhoneNumber() const			{ return (getTelephonesConst().size() > 0 );	}
	//End VOXOX change.

	//Email
	//VOXOX - JRT - 2009.03.24 - Replace separate email address entries with a list.  Retain set/get accessors.
	EmailAddresses& getEmailAddresses()							{ return _emailAddresses;	}
	const EmailAddresses& getEmailAddressesConst() const		{ return _emailAddresses;	}

	void setPersonalEmail(const std::string & emailAddress) { getEmailAddresses().setEmailAddress( "personal", emailAddress.c_str() ); profileChangedEvent(*this); }
	void setWorkEmail	 (const std::string & emailAddress)	{ getEmailAddresses().setEmailAddress( "work",     emailAddress.c_str() ); profileChangedEvent(*this); }
	void setOtherEmail	 (const std::string & emailAddress)	{ getEmailAddresses().setEmailAddress( "other",    emailAddress.c_str() ); profileChangedEvent(*this); }

	std::string getPersonalEmail()	{ return getEmailAddresses().getEmailAddress( "personal" ); }
	std::string getWorkEmail()		{ return getEmailAddresses().getEmailAddress( "work"     ); }
	std::string getOtherEmail()		{ return getEmailAddresses().getEmailAddress( "other"    ); }
	//End VOXOX change
	

	//Street address
	//VOXOX - JRT - 2009.03.24 - Replace separate Street address entries with a list.  Retain set/get accessors.
	StreetAddresses& getStreetAddresses()							{ return _streetAddresses;	}
	const StreetAddresses& getStreetAddressesConst() const			{ return _streetAddresses;	}

	void setStreetAddress(const StreetAddress & streetAddress)		{ getStreetAddresses().setStreetAddress( streetAddress ); profileChangedEvent(*this); }
	StreetAddress getStreetAddress()								{ return (getStreetAddresses().findOrCreateByType( "main" )); }	//JRT - 2009.03.25 - removed const.
	//End VOXOX change

	void				setNotes(const std::string & notes)			{ _notes = notes; profileChangedEvent(*this); }
	std::string			getNotes() const							{ return _notes; }

	void				setSmsSignature(const std::string & signature)		{ _smsSignature = signature; profileChangedEvent(*this); }
	std::string			getSmsSignature() const								{ return _smsSignature; }

	virtual void		setAlias(const std::string & alias)			{ _alias = alias; profileChangedEvent(*this); }
	virtual std::string getAlias() const							{ return _alias; }

	//VOXOX - JRT - 2009.05.21 
	virtual void		setTimeZone(const int tz )					{ _timeZone = tz; profileChangedEvent(*this); }
	virtual int			getTimeZone() const							{ return _timeZone; }

	virtual void		setDescription(const std::string& str )		{ _description = str; profileChangedEvent(*this); }
	virtual std::string getDescription() const						{ return _description; }

	virtual void		setInterests(const std::string& str )		{ _interests = str; profileChangedEvent(*this); }
	virtual std::string getInterests() const						{ return _interests; }

	virtual void		setLanguage(const std::string& str )		{ _language = str; profileChangedEvent(*this); }
	virtual std::string getLanguage() const							{ return _language; }

	virtual void		setIsSearchable(const bool val )			{ _searchable = val; profileChangedEvent(*this); }
	virtual bool		isSearchable() const						{ return _searchable; }

	virtual void		setIsSystemEntry(const bool val )			{ _systemEntry = val; profileChangedEvent(*this); }
	virtual bool		isSystemEntry() const						{ return _systemEntry; }

	void				setTimestamp( int val )						{ _timestamp = val;	}
	int					getTimestamp() const						{ return _timestamp;	}
	//End VoxOx

	//VOXOX CHANGE CJC STATUS MESSAGE
	virtual void		setStatusMessage(const std::string & statusMessage) { _statusMessage = statusMessage; profileChangedEvent(*this); }
	virtual std::string getStatusMessage() const							{ return _statusMessage; }

	virtual void		setWengoPhoneId(const std::string & wengoPhoneId)	{ _wengoPhoneId = wengoPhoneId; profileChangedEvent(*this); } 
	virtual std::string getWengoPhoneId() const								{ return _wengoPhoneId; } 

	virtual void		setIcon   ( const OWPicture& icon )					{ _icon = icon; profileChangedEvent(*this); }
	virtual void		updateIcon( const OWPicture& icon )					{ setIcon( icon); }
//	virtual OWPicture	getIcon() const										{ return _icon; }
	virtual const OWPicture&	getIcon() const								{ return _icon; }	//VOXOX - JRT - 2009.08.17 
//	virtual const OWPicture&	getIconRef() const							{ return _icon; }


	void setId( int val )													{ _id = val;	}	//VOXOX - JRT - 2009.06.28 
	int	 getId() const														{ return _id;	}

	//VOXOX - JRT - 2009.08.03 - add so we have a simple means to identify this Profile without all the events and other baggage.
	//							 In most cases, this will be ignored.
	void		setIMAccount( const IMAccount& imAccount )					{ _imAccount = imAccount;	}
	IMAccount&	getIMAccount()												{ return _imAccount;		}


	/**
	 * Gets the PresenceState of this Profile.
	 *
	 * @return the PresenceState
	 */
	virtual EnumPresenceState::PresenceState getPresenceState() const = 0;

//protected:
public:	//VOXOX - JRT - 2009.08.03 - For some reason, the copy is not accessible in the derived class ProfileData.
	/**
	 * Copy a Profile.
	 *
	 * @param profile the Profile to copy
	 */
	virtual void copy(const Profile & profile, bool copyAvatar = true );


private:
	int				_id;				//For use with JSON

	IMAccount		_imAccount;			//VOXOX - JRT - 2009.08.03 - Simply for identification info.

	std::string		_fullName;			//VOXOX - JRT - 2009.05.21 
	std::string		_firstName;
	std::string		_lastName;
	std::string		_alias;

	std::string		_company;
	std::string		_title;
	std::string		_notes;
	
	std::string		_statusMessage;		//VOXOX CHANGE CJC STATUS MESSAGE
	std::string		_wengoPhoneId;
	std::string		_smsSignature;

	EnumSex::Sex	_sex;
	Date			_birthdate;
	OWPicture		_icon;

	int				_timeZone;
	std::string		_description;
	std::string		_interests;
	std::string		_language;
	bool			_searchable;
	bool			_systemEntry;
	int				_timestamp;			//VOXOX - JRT - 2009.06.03 - for syncing

	Telephones		_telephones;
	EmailAddresses	_emailAddresses;
	StreetAddresses	_streetAddresses;
	Urls			_urls;
	//VOXOX - CJC - 2010.01.21 
	MessageTranslation _messageTranslation;
//	WorkPlaces		_workPlaces;		//VOXOX - JRT - 2009.05.21	//TODO
};

//=============================================================================
//VOXOX - JRT - 2009.08.03 - Sometimes we just need the data without the presence and all the other events.

class ProfileData : public Profile
{
private:
	EnumPresenceState::PresenceState getPresenceState() const	{ return EnumPresenceState::PresenceStateUnknown;	}
};


#endif	//PROFILE_H
