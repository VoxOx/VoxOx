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

#include "Win32VolumeControl.h"

#include "EnumDeviceType.h"
#include "Win32AudioDeviceId.h"

#include <util/Logger.h>

static const unsigned MAXIMUM_VOLUME_LEVEL_DEFINED_BY_USER = 100;

Win32VolumeControl::Win32VolumeControl(const AudioDevice & audioDevice) {

	std::string deviceName = audioDevice.getData()[0];
	//String deviceId = audioDevice.getData()[1];
	EnumDeviceType::DeviceType deviceType = EnumDeviceType::toDeviceType(audioDevice.getData()[2]);

	_hMixer = NULL;

	int deviceId = Win32AudioDeviceId::getMixerDeviceId(deviceName);

	MMRESULT mr = initVolumeControl(deviceId, deviceType);
	if (mr != MMSYSERR_NOERROR) {
		_hMixer = NULL;
		_isSettable = false;
		if (deviceType == EnumDeviceType::DeviceTypeWaveIn) {
			deviceType = EnumDeviceType::DeviceTypeMicrophoneIn;
			MMRESULT mr = initVolumeControl(deviceId, deviceType);
			if (mr == MMSYSERR_NOERROR) {
				_isSettable = true;
			}
		}
	} else {
		_isSettable = true;
	}
}

Win32VolumeControl::~Win32VolumeControl() {
	close();
}

int Win32VolumeControl::getLevel() {
	if (!_isSettable) {
		return 0;
	}

	MMRESULT mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME);
	if (mr != MMSYSERR_NOERROR) {
		return -1;
	}

	const unsigned MINIMUM_VOLUME_LEVEL = _mxc.Bounds.dwMinimum;
	const unsigned MAXIMUM_VOLUME_LEVEL  = _mxc.Bounds.dwMaximum;

	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;

	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = _mxc.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcdVolume;

	mr = ::mixerGetControlDetailsA((HMIXEROBJ) _hMixer, &mxcd,
		MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("couldn't get the volume level, mixerGetControlDetailsA() failed");
		return -1;
	}

	return (int) (((float) ((mxcdVolume.dwValue - MINIMUM_VOLUME_LEVEL) * MAXIMUM_VOLUME_LEVEL_DEFINED_BY_USER) /
		(float) (MAXIMUM_VOLUME_LEVEL - MINIMUM_VOLUME_LEVEL)) + 0.5);
}

bool Win32VolumeControl::setLevel(unsigned level) {
	if (!_isSettable) {
		return false;
	}

	MMRESULT mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME);
	if (mr != MMSYSERR_NOERROR) {
		return false;
	}

	const unsigned MINIMUM_VOLUME_LEVEL = _mxc.Bounds.dwMinimum;
	const unsigned MAXIMUM_VOLUME_LEVEL  = _mxc.Bounds.dwMaximum;

	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
	mxcdVolume.dwValue = level * (MAXIMUM_VOLUME_LEVEL - MINIMUM_VOLUME_LEVEL) / MAXIMUM_VOLUME_LEVEL_DEFINED_BY_USER;

	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = _mxc.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcdVolume;

	mr = ::mixerSetControlDetails((HMIXEROBJ) _hMixer, &mxcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("couldn't set the volume level, mixerSetControlDetails() failed");
		return false;
	}

	return true;
}

bool Win32VolumeControl::setMute(bool mute) {
	if (!_isSettable) {
		return false;
	}

	MMRESULT mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_MUTE);
	if (mr != MMSYSERR_NOERROR) {
		return false;
	}

	MIXERCONTROLDETAILS_BOOLEAN mxcbMute;
	mxcbMute.fValue = mute;

	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = _mxc.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = &mxcbMute;

	mr = ::mixerSetControlDetails((HMIXEROBJ) _hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("couldn't mute/unmute the audio device, mixerSetControlDetails() failed");
		return false;
	}

	return true;
}

bool Win32VolumeControl::isMuted() {
	if (!_isSettable) {
		return false;
	}

	MMRESULT mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_MUTE);
	if (mr != MMSYSERR_NOERROR) {
		return false;
	}

	MIXERCONTROLDETAILS_BOOLEAN mxcbMute;

	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = _mxc.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = &mxcbMute;

	mr = ::mixerGetControlDetailsA((HMIXEROBJ) _hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("couldn't get if the audio device is mute/unmute, mixerGetControlDetailsA() failed");
		return false;
	}

	return mxcbMute.fValue;
}

bool Win32VolumeControl::selectAsRecordDevice() {
	if (!_isSettable) {
		return false;
	}

	MMRESULT mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_MUX);
	if (mr != MMSYSERR_NOERROR) {
		return false;
	}

	MIXERCONTROLDETAILS_BOOLEAN mxcbSelect;
	mxcbSelect.fValue = true;

	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = _mxc.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = &mxcbSelect;

	mr = ::mixerSetControlDetails((HMIXEROBJ) _hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("couldn't select the audio device as the record device, mixerSetControlDetails() failed");
		return false;
	}

	return true;
}

