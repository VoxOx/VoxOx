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

#include <imwrapper/EnumPresenceState.h>

#include <util/String.h>
#include <util/Logger.h>

using namespace std;

string EnumPresenceState::toString(PresenceState presenceState) {
	string result;

	switch (presenceState) {
	case PresenceStateOnline:
		result = "online";
		break;
	case PresenceStateOffline:
		result = "offline";
		break;
	case PresenceStateInvisible:
		result = "invisible";
		break;
	case PresenceStateAway:
		result = "away";
		break;
	case PresenceStateDoNotDisturb:
		result = "donotdisturb";
		break;
	case PresenceStateUnknown:
		result = "unknown";
		break;
	case PresenceStateMulti:
		result = "multi";
		break;
	case PresenceStateUserDefined:
		result = "userdefined";
		break;
	case PresenceStateUnavailable:
		result = "unavailable";
		break;
	default:
		LOG_FATAL("unknown state=" + String::fromNumber(presenceState));
	}

	return result;
}

EnumPresenceState::PresenceState EnumPresenceState::fromString(const string & presenceState) {
	PresenceState result = PresenceStateUnknown;

	if (presenceState == "online") {
		result = PresenceStateOnline;
	} else if (presenceState == "offline") {
		result = PresenceStateOffline;
	} else if (presenceState == "invisible") {
		result = PresenceStateInvisible;
	} else if (presenceState == "away") {
		result = PresenceStateAway;
	} else if (presenceState == "donotdisturb") {
		result = PresenceStateDoNotDisturb;
	} else if (presenceState == "userdefined") {
		result = PresenceStateUserDefined;
	} else if (presenceState == "unknown") {
		result = PresenceStateUnknown;
	} else if (presenceState == "unavailable") {
		result = PresenceStateUnavailable;
	} else if (presenceState == "multi") {
		result = PresenceStateMulti;
	} else {
		LOG_FATAL("unknown state=" + presenceState);
	}

	return result;
}

//static
bool EnumPresenceState::isOnlineEx( PresenceState presenceState ) 
{
	bool result = false;

	switch (presenceState) 
	{
	case PresenceStateOnline:
	case PresenceStateAway:
	case PresenceStateDoNotDisturb:
	case PresenceStateUnavailable:
	case PresenceStateInvisible:		//This is known only to logged in user.  Others see this as Offline.
		result = true;
		break;

	case PresenceStateOffline:
	case PresenceStateUnknown:			//VOXOX - JRT - 2009.07.27 - Not sure about this one.
		result = false;
		break;

	case PresenceStateMulti:
	case PresenceStateUserDefined:
		result = true;
		break;

	default:
		LOG_FATAL("unknown state=" + String::fromNumber(presenceState));
	}

	return result;
}

//static
bool EnumPresenceState::isOfflineEx( PresenceState presenceState ) 
{
	bool result = false;

	switch (presenceState) 
	{
	case PresenceStateOnline:
	case PresenceStateAway:
	case PresenceStateDoNotDisturb:
	case PresenceStateInvisible:		//This is known only to logged in user.  Others see this as Offline.
		result = false;
		break;

	case PresenceStateUnavailable:
	case PresenceStateOffline:
	case PresenceStateUnknown:
		result = true;
		break;

	case PresenceStateMulti:
	case PresenceStateUserDefined:
		result = true;
		break;

	default:
		LOG_FATAL("unknown state=" + String::fromNumber(presenceState));
	}

	return result;
}

