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
#include "PhoneCallState.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <sound/Sound.h>

#include <util/Logger.h>

Sound * PhoneCallState::_soundIncomingCall = NULL;
Sound * PhoneCallState::_soundCallClosed = NULL;

PhoneCallState::PhoneCallState() {
}

void PhoneCallState::stopSoundIncomingCall() {
	if (_soundIncomingCall) {
		_soundIncomingCall->stop();
		delete _soundIncomingCall;
		_soundIncomingCall = NULL;
	}
}

void PhoneCallState::stopSoundCallClosed() {
	if (_soundCallClosed) {
		_soundCallClosed->stop();
		delete _soundCallClosed;
		_soundCallClosed = NULL;
	}
}

std::string PhoneCallState::getSoundIncomingCallFile() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return config.getAudioIncomingCallFile();
}

std::string PhoneCallState::getSoundDoubleCallFile() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return config.getAudioDoubleCallFile();
}

std::string PhoneCallState::getSoundCallClosedFile() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return config.getAudioCallClosedFile();
}

AudioDevice PhoneCallState::getRingerAudioDevice() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return AudioDevice(config.getAudioRingerDeviceId());
}
