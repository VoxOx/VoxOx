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
 * @author Mathieu Stute
 */

#ifndef LINUXSOUND_H
#define LINUXSOUND_H

#include <sound/ISound.h>
#include <sound/AudioDevice.h>
#include <thread/Thread.h>

#include <sndfile.h>

class LinuxSound : public ISound, public Thread {
public:

	LinuxSound(const std::string & filename);

	virtual ~LinuxSound();

	void setLoops(int loops);

	void play();

	void stop();

	bool setWaveOutDevice(const AudioDevice & device);

protected:

	void run();

private:

	std::string _filename;

	std::string _alsaDevice;

	int _loops;

	int _mustStop;
};

#endif	//LINUXSOUND_H
