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

#include "PurpleEnumPresenceState.h"

#include <cstring>

//VOXOX - JRT - 2009.09.10 - Avoid using #define when possible.  Better for debugging.
const char ST_AVAILABLE[]	= "available";
const char ST_OFFLINE[]		= "offline";
const char ST_INVISIBLE[]	= "invisible";
const char ST_AWAY[]		= "away";
const char ST_UNAVAILABLE[]	= "unavailable";
const char ST_EXT_AWAY[]	= "extended_away";
const char ST_BUSY[]		= "busy";				//Yahoo and MSN
const char ST_DND[]			= "dnd";

//VOXOX CHANGE (CJC?) all status of Skype are different than this.  That's why its not updating presence. 
//		These are dictated by the Skype API.  Adding the status the Skype expects
const char ST_SKYPE_ONLINE[]		= "ONLINE";
const char ST_SKYPE_AWAY[]			= "AWAY";
const char ST_SKYPE_UNAVAILABLE[]	= "DND";
const char ST_SKYPE_INVISIBLE[]		= "INVISIBLE";
const char ST_SKYPE_OFFLINE[]		= "OFFLINE";

//VOXOX - JRT - 2009.09.10 - There also appears to be issues with FaceBook and Twitter.
const char ST_TWIT_ONLINE[]			= "online";

//const char ST_FB_ONLINE[]			= "Online";
//const char ST_FB_OFFLINE[]			= "Offline"; //VOXOX - CJC - 2009.09.17 Not needed

//VOXOX - JRT - 2009.09.15 - NOTE: Facebook and Twitter only support 'offline' and 'available', so other
//							 presence status will be mapped to either of those.

//							 Jabber (in LibPurple) does not support 'invisible' but we are working on that.

//							 MySpace (in LibPurple) does not support 'unavailable'- Not sure what to do.

//-----------------------------------------------------------------------------

const char *PurplePreState::GetStatusId(EnumPresenceState::PresenceState status, EnumIMProtocol::IMProtocol protocol)
{
	const char* result = NULL;

	//TODO: move to QtEnumIMProtocolMap	//VOXOX - JRT - 2009.06.28 
	switch (status)
	{
		case EnumPresenceState::PresenceStateOffline:
			result = GetOfflineStatusId( protocol );
			break;

		case EnumPresenceState::PresenceStateOnline:
			result = GetOnlineStatusId( protocol );
			break;

		case EnumPresenceState::PresenceStateAway:
			result = GetAwayStatusId( protocol );
			break;

		case EnumPresenceState::PresenceStateDoNotDisturb:
			result = GetDoNotDisturbStatusId( protocol );
			break;

		case EnumPresenceState::PresenceStateInvisible:
			result = GetInvisibleStatusId( protocol );
			break;

			//VOXOX - JRT - 2009.09.15 - TODO: support for Extended_Away, Mobile, and Tune?

		default:
			result = ST_AWAY;	//VOXOX - JRT - 2009.09.15 - Odd choice of default.
	}

	return result;
}

//-----------------------------------------------------------------------------

const char* PurplePreState::GetOnlineStatusId( EnumIMProtocol::IMProtocol protocol )
{
	const char* result = NULL;

	switch( protocol )
	{
	case EnumIMProtocol::IMProtocolSkype:
		result = ST_SKYPE_ONLINE;
		break;

	case EnumIMProtocol::IMProtocolTwitter:
		result = ST_TWIT_ONLINE;
		break;

	/*case EnumIMProtocol::IMProtocolFacebook: //VOXOX - CJC - 2009.09.17 Facebook use ST_AVAILABLE
		result = ST_FB_ONLINE;
		break;*/

	default:
		result = ST_AVAILABLE;
		break;
	}

	return result;
}

//-----------------------------------------------------------------------------

const char* PurplePreState::GetOfflineStatusId( EnumIMProtocol::IMProtocol protocol )
{
	const char* result = NULL;

	switch( protocol )
	{
	case EnumIMProtocol::IMProtocolSkype:
		result = ST_SKYPE_OFFLINE;
		break;

	/*case EnumIMProtocol::IMProtocolFacebook: Facebook use ST_OFFLINE
		result = ST_FB_OFFLINE;
		break;*/

	default:
		result = ST_OFFLINE;
	}

	return result;
}

