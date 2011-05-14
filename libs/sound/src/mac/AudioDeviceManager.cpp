/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include <sound/AudioDeviceManager.h>

#include "CoreAudioUtilities.h"
#include "MacAudioDevice.h"

#include <util/Logger.h>

std::list<AudioDevice> AudioDeviceManager::getInputDeviceList() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	std::list<AudioDevice> result;

	std::vector<AudioDeviceID> devList = CoreAudioUtilities::audioDeviceList(true);
	for (std::vector<AudioDeviceID>::const_iterator devIt = devList.begin();
		devIt != devList.end();
		++devIt) {
		std::vector<UInt32> dsList = CoreAudioUtilities::dataSourceList(*devIt, true);
		if (dsList.size() > 0) {
			for (std::vector<UInt32>::const_iterator dsIt = dsList.begin();
				dsIt != dsList.end();
				++dsIt) {
				MacAudioDevice macAudioDevice(*devIt, *dsIt, true);
				AudioDevice audioDevice(macAudioDevice.getData());
				result.push_back(audioDevice);
			}
		} else {
			MacAudioDevice macAudioDevice(*devIt, true);
			AudioDevice audioDevice(macAudioDevice.getData());
			result.push_back(audioDevice);
		}
	}

	return result;
}

std::list<AudioDevice> AudioDeviceManager::getOutputDeviceList() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	std::list<AudioDevice> result;

	std::vector<AudioDeviceID> devList = CoreAudioUtilities::audioDeviceList(false);
	for (std::vector<AudioDeviceID>::const_iterator devIt = devList.begin();
		devIt != devList.end();
		++devIt) {
		std::vector<UInt32> dsList = CoreAudioUtilities::dataSourceList(*devIt, false);
		if (dsList.size() > 0) {
			for (std::vector<UInt32>::const_iterator dsIt = dsList.begin();
				dsIt != dsList.end();
				++dsIt) {
				MacAudioDevice macAudioDevice(*devIt, *dsIt, false);
				AudioDevice audioDevice(macAudioDevice.getData());
				result.push_back(audioDevice);
			}
		} else {
			MacAudioDevice macAudioDevice(*devIt, false);
			AudioDevice audioDevice(macAudioDevice.getData());
			result.push_back(audioDevice);
		}
	}

	return result;
}

AudioDevice AudioDeviceManager::getDefaultOutputDevice() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	OSStatus status = noErr;
	AudioDevice result;

	AudioDeviceID deviceId;
	UInt32 size = sizeof(AudioDeviceID);
	status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &deviceId);
	if (status) {
		LOG_ERROR("can't get default output device");
		return result;
	}

	UInt32 dataSourceId;
	size = sizeof(UInt32);
	status = AudioDeviceGetProperty(deviceId, 0, 0, kAudioDevicePropertyDataSource, &size, &dataSourceId);
	if (status) {
		LOG_ERROR("can't get default output data source");
		return result;
	}

	MacAudioDevice macAudioDevice(deviceId, dataSourceId, false);
	result = AudioDevice(macAudioDevice.getData());

	return result;
}

bool AudioDeviceManager::setDefaultOutputDevice(const AudioDevice & audioDevice) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	OSStatus status = noErr;

	// Setting the device
	AudioDeviceID deviceId = String(audioDevice.getData()[0]).toInteger();
	UInt32 size = sizeof(AudioDeviceID);
	status = AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, size, &deviceId);
	if (status) {
		LOG_ERROR("can't set default output device to " + String::fromNumber(deviceId));
		return false;
	}
	////

	// Setting the data source
	UInt32 dataSourceId = String(audioDevice.getData()[1]).toInteger();
	size = sizeof(UInt32);
	status = AudioDeviceSetProperty(deviceId, NULL, 0, 0, kAudioDevicePropertyDataSource, size, &dataSourceId);
	if (status) {
		LOG_ERROR("can't set default output data source");
		return false;
	}
	////

	return true;
}

AudioDevice AudioDeviceManager::getDefaultInputDevice() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	OSStatus status = noErr;
	AudioDevice result;

	AudioDeviceID deviceId;
	UInt32 size = sizeof(AudioDeviceID);
	status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &size, &deviceId);
	if (status) {
		LOG_ERROR("can't get default output device");
		return result;
	}

	UInt32 dataSourceId;
	size = sizeof(UInt32);
	status = AudioDeviceGetProperty(deviceId, 0, 1, kAudioDevicePropertyDataSource, &size, &dataSourceId);
	if (status) {
		LOG_ERROR("can't get default output data source");
		return result;
	}

	MacAudioDevice macAudioDevice(deviceId, dataSourceId, true);
	result = AudioDevice(macAudioDevice.getData());

	return result;
}

bool AudioDeviceManager::setDefaultInputDevice(const AudioDevice & audioDevice) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	OSStatus status = noErr;

	// Setting the device
	AudioDeviceID deviceId = String(audioDevice.getData()[0]).toInteger();
	UInt32 size = sizeof(AudioDeviceID);
	status = AudioHardwareSetProperty(kAudioHardwarePropertyDefaultInputDevice, size, &deviceId);
	if (status) {
		LOG_ERROR("can't set default input device to " + String::fromNumber(deviceId));
		return false;
	}
	////

	// Setting the data source
	UInt32 dataSourceId = String(audioDevice.getData()[2]).toInteger();
	size = sizeof(UInt32);
	status = AudioDeviceSetProperty(deviceId, NULL, 0, 1, kAudioDevicePropertyDataSource, size, &dataSourceId);
	if (status) {
		LOG_ERROR("can't set default input data source");
		return false;
	}
	////

	return true;
}

AudioDeviceManager::AudioDeviceManager() {
}

AudioDeviceManager::~AudioDeviceManager() {
}
