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

#ifndef OWIMCONTACTSET_H
#define OWIMCONTACTSET_H

#include <imwrapper/IMContact.h>
#include <serialization/Serializable.h>
#include <thread/RecursiveMutex.h>
#include <set>

class IMContactSet : public std::set<IMContact>, public Serializable {
public:
	//VOXOX - JRT - 2009.07.11 - For Serializable, but does nothing.
	std::string serialize();							
	bool		unserialize(const std::string & data);

	IMContactSet();
	IMContactSet( const IMContactSet& src );	
	virtual ~IMContactSet();								//So we can lock during destruction.

	IMContactSet& operator=( const IMContactSet& src );

	void add( const IMContact &imContact );
	bool Delete( const IMContact& imContact );

	std::string getFirstContactId();
	std::string getFirstDisplayContactId() const;
	std::string getFirstKey() const;
	std::string	getFirstStatusMessage() const;
	std::string getFirstAvailableSIPNumber() const ;
	std::string getFirstWengoId() const;
	std::string getFirstSkypeContactId() const;

	bool		hasAvailableSIPNumberAndNoWengoId() const;
	bool		hasIMContact( const IMContact& imContact ) const		{ return (findByContact(imContact) != NULL);	}

	IMContact* getFirstAvailable( const std::string imAccountId ) const;
	IMContact  getFirstAvailableWengoIMContact() const; //VOXOX - JRT - 2009.07.11 - TODO why is this not a ptr/reference?
	IMContact* getFirstValidAlias() const;

	IMContact*	getFirst     () const;
	IMContact*	findByContact( const IMContact& imContact ) const;
	IMContact*	findBy		 ( const std::string& accountId, const std::string& contactId, bool tryUserOnly = false) const;
	IMContact*	findByKey	 ( const std::string& key ) const;

//	IMContact * getPreferredIMContact() const;

	EnumIMProtocol::IMProtocol		getIMProtocol()   const;	
	QtEnumIMProtocol::IMProtocol	getQtIMProtocol() const;	

private:
	mutable RecursiveMutex _mutex;

};

#endif	//OWIMCONTACTSET_H
