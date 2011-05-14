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

#ifndef STREETADDRESS_H
#define STREETADDRESS_H

#include <string>

#include "VoxListTemplate.h"	//VOXOX - JRT - 2009.03.24
#include "EnumVisibility.h"

/**
 * Class for the living address of a Contact.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class StreetAddress {
	friend class StreetAddressXMLSerializer;
public:

	StreetAddress();

	StreetAddress(const StreetAddress & streetAddress);

	~StreetAddress();

	StreetAddress & operator=(const StreetAddress & streetAddress);

	bool operator == (const StreetAddress & streetAddress) const;

	void setStreet1(const std::string & street) { _street1 = street; }
	const std::string& getStreet1() const { return _street1; }

	void setStreet2(const std::string & street) { _street2 = street; }
	const std::string& getStreet2() const { return _street2; }

	void setStreet3(const std::string & street) { _street3 = street; }
	const std::string& getStreet3() const { return _street3; }

	void setStateProvince(const std::string & stateProvince) { _stateProvince = stateProvince; }
	const std::string& getStateProvince() const { return _stateProvince; }

	void setCity(const std::string & city) { _city = city; }
	const std::string& getCity() const { return _city; }

	void setPostalCode(const std::string & code)		{ _postalCode = code; }
	const std::string& getPostalCode() const { return _postalCode; }

	void setCountry(const std::string & country) { _country = country; }
	const std::string& getCountry() const { return _country; }

	//VOXOX - JRT - 2009.03.25
	void setType( const std::string & type )		{ _type = type;	}
	const std::string&	getType() const				{ return _type;	}

	void setVisibility( const EnumVisibility::Visibility vis )	{ _visibility = vis;	}
	EnumVisibility::Visibility	getVisibility() const			{ return _visibility;	}

	void verifyType();
	bool isValid() const;
	//End VOXOX change.

private:
	/** Factorizes code between contructor and copy contructor. */
	void copy(const StreetAddress & streetAddress);
	void init();

	std::string	_type;				//VOXOX - JRT - 2009.03.25
	std::string _street1;
	std::string _street2;
	std::string _street3;
	std::string _stateProvince;
	std::string _city;
	std::string _postalCode;
	std::string _country;

	EnumVisibility::Visibility	_visibility;	//VOXOX - JRT - 2009.03.25
};

//=============================================================================
//VOXOX - JRT - 2009.03.24
class StreetAddresses : public VoxListTemplate<StreetAddress>
{
public:
	void setStreetAddress(	const char* type, const char* street1,
							const char* street2, const char* street3,
							const char* locality, const char* region, 
							const char* postalCode, const char* country,
						   EnumVisibility::Visibility vis = EnumVisibility::VisibilityUnknown );	//TODO - Remove default param during code integration.

	void setStreetAddress( const StreetAddress& sa );

//	StreetAddress* findByType  ( const char* type );
//	void Merge( const StreetAddresses& src );
};
//End VOXOX
//=============================================================================

#endif	//STREETADDRESS_H
