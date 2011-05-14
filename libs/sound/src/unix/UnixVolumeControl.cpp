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

#include "UnixVolumeControl.h"
#include "EnumDeviceType.h"

#include <cutil/global.h>
#include <util/Logger.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#if defined(OS_LINUX)
	#include <linux/soundcard.h>
#elif defined(OS_BSD)
	#include <sys/soundcard.h>
#endif

#include <iostream>
using namespace std;

const char * sound_device_names[] = SOUND_DEVICE_NAMES;

UnixVolumeControl::UnixVolumeControl(const AudioDevice & audioDevice) {
	_audioDevice = audioDevice;

	EnumDeviceType::DeviceType deviceType = 
		EnumDeviceType::toDeviceType(audioDevice.getData()[2]);

	switch (deviceType) {
	case EnumDeviceType::DeviceTypeMicrophoneIn:
	case EnumDeviceType::DeviceTypeWaveIn:
		_strDeviceType = "igain";
		break;

	case EnumDeviceType::DeviceTypeMasterVolume:
	case EnumDeviceType::DeviceTypeWaveOut:
		_strDeviceType = "pcm";
		break;

	default:
		LOG_FATAL("Unknown audio device type");
	}
}

int UnixVolumeControl::getLevel() {
	int fd, devmask, i, level;

	fd = open("/dev/mixer", O_RDONLY);
	ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask);

	//Find mixer
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		bool nameTest = !strcmp(_strDeviceType.c_str(), sound_device_names[i]);
		if (((1 << i) & devmask) && nameTest) {
			break;
		}
	}

	ioctl(fd, MIXER_READ(i), & level);
	level = level >> 8;
	::close(fd);
	return level;
}

bool UnixVolumeControl::setLevel(unsigned level) {
	int fd, devmask, i, new_level;

	fd = open("/dev/mixer", O_RDONLY);
	ioctl(fd, SOUND_MIXER_READ_DEVMASK, & devmask);

	//Find mixer
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		bool nameTest = !strcmp(_strDeviceType.c_str(), sound_device_names[i]);
		if (((1 << i) & devmask) && nameTest) {
			break;
		}
	}

	new_level = (level << 8) + level;
	ioctl(fd, MIXER_WRITE(i), &new_level);
	::close(fd);

	return true;
}

bool UnixVolumeControl::isMuted() {
	int fd, devmask, i, level;

	fd = open("/dev/mixer", O_RDONLY);
	ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask);

	//Find mixer
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		bool nameTest = !strcmp(_strDeviceType.c_str(), sound_device_names[i]);
		if (((1 << i) & devmask) && nameTest) {
			break;
		}
	}

	ioctl(fd, MIXER_READ(i), &level);
	level = level >> 8;
	::close(fd);

	return (level == 0);
}

bool UnixVolumeControl::setMute(bool mute) {
	setLevel(0);
	return false;
}

bool UnixVolumeControl::isSettable() const {
	return true;
}
