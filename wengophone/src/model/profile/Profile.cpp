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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "Profile.h"

#include <util/String.h>
#include <util/Logger.h>

Profile::Profile() 
{
	_sex			= EnumSex::SexUnknown;
	_firstName		= String::null;
	_lastName		= String::null;
	_alias			= String::null;

	_company		= String::null;
	_title			= String::null;
	_notes			= String::null;
	
	_statusMessage	= String::null;	//VOXOX CHANGE ADD STATUS MESSAGE
	_wengoPhoneId	= String::null;
	_smsSignature	= String::null;

	_timeZone		= 0;		//GMT
	_description	= String::null;
	_interests		= String::null;
	_language		= String::null;
	_searchable		= false;
	_systemEntry	= false;
	_timestamp		= 0;
	_id				= 0;	//VOXOX - JRT - 2009.06.28 
}

Profile::Profile(const Profile & profile)
	: Interface() {
	copy(profile);
}

void Profile::copy(const Profile& profile, bool copyAvatar ) 
{
	_firstName		= profile._firstName;
	_lastName		= profile._lastName;
	_alias			= profile._alias;

	_sex			= profile._sex;
	_birthdate		= profile._birthdate;
	_company		= profile._company;
	_title			= profile._title;
	_notes			= profile._notes;
	_wengoPhoneId	= profile._wengoPhoneId;
	
	_statusMessage	= profile._statusMessage;	//VOXOX CHANGE SUPPORT STATUS MESSAGE
	_smsSignature	= profile._smsSignature;

	if ( copyAvatar )
	{
		_icon			= profile._icon;
	}

	//VOXOX - JRT - 2009.05.21 
	_timeZone		= profile._timeZone;
	_description	= profile._description;
	_interests		= profile._interests;
	_language		= profile._language;
	_searchable		= profile._searchable;
	_systemEntry	= profile._systemEntry;
	_timestamp		= profile._timestamp;
	_imAccount		= profile._imAccount;	//VOXOX - JRT - 2009.08.03 

	_messageTranslation = profile._messageTranslation;//VOXOX - CJC - 2010.01.22 
	if ( profile._id > 0 )
	{
		_id				= profile._id;
	}

	getTelephones()		 = const_cast<Profile&>(profile).getTelephones();
	getEmailAddresses()  = const_cast<Profile&>(profile).getEmailAddresses();
	getStreetAddresses() = const_cast<Profile&>(profile).getStreetAddresses();
	getUrls()			 = const_cast<Profile&>(profile).getUrls();

}

//VOXOX - JRT - 2009.08.03 
Profile& Profile::operator=( const Profile& src )
{
	if ( this != &src )
	{
		copy( src );
	}

	return *this;
}

bool Profile::operator==(const Profile & profile) const 
{
	return ((_firstName == profile._firstName)
		&& (_lastName	== profile._lastName)
		&& (_sex		== profile._sex)
		&& (_birthdate	== profile._birthdate)
		&& (_company	== profile._company)
		&& (_title		== profile._title)

		&& (_notes		  == profile._notes)
		&& (_wengoPhoneId == profile._wengoPhoneId)
		&& (_smsSignature == profile._smsSignature)
		//VOXOX - JRT - 2009.03.24 
//		&& ( getTelephones() == const_cast<Profile&>(profile).getTelephones() )				//JRT - TODO: C2678 const issue
//		&& ( getEmailAddresses() == profile.getEmailAddresses() )		
//		&& ( getEmailAddresses() == const_cast<Profile&>(profile).getEmailAddresses() )		//JRT - TODO: C2678 const issue
//		&& ( getStreetAddresses() == const_cast<Profile&>(profile).getStreetAddresses() )	//JRT - TODO: C2678 const issue
//		&& ( getUrls == const_cast<Profile&>(profile).getUrls() )							//JRT - TODO: C2678 const issue
		);
}

void Profile::initLists()
{
	getStreetAddresses().DeleteAll();
	getEmailAddresses().DeleteAll();
	getTelephones().DeleteAll();
	getUrls().DeleteAll();
}