bool Win32VolumeControl::isSelectedAsRecordDevice() {
	if (!_isSettable) {
		return false;
	}

	MMRESULT mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_MUX);
	if (mr != MMSYSERR_NOERROR) {
		return false;
	}

	MIXERCONTROLDETAILS_BOOLEAN mxcbSelect;

	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = _mxc.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = &mxcbSelect;

	mr = ::mixerGetControlDetailsA((HMIXEROBJ) _hMixer, &mxcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("couldn't select the audio device as the record device, mixerSetControlDetails() failed");
		return false;
	}

	return mxcbSelect.fValue;
}

bool Win32VolumeControl::close() {
	if (!_isSettable) {
		return false;
	}

	if (_hMixer) {
		MMRESULT mr = ::mixerClose(_hMixer);
		if (mr != MMSYSERR_NOERROR) {
			LOG_ERROR("couldn't close the mixer, mixerClose() failed");
			return false;
		}
		return true;
	}

	return false;
}

MMRESULT Win32VolumeControl::initVolumeControl(unsigned deviceId, EnumDeviceType::DeviceType deviceType) {
	MMRESULT mr = ::mixerOpen(&_hMixer, deviceId, NULL, NULL, MIXER_OBJECTF_MIXER);
	if (mr != MMSYSERR_NOERROR) {
		_hMixer = NULL;
		return mr;
	}

	MIXERCAPSA mxcaps;
	mr = ::mixerGetDevCapsA(deviceId, &mxcaps, sizeof(MIXERCAPSA));
	if (mr != MMSYSERR_NOERROR) {
		return mr;
	}

	LOG_DEBUG("manufacturer's name for the mixer=" +
		std::string(mxcaps.szPname) + " " +
		String::fromNumber(mxcaps.wMid) + " " +
		String::fromNumber(mxcaps.wPid));

	DWORD dwComponentType;

	switch (deviceType) {
	case EnumDeviceType::DeviceTypeWaveOut:
		dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
		break;

	case EnumDeviceType::DeviceTypeWaveIn:
		dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
		break;

	case EnumDeviceType::DeviceTypeCDOut:
		dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;
		break;

	case EnumDeviceType::DeviceTypeMicrophoneOut:
		dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
		break;

	case EnumDeviceType::DeviceTypeMicrophoneIn:
		dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
		break;

	case EnumDeviceType::DeviceTypeMasterVolume:
		dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
		break;

	default:
		LOG_FATAL("unknow device type=" + EnumDeviceType::toString(deviceType));
	}

	mr = createMixerLine(dwComponentType);
	if (mr != MMSYSERR_NOERROR) {
		return mr;
	}

	//For microphone in, we first look for the wave in mixer
	//and then for the microphone
	if (deviceType == EnumDeviceType::DeviceTypeMicrophoneIn) {
		mr = createSecondMixerLine(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE);
		if (mr != MMSYSERR_NOERROR) {
			return mr;
		}
	}
	//

	mr = createMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME);
	if (mr != MMSYSERR_NOERROR) {
		return mr;
	}

	LOG_DEBUG("destination line name=" + std::string(_mxl.szName) +
		" volume controller name=" + std::string(_mxc.szName));

	//Everything went fine
	return mr;
}

MMRESULT Win32VolumeControl::createMixerLine(DWORD dwComponentType) {
	_mxl.cbStruct = sizeof(MIXERLINEA);
	_mxl.dwComponentType = dwComponentType;

	MMRESULT mr = ::mixerGetLineInfoA((HMIXEROBJ) _hMixer, &_mxl,
		MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("mixerGetLineInfoA() failed using dwComponentType=" + String::fromNumber(dwComponentType));
	}

	return mr;
}

MMRESULT Win32VolumeControl::createSecondMixerLine(DWORD dwComponentType) {
	unsigned connections = _mxl.cConnections;
	DWORD destination = _mxl.dwDestination;
	MMRESULT mr;

	for (unsigned i = 0; i < connections; ++i) {
		_mxl.cbStruct = sizeof(MIXERLINEA);
		_mxl.dwSource = i;
		_mxl.dwDestination = destination;

		mr = ::mixerGetLineInfoA((HMIXEROBJ) _hMixer, &_mxl,
						MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_SOURCE);

		if (mr == MMSYSERR_NOERROR && _mxl.dwComponentType == dwComponentType) {
			break;
		}
	}

	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("mixerGetLineInfoA() failed using dwComponentType=" + String::fromNumber(dwComponentType));
	}

	return mr;
}

MMRESULT Win32VolumeControl::createMixerControl(DWORD dwControlType) {
	_mxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
	_mxlc.dwLineID = _mxl.dwLineID;

	//MIXERCONTROL_CONTROLTYPE_VOLUME
	//MIXERCONTROL_CONTROLTYPE_MIXER
	//MIXERCONTROL_CONTROLTYPE_MUX
	//MIXERCONTROL_CONTROLTYPE_MUTE
	_mxlc.dwControlType = dwControlType;

	_mxlc.cControls = 1;
	_mxlc.cbmxctrl = sizeof(MIXERCONTROLA);
	_mxlc.pamxctrl = &_mxc;

	MMRESULT mr = ::mixerGetLineControlsA((HMIXEROBJ) _hMixer, &_mxlc,
		MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (mr != MMSYSERR_NOERROR) {
		LOG_ERROR("mixerGetLineControlsA() failed using dwControType=" + String::fromNumber(dwControlType));
	}

	return mr;
}
