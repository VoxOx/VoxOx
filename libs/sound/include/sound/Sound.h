/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef OWSOUND_H
#define OWSOUND_H

#include <sound/AudioDevice.h>
#include <sound/ISound.h>

#include <string>

/**
 * Sound implementation.
 *
 * @see ISound
 * @author Tanguy Krotoff
 */
class Sound : public ISound {
public:

	/**
	 * Constructs a Sound that can quickly play the sound in a file named filename.
	 *
	 * @param filename sound file
	 */
	OWSOUND_API Sound(const std::string & filename);

	OWSOUND_API virtual ~Sound();

	OWSOUND_API void setLoops(int loops);

	OWSOUND_API bool setWaveOutDevice(const AudioDevice & device);

	/**
	 * Plays the sound from filename to the device.
	 *
	 * @param filename sound file
	 * @param device output audio device name
	 */
	OWSOUND_API static void play(const std::string & filename, const AudioDevice & device);

	/**
	 * Plays the sound from filename to the default output device.
	 *
	 * @param filename sound file
	 * @param device output audio device name
	 */
	OWSOUND_API static void play(const std::string & filename);

	OWSOUND_API void play();

	OWSOUND_API void stop();

private:

	/** System-dependant implementation. */
	ISound * _soundPrivate;
};

#endif	//OWSOUND_H
