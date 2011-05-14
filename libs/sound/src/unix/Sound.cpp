/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include <sound/AudioDeviceManager.h>
#include <sound/Sound.h>

#include "UnixSoundThread.h"

/**
 * @author Mathieu Stute
 */
Sound::Sound(const std::string & filename) {
	_soundPrivate = new UnixSoundThread(filename);
}

Sound::~Sound() {
	stop();
	delete _soundPrivate;
}

void Sound::setLoops(int loops) {
	_soundPrivate->setLoops(loops);
}

bool Sound::setWaveOutDevice(const AudioDevice & device) {
	return _soundPrivate->setWaveOutDevice(device);
}

void Sound::stop() {
	_soundPrivate->stop();
}

void Sound::play() {
	_soundPrivate->play();
}

void Sound::play(const std::string & filename, const AudioDevice & device) {
	UnixSoundThread * unixSoundThread = new UnixSoundThread(filename);
	unixSoundThread->setWaveOutDevice(device);
	unixSoundThread->setAutoDelete(true);
	unixSoundThread->play();
}
