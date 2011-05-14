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

#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <sound/ISound.h>

#include <sound/AudioDevice.h>
#include <thread/Thread.h>

#include "playsound/PlaySoundFile.h"

#include <string>

/**
 * Plays a sound in a threaded way.
 *
 * Helper for the Sound class.
 *
 * @see Sound
 * @see Thread
 * @author Tanguy Krotoff
 */
class SoundThread : public ISound, public Thread {
public:

	/**
	 * Plays a sound file given its filename.
	 *
	 * @param filename sound file to play
	 */
	SoundThread(const std::string & filename);

	virtual ~SoundThread();

	void setLoops(int loops);

	bool setWaveOutDevice(const AudioDevice & device);

	void play();

	void stop();

protected:

	/**
	 * Starts the thread.
	 */
	void run();

private:

	/**
	 * Sound filename.
	 */
	std::string _filename;

	/**
	 * Wave out audio device name.
	 */
	AudioDevice _device;

	/**
	 * Number of time the sound has to be played, -1 for infinite
	 */
	int _loops;

	/**
	 * Stops or continues playing the sound.
	 */
	bool _stop;

	PlaySoundFile _soundFile;
};

#endif	//SOUNDTHREAD_H
