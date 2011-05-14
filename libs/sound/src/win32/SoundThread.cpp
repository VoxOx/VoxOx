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
#include <string>

#include "SoundThread.h"

#include "playsound/PlaySoundFile.h"

#include <windows.h>
#include <process.h>

#include <util/File.h>

SoundThread::SoundThread(const std::string & filename) {
	_filename = filename;
	_stop = false;

	//Plays the sound only one time by default
	_loops = 1;
}

SoundThread::~SoundThread() {
}

bool SoundThread::setWaveOutDevice(const AudioDevice & device) {
	_device = device;
	return true;
}

void SoundThread::setLoops(int loops) {
	_loops = loops;
}

void SoundThread::play() {
	if (File::exists(std::string(_filename))) {
		start();
	}
}

void SoundThread::run() {
	_soundFile.setWaveOutDevice(_device);

	int i = 0;
	while ((i < _loops || _loops == -1) && !_stop) {
		if (!_soundFile.play(_filename)) {
			//If the file cannot be played, stop the thread
			_stop = true;
		}
		i++;
	}
	_stop = false;
}

void SoundThread::stop() {
	_stop = true;
	_soundFile.stop();
}
