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

#include <sipwrapper/EnumPhoneCallState.h>
#include <util/String.h>
#include <util/Logger.h>

std::string EnumPhoneCallState::toString(PhoneCallState state) {
	switch(state) {
	case PhoneCallStateUnknown:
		return "PhoneCallStateUnknown";

	case PhoneCallStateError:
		return "PhoneCallStateError";

	case PhoneCallStateResumed:
		return "PhoneCallStateResumed";

	case PhoneCallStateTalking:
		return "PhoneCallStateTalking";

	case PhoneCallStateDialing:
		return "PhoneCallStateDialing";

	case PhoneCallStateRinging:
		return "PhoneCallStateRinging";

	case PhoneCallStateClosed:
		return "PhoneCallStateClosed";

	case PhoneCallStateIncoming:
		return "PhoneCallStateIncoming";

	case PhoneCallStateHold:
		return "PhoneCallStateHold";

	case PhoneCallStateMissed:
		return "PhoneCallStateMissed";

	case PhoneCallStateRedirected:
		return "PhoneCallStateRedirected";

	case PhoneCallStateRingingStart:
		return "PhoneCallStateRingingStart";

	case PhoneCallStateRingingStop:
		return "PhoneCallStateRingingStop";

	default:
		LOG_FATAL("unknown PhoneCallState=" + String::fromNumber(state));
		return "PhoneCallStateUnknown";
	}

	return "Unknown Phone Call State";
}

