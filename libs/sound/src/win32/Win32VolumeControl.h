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

#ifndef OWWIN32VOLUMECONTROL_H
#define OWWIN32VOLUMECONTROL_H

#include <sound/IVolumeControl.h>
#include <sound/AudioDevice.h>

#include "../EnumDeviceType.h"

#include <windows.h>

/**
 * Gets and changes the volume of a Windows sound mixer.
 *
 * Inspired from Win32Volume class from the Zinf
 * (former FreeAmp) project.
 *
 * @see http://www.zinf.org/
 * @see http://www.codeproject.com/audio/admixer.asp
 * @see http://www.codeproject.com/audio/mixerSetControlDetails.asp
 * @author Tanguy Krotoff
 */
class Win32VolumeControl : public IVolumeControl {
public:

	/**
	 * Constructs a Win32VolumeControl object.
	 *
	 * @param audioDevice Windows audio device
	 */
	Win32VolumeControl(const AudioDevice & audioDevice);

	~Win32VolumeControl();

	bool setLevel(unsigned level);

	int getLevel();

	bool setMute(bool mute);

	bool isMuted();

	bool isSettable() const {
		return _isSettable;
	}

	/**
	 * Selects the audio device + input type as the record device.
	 *
	 * @return true if no error; false otherwise
	 */
	bool selectAsRecordDevice();

	/**
	 * Gets if the audio device + input type is the record device.
	 *
	 * @return true if audio device + input type is the record device; false otherwise
	 */
	bool isSelectedAsRecordDevice();

private:

	bool close();

	/**
	 * Inits the audio mixer device.
	 *
	 * @param deviceId Windows audio device id, id of a sound card
	 * @param deviceType the kind of audio mixer device to manipulate
	 * @return the win32 error code
	 */
	MMRESULT initVolumeControl(unsigned int deviceId, EnumDeviceType::DeviceType deviceType);

	/**
	 * Creates the mixer line.
	 *
	 * @param dwComponentType type of mixer controller
	 * @return the win32 error code
	 */
	MMRESULT createMixerLine(DWORD dwComponentType);

	/**
	 * Creates the mixer controller.
	 *
	 * @return the win32 error code
	 */
	MMRESULT createMixerControl(DWORD dwControlType);

	/**
	 * Creates the mixer controller.
	 *
	 * @param dwComponentType type of mixer controller
	 * @return the win32 error code
	 */
	MMRESULT createSecondMixerLine(DWORD dwComponentType);

	/**
	 * Handle to the mixer device.
	 */
	HMIXER _hMixer;

	/**
	 * The MIXERLINE structure describes the state and metrics of an audio line.
	 */
	MIXERLINEA _mxl;

	/**
	 * The MIXERCONTROL structure describes the state and metrics of a single control for an audio line.
	 */
	MIXERCONTROLA _mxc;

	/**
	 * The MIXERLINECONTROLS structure contains information about the controls of an audio line.
	 */
	MIXERLINECONTROLSA _mxlc;

	bool _isSettable;
};

#endif	//OWWIN32VOLUMECONTROL_H
