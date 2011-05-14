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

#include <sound/AudioDevice.h>

#include <cutil/global.h>

#if defined(OS_WINDOWS)
	#include "win32/Win32AudioDevice.h"
#elif defined(OS_MACOSX)
	#include "mac/MacAudioDevice.h"
#elif defined(OS_LINUX)
	#include "linux/LinuxAudioDevice.h"
#else
	#error This OS has not been tested
#endif

AudioDevice::AudioDevice(const StringList & data) {
	_audioDevicePrivate = NULL;
	updateAudioDevicePrivate(data);
}

AudioDevice::AudioDevice() {
	_audioDevicePrivate = NULL;
}

AudioDevice::AudioDevice(const AudioDevice & audioDevice) {
	_audioDevicePrivate = NULL;
	if (audioDevice._audioDevicePrivate) {
		updateAudioDevicePrivate(audioDevice._audioDevicePrivate->getData());
	}
}

AudioDevice & AudioDevice::operator=(const AudioDevice & audioDevice) {
	if (audioDevice._audioDevicePrivate) {
		updateAudioDevicePrivate(audioDevice._audioDevicePrivate->getData());
	}

	return *this;
}

AudioDevice::~AudioDevice() {
	if (_audioDevicePrivate) {
		delete _audioDevicePrivate;
		_audioDevicePrivate = NULL;
	}
}

void AudioDevice::updateAudioDevicePrivate(const StringList & data) {
	if (_audioDevicePrivate) {
		delete _audioDevicePrivate;
		_audioDevicePrivate = NULL;
	}

#if defined(OS_WINDOWS)
	_audioDevicePrivate = new Win32AudioDevice(data);
#elif defined(OS_MACOSX)
	_audioDevicePrivate = new MacAudioDevice(data);
#elif defined(OS_LINUX)
	_audioDevicePrivate = new LinuxAudioDevice(data);
#endif
}

std::string AudioDevice::getName() const {
	std::string result;

	if (_audioDevicePrivate) {
		result = _audioDevicePrivate->getName();
	}

	return result;
}

StringList AudioDevice::getData() const {
	StringList result;

	if (_audioDevicePrivate) {
		result = _audioDevicePrivate->getData();
	}

	return result;
}
