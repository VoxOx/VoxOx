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

#include "MacAudioDevice.h"

#include "CoreAudioUtilities.h"

#include <util/String.h>

MacAudioDevice::MacAudioDevice(const StringList & data)
	: AudioDevice() {

	_data = data;
}

MacAudioDevice::MacAudioDevice(AudioDeviceID audioDeviceID, UInt32 dataSourceID, bool isInput)
	: AudioDevice() {
	_data += String::fromNumber(audioDeviceID);
	_data += String::fromNumber(dataSourceID);
	_data += String::fromNumber(isInput);
}

MacAudioDevice::MacAudioDevice(AudioDeviceID audioDeviceID, bool isInput)
	: AudioDevice() {
	_data += String::fromNumber(audioDeviceID);
	_data += String::fromNumber(0);
	_data += String::fromNumber(isInput);
}

MacAudioDevice::~MacAudioDevice() {
}

std::string MacAudioDevice::getName() const {
	std::string result;

	AudioDeviceID audioDeviceId = String(_data[0]).toInteger();
	UInt32 dataSourceId = String(_data[1]).toInteger();
	Boolean isInput = String(_data[2]).toInteger();

	std::string deviceName = CoreAudioUtilities::audioDeviceName(audioDeviceId, isInput);

	if (dataSourceId != 0) {
		std::string dataSourceName = CoreAudioUtilities::dataSourceName(audioDeviceId, isInput, dataSourceId);
		result = deviceName + " - " + dataSourceName;
	} else {
		result = deviceName;
	}

	return result;
}

StringList MacAudioDevice::getData() const {
	return _data;
}
