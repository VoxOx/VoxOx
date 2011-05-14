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

#include "Win32AudioDeviceId.h"

#include <util/Logger.h>

#include <windows.h>

int Win32AudioDeviceId::getWaveOutDeviceId(const std::string & deviceName) {
	unsigned nbDevices = ::waveOutGetNumDevs();
	if (nbDevices == 0) {
		//No audio device are present
		return -1;
	}

	WAVEOUTCAPSA outcaps;

	for (unsigned deviceId = 0; deviceId < nbDevices; deviceId++) {
		if (MMSYSERR_NOERROR == ::waveOutGetDevCapsA(deviceId, &outcaps, sizeof(WAVEOUTCAPSA))) {
			if (deviceName == outcaps.szPname) {
				return deviceId;
			}
		}
	}

	//Default deviceId is 0
	return 0;
}

int Win32AudioDeviceId::getWaveInDeviceId(const std::string & deviceName) {
	unsigned nbDevices = ::waveInGetNumDevs();
	if (nbDevices == 0) {
		//No audio device are present
		return -1;
	}

	WAVEINCAPSA incaps;

	for (unsigned deviceId = 0; deviceId < nbDevices; deviceId++) {
		if (MMSYSERR_NOERROR == ::waveInGetDevCapsA(deviceId, &incaps, sizeof(WAVEINCAPSA))) {
			if (deviceName == incaps.szPname) {
				return deviceId;
			}
		}
	}

	//Default deviceId is 0
	return 0;
}

int Win32AudioDeviceId::getMixerDeviceId(const std::string & mixerName) {
	unsigned nbMixers = ::mixerGetNumDevs();
	if (nbMixers == 0) {
		//No audio mixer device are present
		return -1;
	}

	MIXERCAPSA mixcaps;

	for (unsigned mixerId = 0; mixerId < nbMixers; mixerId++) {
		if (MMSYSERR_NOERROR == ::mixerGetDevCapsA(mixerId, &mixcaps, sizeof(MIXERCAPSA))) {
			if (mixerName == mixcaps.szPname) {
				return mixerId;
			}
		}
	}

	//Default deviceId is 0
	return 0;
}
