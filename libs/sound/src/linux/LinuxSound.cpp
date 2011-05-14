/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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

/**
 * @author Mathieu Stute <mstute@gmail.com>
 */

#include "LinuxSound.h"

#include <thread/ThreadEvent.h>
#include <util/Logger.h>
#include <util/String.h>
#include <util/File.h>

#include "alsa_sndfile.h"

LinuxSound::LinuxSound(const std::string & filename)
	: _filename(filename), _loops(1) {

	_alsaDevice = "default";
	_mustStop = 0;
}

LinuxSound::~LinuxSound() {
}

bool LinuxSound::setWaveOutDevice(const AudioDevice & device) {
	_alsaDevice = String(device.getData()[1]);
	return true;
}

void LinuxSound::run() {
	for (int i = 0; i < _loops; i++) {
		alsa_play_file(_filename.c_str(), _alsaDevice.c_str(), &_mustStop);
	}
}

void LinuxSound::play() {
	_mustStop = 0;
	if (File::exists(String(_filename))) {
		start();
	}
}

void LinuxSound::stop() {
	_mustStop = 1;
	terminate();
}

void LinuxSound::setLoops(int loops) {
	_loops = loops;
}