//-----------------------------------------------------------------------------

const char* PurplePreState::GetInvisibleStatusId( EnumIMProtocol::IMProtocol protocol )
{
	const char* result = NULL;

	switch( protocol )
	{
	case EnumIMProtocol::IMProtocolSkype:
		result = ST_SKYPE_INVISIBLE;
		break;

	//Presence not supported so map to online
	case EnumIMProtocol::IMProtocolFacebook:
	case EnumIMProtocol::IMProtocolTwitter:
		result = GetOnlineStatusId( protocol );
		break;

	default:
		result = ST_INVISIBLE;
	}

	return result;
}

//-----------------------------------------------------------------------------

const char* PurplePreState::GetAwayStatusId( EnumIMProtocol::IMProtocol protocol )
{
	const char* result = NULL;

	switch( protocol )
	{
	case EnumIMProtocol::IMProtocolSkype:
		result = ST_SKYPE_AWAY;
		break;

	//Presence not supported so map to online
//	case EnumIMProtocol::IMProtocolFacebook:		//Not sure about this.  Need to research
	case EnumIMProtocol::IMProtocolTwitter:
		result = GetOnlineStatusId( protocol );
		break;

	default:
		result = ST_AWAY;
	}

	return result;
}

//-----------------------------------------------------------------------------

const char* PurplePreState::GetDoNotDisturbStatusId( EnumIMProtocol::IMProtocol protocol )
{
	const char* result = NULL;

	switch( protocol )
	{
	case EnumIMProtocol::IMProtocolSkype:
		result = ST_SKYPE_UNAVAILABLE;
		break;

	case EnumIMProtocol::IMProtocolMSN:
	case EnumIMProtocol::IMProtocolYahoo:
		result = ST_BUSY;
		break;

	case EnumIMProtocol::IMProtocolAIM:
	case EnumIMProtocol::IMProtocolICQ:
	case EnumIMProtocol::IMProtocolJabber:
		result = ST_DND;
		break;

	//Presence not supported so map to online
	case EnumIMProtocol::IMProtocolFacebook:
	case EnumIMProtocol::IMProtocolTwitter:
		result = GetOnlineStatusId( protocol );
		break;

	default:
		result = ST_UNAVAILABLE;
	}

	return result;
}

//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------

EnumPresenceState::PresenceState PurplePreState::GetPresenceState(const char *StatusId)
{
	if (!strcmp(StatusId, ST_AVAILABLE   ) || 
		!strcmp(StatusId, ST_SKYPE_ONLINE) || 
		!strcmp(StatusId, ST_TWIT_ONLINE ) /*||*/
		/*!strcmp(StatusId, ST_FB_ONLINE   )*/ )//VOXOX - CJC - 2009.09.17 
	{
		return EnumPresenceState::PresenceStateOnline;
	}
	else if (!strcmp(StatusId, ST_OFFLINE      ) || 
			 !strcmp(StatusId, ST_SKYPE_OFFLINE) /*||*/
			 /*!strcmp(StatusId, ST_FB_OFFLINE   )*/ )//VOXOX - CJC - 2009.09.17 
	{
		return EnumPresenceState::PresenceStateOffline;
	}
	else if (!strcmp(StatusId, ST_INVISIBLE      ) || 
		     !strcmp(StatusId, ST_SKYPE_INVISIBLE) )
	{
		return EnumPresenceState::PresenceStateInvisible;
	}
	else if (!strcmp(StatusId, ST_AWAY      ) || 
			 !strcmp(StatusId, ST_SKYPE_AWAY) )
	{
		return EnumPresenceState::PresenceStateAway;
	}
	else if (!strcmp(StatusId, ST_UNAVAILABLE		) ||
			 !strcmp(StatusId, ST_BUSY				) ||
			 !strcmp(StatusId, ST_DND				) ||
			 !strcmp(StatusId, ST_SKYPE_UNAVAILABLE ) )
	{
		return EnumPresenceState::PresenceStateDoNotDisturb;
	}
	else
	{
		return EnumPresenceState::PresenceStateAway;	//VOXOX - JRT - 2009.09.15 - Odd choice of default
	}

	//VOXOX - JRT - 2009.09.15 - TODO: support for Extended_Away, Mobile, Tune.

}

//-----------------------------------------------------------------------------
