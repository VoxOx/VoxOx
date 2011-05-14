/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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

/**
 * @author Mathieu Stute <mstute@gmail.com>
 */

#include "LinuxVolumeControl.h"
#include "EnumDeviceType.h"

#include "alsa_mixer.h"

#include <util/Logger.h>

LinuxVolumeControl::LinuxVolumeControl(const AudioDevice & audioDevice) {
	_audioDevice = audioDevice;

	EnumDeviceType::DeviceType deviceType =
		EnumDeviceType::toDeviceType(audioDevice.getData()[2]);

	switch (deviceType) {
	case EnumDeviceType::DeviceTypeMicrophoneIn:
	case EnumDeviceType::DeviceTypeWaveIn:
		_strDeviceType = "capture";
		break;
	case EnumDeviceType::DeviceTypeMasterVolume:
	case EnumDeviceType::DeviceTypeWaveOut:
		_strDeviceType = "pcm";
		break;
	default:
		LOG_FATAL("Unknown audio device type");
	}
}

int LinuxVolumeControl::getLevel() {
	int level;
	// TODO: retrieve the sound card name from the AudioDevice
	if (_strDeviceType == "pcm") {
		get_mixer_level("hw:0", PCM_VOLUME, &level);
		return level;
	} else if (_strDeviceType == "capture") {
		get_mixer_level("hw:0", CAPTURE_VOLUME, &level);
		return level;
	}
	return -1;
}

bool LinuxVolumeControl::setLevel(unsigned level) {
	// TODO: retrieve the sound card name from the AudioDevice
	if (_strDeviceType == "pcm") {
		return set_mixer_level("hw:0", PCM_VOLUME, level);
		return level;
	} else if (_strDeviceType == "capture") {
		return set_mixer_level("hw:0", CAPTURE_VOLUME, level);
	}
	return false;
}

bool LinuxVolumeControl::isMuted() {
	return false;
}

bool LinuxVolumeControl::setMute(bool mute) {
	return false;
}

bool LinuxVolumeControl::isSettable() const {
	return true;
}
