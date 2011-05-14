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

#ifndef OWISOUND_H
#define OWISOUND_H

#include <util/Interface.h>

class AudioDevice;

/**
 * Reimplementation of QSound from Qt.
 *
 * The big difference is that you can choose the wave out audio device.
 * Currently it only plays wave audio files.
 *
 * Different backends are implemented: UNIX, PortAudio, Windows...
 *
 * @see QSound
 * @see Sound
 * @author Tanguy Krotoff
 */
class ISound : Interface {
public:

	/**
	 * Sets the sound to repeat loops times when it is played.
	 * Passing the value -1 will cause the sound to loop indefinitely.
	 *
	 * @param loops number of time the sound has to be played; -1 for infinite
	 */
	virtual void setLoops(int loops) = 0;

	/**
	 * Sets the wave out audio device.
	 *
	 * @param device wave out audio device
	 * @return true if the device was changed; false otherwise
	 */
	virtual bool setWaveOutDevice(const AudioDevice & device) = 0;

	/**
	 * Plays the sound.
	 */
	virtual void play() = 0;

	/**
	 * Stops playing the sound.
	 */
	virtual void stop() = 0;
};

#endif	//OWISOUND_H
