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

#ifndef OWXPCOMUSERPROFILE_H
#define OWXPCOMUSERPROFILE_H

#include <presentation/PUserProfile.h>

class CUserProfile;
class Listener;

class XPCOMUserProfile : public PUserProfile {
public:

	XPCOMUserProfile(CUserProfile * cUserProfile);

	static CWengoPhone & getCUserProfile();

	void loginStateChangedEventHandler(SipAccount & sender,
		EnumSipLoginState::SipLoginState state) { }

	void networkDiscoveryStateChangedEventHandler(SipAccount & sender,
		SipAccount::NetworkDiscoveryState state) { }

	void authorizationRequestEventHandler(PresenceHandler & sender,
		const IMContact & imContact, const std::string & message) { }

	void updatePresentation() { }

private:

	void updatePresentationThreadSafe() { }

	void initThreadSafe() { }

	static CUserProfile * _cUserProfile;
};

#endif	//OWXPCOMUSERPROFILE_H
