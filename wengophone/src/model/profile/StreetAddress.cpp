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
#include "StreetAddress.h"

using namespace std;

StreetAddress::StreetAddress() 
{
	init();
}

StreetAddress::StreetAddress(const StreetAddress & streetAddress) 
{
	copy(streetAddress);
}

StreetAddress::~StreetAddress() 
{
}

StreetAddress & StreetAddress::operator = (const StreetAddress & streetAddress) {
	if (&streetAddress != this) 
	{
		copy(streetAddress);
	}

	return *this;
}

bool StreetAddress::operator == (const StreetAddress & streetAddress) const {
	return (   (_street1		== streetAddress._street1	)
			&& (_street2		== streetAddress._street2	)
			&& (_street3		== streetAddress._street3	)
			&& (_city			== streetAddress._city		)
			&& (_stateProvince	== streetAddress._stateProvince)
			&& (_postalCode		== streetAddress._postalCode	)
			&& (_country		== streetAddress._country	));
}

void StreetAddress::copy(const StreetAddress & streetAddress) 
{
	_type			= streetAddress._type;		//VOXOX - JRT - 2009.04.05
	_street1		= streetAddress._street1;
	_street2		= streetAddress._street2;
	_street3		= streetAddress._street3;
	_city			= streetAddress._city;
	_stateProvince	= streetAddress._stateProvince;
	_postalCode		= streetAddress._postalCode;
	_country		= streetAddress._country;
	_visibility		= streetAddress._visibility;
}

//-----------------------------------------------------------------------------

void StreetAddress::init()
{
	_type			= "";
	_street1		= "";
	_street2		= "";
	_street3		= "";
	_city			= "";
	_stateProvince	= "";
	_postalCode		= "";
	_country		= "";
	_visibility		= EnumVisibility::VisibilityUnknown;
}

//-----------------------------------------------------------------------------

void StreetAddress::verifyType()
{
	if ( getType().empty() )
	{
		setType( "main");
	}
}

//-----------------------------------------------------------------------------

bool StreetAddress::isValid() const
{
	bool bValid = false;

	if ( !getStreet1().empty() ||
		 !getStreet2().empty() ||
		 !getStreet3().empty() ||
		 !getCity().empty()	   ||
		 !getStateProvince().empty() ||
		 !getCountry().empty() ||
		 !getPostalCode().empty() )
	{
		bValid = true;
	}

	return bValid;
}

//End VOXOX

//=============================================================================

void StreetAddresses::setStreetAddress( const char* type, const char* street1, 
									    const char* street2, const char* street3,
									    const char* locality, const char* region, 
										const char* postalCode, const char* country,
										EnumVisibility::Visibility vis )
{
	StreetAddress* pAddr = findByType( type );

	if ( pAddr )
	{
		pAddr->setType			( type		  );
		pAddr->setStreet1		( street1	  );
		pAddr->setStreet2		( street2	  );
		pAddr->setStreet3		( street3	  );
		pAddr->setCity			( locality	  );
		pAddr->setStateProvince	( region	  );
		pAddr->setPostalCode	( postalCode  );
		pAddr->setCountry		( country	  );
		pAddr->setVisibility	( vis		  );
	}
	else
	{
		StreetAddress addr;

		addr.setType		 ( type		  );
		addr.setStreet1		 ( street1	  );
		addr.setStreet2		 ( street2	  );
		addr.setStreet3		 ( street3	  );
		addr.setCity		 ( locality	  );
		addr.setStateProvince( region	  );
		addr.setPostalCode	 ( postalCode );
		addr.setCountry		 ( country	  );
		addr.setVisibility	 ( vis		  );

		Add( &addr );
	}
}

//-----------------------------------------------------------------------------

void StreetAddresses::setStreetAddress( const StreetAddress& sa )
{
	setStreetAddress( sa.getType().c_str(),			sa.getStreet1().c_str(),
					  sa.getStreet2().c_str(),		sa.getStreet3().c_str(),
					  sa.getCity().c_str(),			sa.getStateProvince().c_str(),
					  sa.getPostalCode().c_str(),	sa.getCountry().c_str(),
					  sa.getVisibility() );
}

//-----------------------------------------------------------------------------

//StreetAddress* StreetAddresses::findByType( const char* type )
//{
//	StreetAddress* pRet = NULL;
//
//	for ( StreetAddresses::iterator iter = begin(); iter != end(); iter++ )
//	{
//		if ( iter->getType() == type )
//		{
//			pRet = &(*iter);
//			break;
//		}
//	}
//
//	return pRet;
//}

//-----------------------------------------------------------------------------

//void StreetAddresses::Merge( const StreetAddresses& src )
//{
//	StreetAddress* pAddr = NULL;
//
//	for ( StreetAddresses::const_iterator it = src.begin(); it != src.end(); it++ )
//	{
//		pAddr = findByType( (*it).getType().c_str() );
//
//		if ( pAddr )
//		{
//			if ( !(*pAddr == *it) )
//			{
//				*pAddr = *it;
//			}
//		}
//		else
//		{
//			this->Add( pAddr );
//		}
//	}
//}

//=============================================================================
