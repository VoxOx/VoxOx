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

#ifndef PURPLEENUMPRESENCESTATE_H
#define PURPLEENUMPRESENCESTATE_H

#include <imwrapper/EnumPresenceState.h>
#include <imwrapper/EnumIMProtocol.h>


class PurplePreState : EnumPresenceState
{
public:
	static const char* GetStatusId(PresenceState status, EnumIMProtocol::IMProtocol protocol);

	static PresenceState GetPresenceState(const char *StatusId);

private:
	static const char* GetOnlineStatusId	  ( EnumIMProtocol::IMProtocol protocol );
	static const char* GetOfflineStatusId	  ( EnumIMProtocol::IMProtocol protocol );
	static const char* GetInvisibleStatusId   ( EnumIMProtocol::IMProtocol protocol );
	static const char* GetAwayStatusId		  ( EnumIMProtocol::IMProtocol protocol );
	static const char* GetDoNotDisturbStatusId( EnumIMProtocol::IMProtocol protocol );

};

#endif	//PURPLEENUMPRESENCESTATE_H
