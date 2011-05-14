/*
	VOXOX !!!
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include "stdafx.h"	
#include "CWsContactProfile.h"
#include <control/CWengoPhone.h>

CWsContactProfile::CWsContactProfile(CWengoPhone & cWengoPhone, WsContactProfile & wsContactProfile )
	:	_wsContactProfile( wsContactProfile ), 
		_cWengoPhone(cWengoPhone) 
{
	_wsContactProfile.wsContactProfileSetEvent += wsContactProfileSetEvent;
	_wsContactProfile.wsContactProfileGetEvent += wsContactProfileGetEvent;
}

CWsContactProfile::~CWsContactProfile() 
{
}

void CWsContactProfile::setContactProfile( const ContactProfile& profile )
{
	_wsContactProfile.setContactProfile( profile );
}

void CWsContactProfile::setUserProfile( const UserProfile& profile )
{
	_wsContactProfile.setUserProfile( profile );
}

//void CWsContactProfile::contactProfileGetEventHandler( WsContactProfile& sender, const ContactProfile& profile )
//{
//}
//
//void CWsContactProfile::contactProfileSetEventHandler( WsContactProfile& sender, const std::string& contactId )
//{
//}

CWengoPhone& CWsContactProfile::getCWengoPhone() 
{
	return _cWengoPhone;
}
