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

#ifndef OWPHONECALLSTATE_H
#define OWPHONECALLSTATE_H

#include <sipwrapper/EnumPhoneCallState.h>

#include <sound/AudioDevice.h>
#include <util/StringList.h>

class PhoneCall;
class Sound;

/**
 * Represents the state of a PhoneCall.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class PhoneCallState {
public:

	PhoneCallState();

	virtual ~PhoneCallState() {
	}

	virtual void execute(PhoneCall & phoneCall,bool doublecall = false) = 0;

	/**
	 * Gets the status code corresponding to this PhoneCall state.
	 *
	 * @return status code of this state
	 */
	virtual EnumPhoneCallState::PhoneCallState getCode() const = 0;

	/**
	 * Stops the incoming phone call ringtone.
	 */
	static void stopSoundIncomingCall();

protected:

	static AudioDevice getRingerAudioDevice();

	static std::string getSoundIncomingCallFile();

	static std::string getSoundDoubleCallFile();

	static std::string getSoundCallClosedFile();

	static void stopSoundCallClosed();

	static Sound * _soundIncomingCall;

	static Sound * _soundCallClosed;
};

#endif	//OWPHONECALLSTATE_H
