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

#include "../EnumDeviceType.h"

#include <portaudio.h>

#include <util/Logger.h>

/**
 * Retrieves the number of audio device found.
 */
static int getNbDevices() {
	int nbDevices = Pa_GetDeviceCount();
	if (nbDevices < 0) {
		LOG_ERROR("Pa_GetDeviceCount()=" + String::fromNumber(nbDevices));
		PaError err = nbDevices;
		LOG_ERROR("an error occured while using the portaudio stream, error message=" + String(Pa_GetErrorText(err)));
	}
	return nbDevices;
}

std::list<AudioDevice> AudioDeviceManager::getInputDeviceList() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	std::list<AudioDevice> deviceList;

	int nbDevices = getNbDevices();

	//Iterates over devices
	for (int i = 0; i < nbDevices; i++) {
		const PaDeviceInfo * deviceInfo = Pa_GetDeviceInfo(i);

		//If the device has input capability
		if (deviceInfo->maxInputChannels > 0) {

			std::string deviceName;
			if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice) {
				deviceName = "(default) ";
			}
			deviceName += deviceInfo->name;

			StringList data;
			data += deviceName;
			data += String::fromNumber(i);
			data += EnumDeviceType::toString(EnumDeviceType::DeviceTypeWaveIn);
			deviceList.push_back(AudioDevice(data));
		}
	}

	return deviceList;
}

std::list<AudioDevice> AudioDeviceManager::getOutputDeviceList() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	std::list<AudioDevice> deviceList;

	int nbDevices = getNbDevices();

	//Iterates over devices
	for (int i = 0; i < nbDevices; i++) {
		const PaDeviceInfo * deviceInfo = Pa_GetDeviceInfo(i);

		//If the device has input capability
		if (deviceInfo->maxOutputChannels > 0) {

			std::string deviceName;
			if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice) {
				deviceName = "(default) ";
			}
			deviceName += deviceInfo->name;

			StringList data;
			data += deviceName;
			data += String::fromNumber(i);
			data += EnumDeviceType::toString(EnumDeviceType::DeviceTypeMasterVolume);
			deviceList.push_back(AudioDevice(data));
		}
	}

	return deviceList;
}

AudioDevice AudioDeviceManager::getDefaultOutputDevice() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	std::list<AudioDevice> list = getOutputDeviceList();

	std::list<AudioDevice>::iterator it;
	for (it = list.begin(); it != list.end(); it++) {
		String deviceName = (*it).getName();
		if (deviceName.contains("default")) {
			return (*it);
		}
	}

	return AudioDevice();
}

bool AudioDeviceManager::setDefaultOutputDevice(const AudioDevice & audioDevice) {
	return false;
}

AudioDevice AudioDeviceManager::getDefaultInputDevice() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	std::list<AudioDevice> list = getInputDeviceList();

	std::list<AudioDevice>::iterator it;
	for (it = list.begin(); it != list.end(); it++) {
		String deviceName = (*it).getName();
		if (deviceName.contains("default")) {
			return (*it);
		}
	}

	return AudioDevice();
}

bool AudioDeviceManager::setDefaultInputDevice(const AudioDevice & audioDevice) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	return false;
}

AudioDeviceManager::AudioDeviceManager() {
	Pa_Initialize();
}

AudioDeviceManager::~AudioDeviceManager() {
	Pa_Terminate();
}
