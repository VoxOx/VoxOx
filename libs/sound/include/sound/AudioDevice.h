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

#ifndef OWAUDIODEVICE_H
#define OWAUDIODEVICE_H

#include <sound/owsounddll.h>

#include <util/StringList.h>

/**
 * Identifies a part of an audio device.
 * e.g: the microphone input of device #1
 *
 * @author Philippe Bernery
 */
class AudioDevice {
public:
	/**
	 * Constructs an AudioDevice from a string identifying uniquely a
	 * sound device.
	 *
	 * @param data @see getData for more info
	 */
	OWSOUND_API AudioDevice(const StringList & data);

	OWSOUND_API AudioDevice();

	OWSOUND_API AudioDevice(const AudioDevice & audioDevice);

	OWSOUND_API AudioDevice & operator=(const AudioDevice & audioDevice);

	OWSOUND_API virtual ~AudioDevice();

	/**
	 * Gets a human readable string representing this audio device.
	 */
	OWSOUND_API virtual std::string getName() const;

	/**
	 * Gets data that describe the device.
	 *
	 * This is system-dependent.
	 */
	OWSOUND_API virtual StringList getData() const;

protected:

	/**
	 * System dependent id.
	 */
	StringList _data;

private:

	/** Updates _audioDevicePrivate. */
	void updateAudioDevicePrivate(const StringList & deviceId);

	/** System-dependent implementation. */
	AudioDevice * _audioDevicePrivate;
};

#endif	//OWAUDIODEVICE_H
