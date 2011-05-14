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

#include "MacVolumeControl.h"

#include "MacAudioDevice.h"

#include <util/Logger.h>

#include <CoreAudio/CoreAudio.h>

static const UInt32 MASTER_CHANNEL = 0;

MacVolumeControl::MacVolumeControl(const AudioDevice & audioDevice) {
	_audioDevice = audioDevice;
}

int MacVolumeControl::getLevel() {
	OSStatus status = noErr;
	UInt32 size = 0;
	Float32 level = 0.0;
	Float32 result = 0.0;
	AudioDeviceID audioDeviceId = String(_audioDevice.getData()[0]).toInteger();
	Boolean isInput = String(_audioDevice.getData()[2]).toInteger();

	size = sizeof(level);

	std::pair<int, int> range = getVolumeSettableChannelRange();
	for (int i = range.first; (i != - 1) && (i != range.second + 1); i++) { 
		status = AudioDeviceGetProperty(audioDeviceId, i, 
			isInput, kAudioDevicePropertyVolumeScalar, &size, &level);
		if (status) {
			LOG_ERROR("can't get device property: kAudioDevicePropertyVolumeScalar for channel "
				+ String::fromNumber(i));
			return -1;
		}

		result += level;
	}

	if (range.first != - 1) {
		result = result / ((range.second - range.first) + 1.0);
	}

	return (int) (result * 100.0);
}

bool MacVolumeControl::setLevel(unsigned level) {
	OSStatus status = noErr;
	Float32 fVolume = level / 100.0;
	UInt32 size = sizeof(fVolume);
	AudioDeviceID audioDeviceId = String(_audioDevice.getData()[0]).toInteger();
	Boolean isInput = String(_audioDevice.getData()[2]).toInteger();
	bool result = true;

	std::pair<int, int> range = getVolumeSettableChannelRange();
	for (int i = range.first; (i != - 1) && (i != range.second + 1); i++) { 
		status = AudioDeviceSetProperty(audioDeviceId, NULL, i,
			isInput, kAudioDevicePropertyVolumeScalar, size, &fVolume);
		if (status) {
			LOG_ERROR("can't set device property: kAudioDevicePropertyVolumeScalar on channem "
				+ String::fromNumber(i));
			result = false;
		}
	}

	return result;
}

bool MacVolumeControl::setMute(bool mute) {
	OSStatus status = noErr;
	bool bMute = mute;
	UInt32 size = sizeof(bool);
	AudioDeviceID audioDeviceId = String(_audioDevice.getData()[0]).toInteger();
	Boolean isInput = String(_audioDevice.getData()[2]).toInteger();

	std::pair<int, int> range = getVolumeSettableChannelRange();
	for (int i = range.first; (i != - 1) && (i != range.second + 1); i++) { 
		status = AudioDeviceSetProperty(audioDeviceId, NULL, i,
			isInput, kAudioDevicePropertyMute, size, &bMute);
		if (status) {
			LOG_ERROR("can't set device property: kAudioDevicePropertyMute on channel "
				+ String::fromNumber(i));
		}
	}

	return true;
}

bool MacVolumeControl::isMuted() {
	OSStatus status = noErr;
	bool bMute = false;
	UInt32 size = sizeof(bool);
	AudioDeviceID audioDeviceId = String(_audioDevice.getData()[0]).toInteger();
	Boolean isInput = String(_audioDevice.getData()[2]).toInteger();

	std::pair<int, int> range = getVolumeSettableChannelRange();
	for (int i = range.first; (i != - 1) && (i != range.second + 1); i++) { 
		status = AudioDeviceGetProperty(audioDeviceId, 0,
			isInput, kAudioDevicePropertyMute, &size, &bMute);
		if (status) {
			LOG_ERROR("can't get device property: kAudioDevicePropertyMute\n");
		} else if (bMute) {
			break;
		}
	}

	return bMute;
}

bool MacVolumeControl::isSettable() const {
	bool result = false;

	if (getVolumeSettableChannelRange().first != -1) {
		result = true;
	}

	return result;
}

std::pair<int, int> MacVolumeControl::getVolumeSettableChannelRange() const {
	OSStatus status = noErr;
	UInt32 size = 0;
	std::pair<int, int> result;
	result.first = -1;

	if (isChannelVolumeSettable(MASTER_CHANNEL)) {
		result.first = 0;
		result.second = 0;
	} else {
		AudioDeviceID audioDeviceId = String(_audioDevice.getData()[0]).toInteger();
		Boolean isInput = String(_audioDevice.getData()[2]).toInteger();

		status = AudioDeviceGetPropertyInfo(audioDeviceId, 0, isInput,
			kAudioDevicePropertyStreamConfiguration, &size, NULL);
		if (status) {
			LOG_ERROR("Can't get device property info: kAudioDevicePropertyStreamConfiguration");
			return result;
		}

		AudioBufferList *list = (AudioBufferList *) malloc(size);
		status = AudioDeviceGetProperty(audioDeviceId, 0, isInput,
			kAudioDevicePropertyStreamConfiguration, &size, list);
		if (status) {
			LOG_INFO("Can't get device property: kAudioDevicePropertyStreamConfiguration."
				" The device has no " + (isInput ? std::string("input") : std::string("output")) + " device.");
			free(list);
			return result;
		}

		for (unsigned i = 1; i < list->mBuffers[0].mNumberChannels + 1; i++) {
			if (isChannelVolumeSettable(i)) {
				result.first = i;
				break;
			}
		}

		result.second = list->mBuffers[0].mNumberChannels;

		free(list);
	}

	return result;
}

bool MacVolumeControl::isChannelVolumeSettable(UInt32 channel) const {
	OSStatus status = noErr;
	AudioDeviceID audioDeviceId = String(_audioDevice.getData()[0]).toInteger();
	Boolean isInput = String(_audioDevice.getData()[2]).toInteger();

	status = AudioDeviceGetPropertyInfo(audioDeviceId, channel,
		isInput, kAudioDevicePropertyVolumeScalar, NULL, NULL);
	if (status) {
		LOG_INFO("can't get device property: kAudioDevicePropertyVolumeScalar => cannot set the volume on this channel\n");
	}

	return (status == noErr);
}
