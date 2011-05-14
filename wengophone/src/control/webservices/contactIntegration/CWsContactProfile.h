/*
	VOXOX!!!
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

#ifndef CWS_CONTACT_PROFILE_H
#define CWS_CONTACT_PROFILE_H

#include <model/webservices/contactIntegration/WsContactProfile.h>

#include <util/Trackable.h>

class CWengoPhone;

/**
 *
 * @Control to upload/download ContactProfiles
 * @author Jeff Theinert
 */
class CWsContactProfile : public Trackable 
{
public:
	CWsContactProfile( CWengoPhone& cWengoPhone, WsContactProfile& wsContactProfile );

	virtual ~CWsContactProfile();

	/**
	 * @see WsContactProfile
	 */
	Event<void (WsContactProfile& sender, int id, const std::string& contactId, int userNetworkId, int timestamp )> wsContactProfileSetEvent;
	Event<void (WsContactProfile& sender, int id, ContactProfile& contactProfile)>								    wsContactProfileGetEvent;

	/**
	 * @see WsContactProfile
	 */
	void setContactProfile( const ContactProfile& profile );
	void getContactProfile( const std::string& contactKey );

	void setUserProfile( const UserProfile& profile );

	CWengoPhone& getCWengoPhone();

//	void contactProfileGetEventHandler( WsContactProfile& sender, const ContactProfile& profile );
//	void contactProfileSetEventHandler( WsContactProfile& sender, const std::string& contactId );

private:
	WsContactProfile&	_wsContactProfile;
	CWengoPhone&		_cWengoPhone;
};

#endif /* CWS_CONTACT_PROFILE_H */
